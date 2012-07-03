#include"tknet.h"

#define CHECK_NAT_TRY_AGAIN   0
#define CHECK_NAT_FINE        1

struct CheckNATProc
{
	struct Process       proc;
	struct KeyInfoCache  *pKeyInfo;
	struct KeyInfo       *pFailedKey;
	uchar                TryFlag;
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
	struct pipe *pCmdPipe = PipeFindByName("cmd"), *pPipe = PipeMap("NatCmd");
	struct FindPossibleKeyInfoByNotNumPa FpkibnnPa;
	char   CmdStr[32];

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

	if(!ifPipeTo(pPipe,pCmdPipe))
	{
		PipeDirectTo(pPipe,pCmdPipe);
	}

	sprintf(CmdStr,"nat %d",FpkibnnPa.pPossible->num);
	PipeFlow(pPipe,CmdStr,strlen(CmdStr)+1,NULL);

	g_BkgdNatTestRes = NAT_T_UNKNOWN;
	
	pCkProc->pFailedKey = FpkibnnPa.pPossible;
	//we assume it will fail, if not, we set pFailedKey to NULL.

	return PS_CALLBK_RET_DONE;
}

STEP( WaitCheckRes )
{
	struct CheckNATProc *pCkProc = GET_STRUCT_ADDR(pa_pProc,struct CheckNATProc,proc);
			
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

			if(pCkProc->TryFlag == CHECK_NAT_FINE)
			{
				pCkProc->TryFlag = CHECK_NAT_TRY_AGAIN;
				return FlagName(pa_pProc,"BeginCheckNAT");
			}
			else if(pCkProc->TryFlag == CHECK_NAT_TRY_AGAIN)
			{
				return FlagName(pa_pProc,"WaitToCheck");
			}
		}
		else if(ifNetAddrEqual(&g_BkgdNatTestAddrRes,&g_NATMapAddr))
		{
			//every thing is fine, go back to wait the next time's check.

			pCkProc->TryFlag = CHECK_NAT_FINE;
			return FlagName(pa_pProc,"WaitToCheck");
		}
		else 
		{
			//this is Check NAT's job, finding that the full-cone NAT's map address
			//has changed, we inform others by sending E-mail with the new address.

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
	return 0;
}

void CheckNATProcConsAndBegin(struct ProcessingList *pa_pProcList,
		struct KeyInfoCache *pa_pKeyInfo)
{
	struct CheckNATProc *pProc = tkmalloc(struct CheckNATProc);
	ProcessCons( &pProc->proc );

	pProc->pFailedKey = NULL;
	pProc->pKeyInfo = pa_pKeyInfo;
	pProc->TryFlag  = CHECK_NAT_FINE;

	PROCESS_ADD_STEP( &pProc->proc , WaitToCheck , g_WaitLevel[5]);
	PROCESS_ADD_STEP( &pProc->proc , BeginCheckNAT , g_WaitLevel[2]);
	PROCESS_ADD_STEP( &pProc->proc , WaitCheckRes , g_WaitLevel[4]);
	PROCESS_ADD_STEP( &pProc->proc , SendingNewAddr , g_WaitLevel[4]);
	
	ProcessStart(&pProc->proc, pa_pProcList);
}

int main()
{
	return 0;
}
