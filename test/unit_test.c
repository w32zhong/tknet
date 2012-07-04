#include"tknet.h"

#define CHECK_NAT_TRY_AGAIN   0
#define CHECK_NAT_FINE        1

#define CHECK_MAIL_BEGIN      0
#define CHECK_MAIL_SEE_IT     1
#define CHECK_MAIL_NOT_SEE    2
#define CHECK_MAIL_ERROR      3


char  sta_NatStr[MAX_MAIL_CONTENT_LEN];//a NAT string which gives to the POP3
                                       //process to see whether it is in mailbox.

BOOL  sta_Pop3ProcState;//it is a flag to notify check process that POP3 process
                        //is ending for which it is waiting, as well as give check
                        //process the result.

struct CheckNATProc
{
	struct Process          proc;
	struct KeyInfoCache    *pKeyInfo;
	struct KeyInfo         *pFailedKey;
	struct pipe            *pCheckPipe;
	uchar                   STUNTryFlag;
	struct ProcessingList  *pProcList;
};

struct FindPossibleKeyInfoByNotNumPa
{
	int             NotNum;
	struct KeyInfo *pPossible;
};

void CheckNATProcEndCallbk(struct Process *pa_pProc)
{
	ProcessFree(pa_pProc);
	PROMPT(Usual,"CheckNATProc end.\n");
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindPossibleKeyInfoByNotNum)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pFpkibnn,struct FindPossibleKeyInfoByNotNumPa, pa_else);

	if(pInfo->valid == KEY_INFO_VALID_UNSURE ||
			pInfo->valid == KEY_INFO_VALID_WORKS)
	{
		if(pFpkibnn->NotNum != pInfo->num)
		{
			pFpkibnn->pPossible = pInfo;
			return 1;
		}
		else
		{
			pFpkibnn->pPossible = pInfo;
		}
	}
		
	return pa_pINow->now == pa_pIHead->last;
}

STEP( WaitToCheck )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
	struct FindPossibleKeyInfoByNotNumPa FpkibnnPa;

	if(pa_state == PS_STATE_OVERTIME)
	{
		return PS_CALLBK_RET_DONE;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BeginCheckNAT )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
	struct pipe *pCmdPipe = PipeFindByName("cmd");
	struct FindPossibleKeyInfoByNotNumPa FpkibnnPa;
	char   CmdStr[BKGD_CMD_MAX_LEN];

	if(pCkProc->pFailedKey)
		FpkibnnPa.NotNum = pCkProc->pFailedKey->num;
	else
		FpkibnnPa.NotNum = -1;

	FpkibnnPa.pPossible = NULL;

	ForEach(&pCkProc->pKeyInfo->IKeyInfo,&FindPossibleKeyInfoByNotNum,&FpkibnnPa);

	if(NULL == FpkibnnPa.pPossible)
	{
		TK_EXCEPTION(
				PROMPT(Usual,"No possible key for NAT check.\n");
				);

		return PS_CALLBK_RET_ABORT;
	}
	else if(pCmdPipe == NULL)
	{
		TK_EXCEPTION(
				PROMPT(Usual,"No cmd pipe for NAT check.\n");
				);

		return PS_CALLBK_RET_ABORT;
	}

	if(!ifPipeTo(pCkProc->pCheckPipe,pCmdPipe))
	{
		PipeDirectTo(pCkProc->pCheckPipe,pCmdPipe);
	}

	sprintf(CmdStr,"nat %d",FpkibnnPa.pPossible->num);

	if(ifBkgdSubProcess())
		return PS_CALLBK_RET_REDO;

	PipeFlow(pCkProc->pCheckPipe,CmdStr,strlen(CmdStr)+1,NULL);

	g_BkgdNatTestRes = NAT_T_UNKNOWN;
	
	pCkProc->pFailedKey = FpkibnnPa.pPossible;
	//we assume it will fail, if not, we set pFailedKey to NULL.

	return PS_CALLBK_RET_DONE;
}

STEP( WaitCheckRes )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
	char  IPText[16];
			
	if(g_BkgdNatTestRes != NAT_T_UNKNOWN)
	{
		//we get the STUN result.
		pCkProc->pFailedKey = NULL;

		if(g_BkgdNatTestRes != NAT_T_FULL_CONE)
		{
			//this is a troublesome situation, I do not know how to best handle
			//it either; we just try it again, if the result is the same, we keep 
			//trying after a new waiting; doing this because a none-full-cone NAT 
			//cannot be a BDG server anymore.

			if(pCkProc->STUNTryFlag == CHECK_NAT_FINE)
			{
				pCkProc->STUNTryFlag = CHECK_NAT_TRY_AGAIN;
				return FlagName(pa_pProc,"BeginCheckNAT");
			}
			else if(pCkProc->STUNTryFlag == CHECK_NAT_TRY_AGAIN)
			{
				return FlagName(pa_pProc,"WaitToCheck");
			}
		}
		else if(ifNetAddrEqual(&g_BkgdNatTestAddrRes,&g_NATMapAddr))
		{
			//every thing is fine, go back to wait the next time's check.

			pCkProc->STUNTryFlag = CHECK_NAT_FINE;
			return FlagName(pa_pProc,"WaitToCheck");
		}
		else 
		{
			//this is Check NAT's job, finding that the full-cone NAT's map address
			//has changed, we inform others by sending E-mail with the new address.
			g_NATMapAddr = g_BkgdNatTestAddrRes;

			GetIPText(&g_BkgdNatTestAddrRes,IPText);
			sprintf(sta_NatStr,"BridgePeer %s %d",IPText,g_BkgdNatTestAddrRes.port);

			return PS_CALLBK_RET_DONE;
		}
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pCkProc->pFailedKey->valid = KEY_INFO_VALID_UNSURE;

		//go back and re-select a possible key,
		//this process may go into an infinite loop.
		return FlagName(pa_pProc,"BeginCheckNAT");
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SendingNewAddr )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
	char                 CmdStr[BKGD_CMD_MAX_LEN];
	struct KeyInfo      *pSmtpKey,*pTmpContentKey;

	if(pa_state == PS_STATE_FIRST_TIME)
	{
		pSmtpKey = KeyInfoSelectA(pCkProc->pKeyInfo,KEY_INFO_TYPE_SMTPSERVER);
		VCK(pSmtpKey == NULL,return PS_CALLBK_RET_ABORT;);

		pTmpContentKey = KeyInfoInsert( NewKeyInfoFromStrLine(sta_NatStr),
				pCkProc->pKeyInfo);
		VCK(pTmpContentKey == NULL,return PS_CALLBK_RET_ABORT;);

		sprintf(CmdStr,"smtp %d %d",pSmtpKey->num,pTmpContentKey->num);
		
		if(ifBkgdSubProcess())
			return PS_CALLBK_RET_REDO;

		PipeFlow(pCkProc->pCheckPipe,CmdStr,strlen(CmdStr)+1,NULL);
		KeyInfoDele(pTmpContentKey,pCkProc->pKeyInfo);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		//retry.
		return PS_CALLBK_RET_REDO;
	}
	else if(!ifBkgdSubProcess())
	{
		//it seems like the bkgd side sending process has finished,
		//go to next step to check mail further.
		return PS_CALLBK_RET_DONE;
	}
	
	return PS_CALLBK_RET_GO_ON;
}

static struct POP3Proc*
NewPOP3ProcFromKeyInfo(struct KeyInfoCache *);
	
STEP( StartPop3Process )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
	struct POP3Proc *pPop3Proc;

	pPop3Proc = NewPOP3ProcFromKeyInfo(pCkProc->pKeyInfo);
	VCK(pPop3Proc == NULL, return PS_CALLBK_RET_ABORT);
	ProcessSafeStart( &pPop3Proc->proc,pCkProc->pProcList,pa_pINow,pa_pIForward);

	sta_Pop3ProcState = CHECK_MAIL_BEGIN;

	return PS_CALLBK_RET_DONE;
}

STEP( WaitPop3Res )
{
	if(sta_Pop3ProcState == CHECK_MAIL_ERROR)
	{
		return FlagName(pa_pProc,"StartPop3Process");
	}
	else if(sta_Pop3ProcState == CHECK_MAIL_SEE_IT)
	{
		return FlagName(pa_pProc,"WaitToCheck");
	}
	else if(sta_Pop3ProcState == CHECK_MAIL_NOT_SEE)
	{
		return FlagName(pa_pProc,"SendingNewAddr");
	}
	else
		return PS_CALLBK_RET_REDO;
}

void CheckNATProcConsAndBegin(struct ProcessingList *pa_pProcList,
		struct KeyInfoCache *pa_pKeyInfo)
{
	struct CheckNATProc *pProc = tkmalloc(struct CheckNATProc);
	ProcessCons( &pProc->proc );

	pProc->pFailedKey = NULL;
	pProc->pKeyInfo = pa_pKeyInfo;
	pProc->STUNTryFlag  = CHECK_NAT_FINE;
	pProc->pCheckPipe = PipeMap("CHECK");
	pProc->pProcList = pa_pProcList;

	sta_NatStr[0] = '\0';

	PROCESS_ADD_STEP( &pProc->proc , WaitToCheck , g_WaitLevel[5]);
	PROCESS_ADD_STEP( &pProc->proc , BeginCheckNAT , g_WaitLevel[2]);
	PROCESS_ADD_STEP( &pProc->proc , WaitCheckRes , g_WaitLevel[4]);
	PROCESS_ADD_STEP( &pProc->proc , SendingNewAddr , g_WaitLevel[4]);
	PROCESS_ADD_STEP( &pProc->proc , StartPop3Process , g_WaitLevel[2]);
	PROCESS_ADD_STEP( &pProc->proc , WaitPop3Res , g_WaitLevel[2]);
	
	ProcessStart(&pProc->proc, pa_pProcList);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( CheckMail )
{
	struct NetInfoMail *pMail = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct NetInfoMail , ln);

	if(strcmp(pMail->content,sta_NatStr) == 0)
	{
		sta_Pop3ProcState = CHECK_MAIL_SEE_IT;
		return 1;
	}
	else
		return pa_pINow->now == pa_pIHead->last;
}

static void 
Pop3Notify(struct Process *pa_)
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR(pa_ , struct POP3Proc , proc);
	
	if( pProc->ifEnterSucc )
	{
		sta_Pop3ProcState = CHECK_MAIL_NOT_SEE;
		ForEach( &pProc->IMailsHead , &CheckMail , NULL );
	}
	else
		sta_Pop3ProcState = CHECK_MAIL_ERROR;
	
	tkfree(pProc->pSock);
	POP3ProcFree( pProc );
	tkfree(pProc);
}

static struct POP3Proc*
NewPOP3ProcFromKeyInfo(struct KeyInfoCache *pa_pKeyInfoCache)
{
	struct POP3Proc *pPop3Proc;
	char             text[KEY_INFO_MAX_LEN];
	char            *pNextWord,*pText = text;
	struct KeyInfo  *pPop3Key;
	char             AddrText[32];
	int              i,ifEnableSSL;
	char 			 buff0[32];
	char 			 buff1[32];
	char 			 buff2[32];

	pPop3Key = KeyInfoSelectA(pa_pKeyInfoCache,KEY_INFO_TYPE_MAILSERVER);
	VCK(pPop3Key == NULL, return NULL);
	
	strcpy(text , pPop3Key->text);
	GetIPText( &pPop3Key->addr , AddrText );

	for( i = 0 ; i < 6 ; i++ )
	{
		pNextWord = GetNextSeparateStr(&pText);
		VCK( pNextWord == NULL , return NULL; );

		switch(i)
		{
			case 3:
				strcpy(buff0,pNextWord);//buff0 = whether in SSL
				break;
			case 4:
				strcpy(buff1,pNextWord);//buff1 = usr name
				break;
			case 5:
				strcpy(buff2,pNextWord);//buff2 = password
				break;
			default:
				break;
		}
	}

	VCK(sscanf(buff0,"%d",&ifEnableSSL) == 0 ,return 0;);
	VCK( !(ifEnableSSL == 0 || ifEnableSSL == 1 ) , return 0;);

	pPop3Proc = tkmalloc(struct POP3Proc);
	PROMPT(Usual,"pop3 proc:%s/%d,%d,%s,%s.\n",AddrText , pPop3Key->addr.port,
			ifEnableSSL,buff1,buff2);
	MakeProtoPOP3Proc( pPop3Proc , AddrText , pPop3Key->addr.port ,
			ifEnableSSL,buff1,buff2);

	pPop3Proc->pSock = tkmalloc(struct Sock);
	SockOpen(pPop3Proc->pSock,TCP,0);

	pPop3Proc->proc.NotifyCallbk = &Pop3Notify;

	return pPop3Proc;
}

int main()
{
	return 0;
}
