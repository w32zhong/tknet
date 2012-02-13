#include "headers.h"

static BOOL sta_LoopFlag;
static BOOL sta_ProcRes;
static struct KeyInfoCache *sta_pKeyInfoCache;
extern uchar g_NATtype;
extern struct NetAddr g_BdgPeerAddr;

DEF_STRUCT_CONSTRUCTOR( KeyInfoCache ,
		out_cons->KeyInfoNumbers = 0;
		out_cons->IKeyInfo = GetIterator(NULL);
		)

char*
GetNextSeparateStr(char ** io_pStrNow)
{
	char *pResStr;

	while( **io_pStrNow == ' ' && **io_pStrNow != '\0')
	{
		(*io_pStrNow) ++;
	}

	pResStr = *io_pStrNow;
	
	while( **io_pStrNow != ' ' && **io_pStrNow != '\0')
	{
		(*io_pStrNow) ++;
	}

	if( **io_pStrNow == ' ')
	{
		**io_pStrNow = '\0';
		(*io_pStrNow) ++;
	}
	else
	{
		**io_pStrNow = '\0';
	}
	
	return pResStr;
}

static struct KeyInfo*
NewKeyInfoFromStrLine(char *io_pStr)
{
	char           *pNextWord;
	uchar          type;
	struct NetAddr addr;
	char           text[KEY_INFO_MAX_LEN];
	struct KeyInfo *pNewKeyInfo;
	uint           i;

	strcpy(text,io_pStr);
	
	i = strlen(text);
	if( i >= 1 && text[i-1] == '\n')
	{
		text[i-1] = '\0';
	}
	//chop the \n at the end of the line,
	//because GetNextSeparateStr() doesn't
	//expect any \n in a line.

	pNextWord = GetNextSeparateStr(&io_pStr);
	VCK( pNextWord == NULL , return NULL; );

	if( strcmp(pNextWord,"MailServer") == 0 )
	{
		type = KEY_INFO_TYPE_MAILSERVER;
	}
	else if( strcmp(pNextWord,"BridgePeer") == 0 )
	{
		type = KEY_INFO_TYPE_BRIDGEPEER;
	}
	else if( strcmp(pNextWord,"STUNServer") == 0 )
	{
		type = KEY_INFO_TYPE_STUNSERVER;
	}
	else if( strcmp(pNextWord,"SMTPServer") == 0 )
	{
		type = KEY_INFO_TYPE_SMTPSERVER;
	}
	else if( strcmp(pNextWord,"Config") == 0 )
	{
		type = KEY_INFO_TYPE_CONFIG;
	}
	else
	{
		TK_EXCEPTION("KeyInfo type");
		return NULL;
	}
	
	pNextWord = GetNextSeparateStr(&io_pStr);
	VCK( pNextWord == NULL , return NULL; );

	addr.IPv4 = GetIPVal(pNextWord);
	addr.IPv4 = ntohl(addr.IPv4);

	pNextWord = GetNextSeparateStr(&io_pStr);
	VCK( pNextWord == NULL , return NULL; );

	VCK( 0 == sscanf(pNextWord,"%d",&i) , return NULL; );
	addr.port = (ushort)i;
	
	pNewKeyInfo = tkmalloc(struct KeyInfo);
	pNewKeyInfo->num = 0;
	pNewKeyInfo->valid = KEY_INFO_VALID_UNSURE;
	pNewKeyInfo->type  = type;
	strcpy(pNewKeyInfo->text,text);
	pNewKeyInfo->addr  = addr;
	ListNodeCons(&pNewKeyInfo->ln);

	return pNewKeyInfo;
}

void 
KeyInfoReadFile( struct KeyInfoCache *pa_pCache , const char *pa_pFileName )
{
	FILE *pf = fopen( pa_pFileName , "r+" );
	char buff[KEY_INFO_MAX_LEN];
	struct KeyInfo *pKeyInfo;

	while(fgets( buff , KEY_INFO_MAX_LEN , pf ))
	{
		pKeyInfo = NewKeyInfoFromStrLine(buff);

		if( NULL != pKeyInfo )
		{
			AddOneToListTail(&pa_pCache->IKeyInfo,&pKeyInfo->ln);
			pKeyInfo->num = pa_pCache->KeyInfoNumbers;
			pa_pCache->KeyInfoNumbers ++;
		}
	}

	fclose(pf);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(WriteKeyInfo)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pf,FILE,pa_else);

	fwrite( pInfo->text , 1 , strlen( pInfo->text ) , pf );
	fwrite( "\n" , 1 , 1 , pf );

	return pa_pINow->now == pa_pIHead->last;
}

void 
KeyInfoWriteFile( struct KeyInfoCache *pa_pCache , const char *pa_pFileName )
{
	FILE *pf = fopen( pa_pFileName , "w+" );
	ForEach( &pa_pCache->IKeyInfo , &WriteKeyInfo , pf );
	fclose(pf);
}

#define SWITCH_CPY_STRING( _keyword , _case  , _str , _buff ) \
		_keyword _case : strcpy( _buff , _str );\
		break

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(TraceKeyInfo)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	char valid[16];
	char type[16];
	char addr[32];

	switch(pInfo->valid)
	{
	SWITCH_CPY_STRING(case,KEY_INFO_VALID_UNSURE,"?",valid);
	SWITCH_CPY_STRING(case,KEY_INFO_VALID_NOT,"X",valid);
	SWITCH_CPY_STRING(case,KEY_INFO_VALID_WORKS,"ok",valid);
	SWITCH_CPY_STRING(;,default,"err",valid);
	}

	switch(pInfo->type)
	{
	SWITCH_CPY_STRING(case,KEY_INFO_TYPE_MAILSERVER,"MailServer",type);
	SWITCH_CPY_STRING(case,KEY_INFO_TYPE_BRIDGEPEER,"BridgePeer",type);
	SWITCH_CPY_STRING(case,KEY_INFO_TYPE_STUNSERVER,"STUNServer",type);
	SWITCH_CPY_STRING(case,KEY_INFO_TYPE_SMTPSERVER,"SMTPServer",type);
	SWITCH_CPY_STRING(case,KEY_INFO_TYPE_CONFIG,"config",type);
	SWITCH_CPY_STRING(;,default,"err",valid);
	}

	GetAddrText(&pInfo->addr,addr);

	printf("%6d %6s %16s %20s \n",pInfo->num,valid,type,addr);

	return pa_pINow->now == pa_pIHead->last;
}

void 
KeyInfoTrace( struct KeyInfoCache *pa_pCache )
{
	printf("%6s %6s %16s %20s \n","num","valid","type","addr");
	ForEach( &pa_pCache->IKeyInfo , &TraceKeyInfo , NULL );
}

static
DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION(FreeKeyInfo,struct KeyInfo ,ln , ;);

void 
KeyInfoFree(struct KeyInfoCache *pa_pCache)
{
	ForEach( &pa_pCache->IKeyInfo , &FreeKeyInfo , NULL );
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoNotUsedByType)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pFkipa,struct FindKeyInfoByTypePa ,pa_else);

	if(pFkipa->TypeToFind == pInfo->type &&
			pInfo->valid == KEY_INFO_VALID_UNSURE )
	{
		pFkipa->found = pInfo;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}
	
struct KeyInfo*
KeyInfoSelectA( struct KeyInfoCache *pa_pCache , uchar pa_type )
{
	struct FindKeyInfoByTypePa fkipa;
	fkipa.TypeToFind = pa_type;
	fkipa.found = NULL;

	ForEach( &pa_pCache->IKeyInfo , &FindKeyInfoNotUsedByType , &fkipa );

	if(fkipa.found)
	{
		fkipa.found->valid = KEY_INFO_VALID_NOT;
	}

	return fkipa.found;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByNum)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pFkibnpa,struct FindKeyInfoByNumPa ,pa_else);

	if(pFkibnpa->NumToFind == pInfo->num )
	{
		pFkibnpa->found = pInfo;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

void
KeyInfoWorksFine( struct KeyInfoCache *pa_pCache , int pa_num )
{
	struct FindKeyInfoByNumPa fkibnpa;
	fkibnpa.NumToFind = pa_num;
	fkibnpa.found = NULL;

	ForEach( &pa_pCache->IKeyInfo , &FindKeyInfoByNum , &fkibnpa );

	if(fkibnpa.found)
	{
		fkibnpa.found->valid = KEY_INFO_VALID_WORKS;
	}
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByValid)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pFkipa,struct FindKeyInfoByValidPa ,pa_else);

	if( pFkipa->valid == pInfo->valid )
	{
		pFkipa->found = pInfo;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

static
DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION( SafeFreeKeyInfo , struct KeyInfo , ln , ; )

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( DeleKeyInfoByType )
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST( pType , uchar , pa_else );

	if( pInfo->valid == KEY_INFO_VALID_NOT &&
			pInfo->type == *pType )
	{
		return SafeFreeKeyInfo( pa_pIHead , pa_pINow , pa_pIForward , NULL );
	}
	if( pInfo->valid == KEY_INFO_VALID_WORKS &&
			pInfo->type == *pType )
	{
		pInfo->valid = KEY_INFO_VALID_UNSURE;
	}
	
	return pa_pINow->now == pa_pIHead->last;
}

void
KeyInfoUpdate( struct KeyInfoCache *pa_pCache )
{
	struct FindKeyInfoByValidPa fkipa;
	fkipa.valid = KEY_INFO_VALID_WORKS;

	while(1)
	{
		fkipa.found = NULL;
		ForEach( &pa_pCache->IKeyInfo , &FindKeyInfoByValid , &fkipa );

		if(fkipa.found)
		{
			ForEach( &pa_pCache->IKeyInfo , &DeleKeyInfoByType , &(fkipa.found->type) );
		}
		else
		{
			break;
		}
	}
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByAddr)
{
	struct KeyInfo *pInfo = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct KeyInfo,ln);
	DEF_AND_CAST(pFkipa,struct FindKeyInfoByAddrPa ,pa_else);

	if( ifNetAddrEqual(&pFkipa->addr,&pInfo->addr) )
	{
		pFkipa->found = pInfo;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( LoadMail )
{
	struct NetInfoMail *pMail = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct NetInfoMail , ln);
	DEF_AND_CAST( pInfoCache , struct KeyInfoCache , pa_else );
	struct KeyInfo *pKeyInfo;
	struct FindKeyInfoByAddrPa fkipa;

	pKeyInfo = NewKeyInfoFromStrLine(pMail->content);

	if( NULL != pKeyInfo )
	{
		fkipa.found = NULL;
		fkipa.addr = pKeyInfo->addr;
		ForEach( &pInfoCache->IKeyInfo , &FindKeyInfoByAddr , &fkipa );

		if( fkipa.found )
		{
			tkfree(pKeyInfo);
		}
		else
		{
			AddOneToListTail(&pInfoCache->IKeyInfo,&pKeyInfo->ln);
			pKeyInfo->num = pInfoCache->KeyInfoNumbers;
			pInfoCache->KeyInfoNumbers ++;
		}
	}

	return pa_pINow->now == pa_pIHead->last;
}

static void 
Pop3Notify(struct Process *pa_)
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR(pa_ , struct POP3Proc , proc);

	if( pProc->ifEnterSucc )
	{
		ForEach( &pProc->IMailsHead , &LoadMail , sta_pKeyInfoCache );
	}
	
	POP3ProcFree( pProc );
	sta_ProcRes = pProc->ifEnterSucc;
	sta_LoopFlag = 0;
}

static void 
StunNotify(struct Process *pa_)
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_ , struct STUNProc , proc);

	if( pProc->NatTypeRes == NAT_T_UNKNOWN )
	{
		sta_ProcRes = 0;
	}
	else
	{
		g_NATtype = pProc->NatTypeRes;
		sta_ProcRes = 1;
	}

	ProcessFree( pa_ );
	sta_LoopFlag = 0;
}

static void 
BdgHelloNotify(struct Process *pa_)
{
	struct BridgeProc *pProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	DEF_AND_CAST(pProcPa,struct BridgeHelloStepPa , pProc->Else);

	g_BdgPeerAddr = pProcPa->addr;
	
	sta_ProcRes = pProcPa->res;
	sta_LoopFlag = 0;
	
	ProcessFree( pa_ );
}

BOOL
KeyInfoUse( struct KeyInfo *pa_pInfo , struct KeyInfoCache *pa_pKeyInfoCache ,struct Sock *pa_pMainSock)
{
	struct Sock Pop3Sock;
	struct POP3Proc Pop3Proc;
	struct STUNProc StunProc;
	struct BridgeProc BdgProc;
	struct BridgeHelloStepPa BdgProcPa;
	struct ProcessingList ProcList;
	char   text[KEY_INFO_MAX_LEN];
	uint i,ifEnableSSL;
	char AddrText[32];
	char buff0[16];
	char buff1[16];
	char buff2[16];
	char *pNextWord;
	char *pText = text;

	sta_LoopFlag = 1;
	ProcessingListCons( &ProcList );
	strcpy(text , pa_pInfo->text);
	GetIPText( &pa_pInfo->addr , AddrText );

	
	if( pa_pInfo->type == KEY_INFO_TYPE_MAILSERVER )
	{
		for( i = 0 ; i < 6 ; i++ )
		{
			pNextWord = GetNextSeparateStr(&pText);
			VCK( pNextWord == NULL , return 0; );

			switch(i)
			{
				case 3:
					strcpy(buff0,pNextWord);
					break;
				case 4:
					strcpy(buff1,pNextWord);
					break;
				case 5:
					strcpy(buff2,pNextWord);
					break;
				default:
					break;
			}
		}

		VCK(sscanf(buff0,"%d",&ifEnableSSL) == 0 ,return 0;);
		VCK( !(ifEnableSSL == 0 || ifEnableSSL == 1 ) , return 0;);

		printf("pop3 proc:%s/%d,%d,%s,%s.\n",AddrText , pa_pInfo->addr.port ,ifEnableSSL,buff1,buff2);
		MakeProtoPOP3Proc( &Pop3Proc , AddrText , pa_pInfo->addr.port ,ifEnableSSL,buff1,buff2);

		Pop3Proc.proc.NotifyCallbk = &Pop3Notify;
		Pop3Proc.pSock = &Pop3Sock;
		SockOpen( &Pop3Sock , TCP , 0);
		sta_pKeyInfoCache = pa_pKeyInfoCache;
		
		ProcessStart( &Pop3Proc.proc , &ProcList );
	}
	else if( pa_pInfo->type == KEY_INFO_TYPE_STUNSERVER )
	{
		printf("stun proc:%s/%d.\n",AddrText , pa_pInfo->addr.port);
		MakeProtoStunProc(&StunProc ,pa_pMainSock ,AddrText,pa_pInfo->addr.port);
		
		StunProc.proc.NotifyCallbk = &StunNotify;
		
		ProcessStart( &StunProc.proc , &ProcList );
	}
	else if( pa_pInfo->type == KEY_INFO_TYPE_BRIDGEPEER )
	{
		printf("bridge 'hello proc':%s/%d.\n",AddrText , pa_pInfo->addr.port);
		BdgProcPa.res = 0;
		BdgProcPa.addr = pa_pInfo->addr;
		BridgeClientTryBdgServerProc(&BdgProc,&BdgProcPa,pa_pMainSock);
		BdgProc.proc.NotifyCallbk = &BdgHelloNotify;

		ProcessStart( &BdgProc.proc , &ProcList );
	}
	else if( pa_pInfo->type == KEY_INFO_TYPE_CONFIG )
	{
		for( i = 0 ; i < 4 ; i++ )
		{
			pNextWord = GetNextSeparateStr(&pText);
			VCK( pNextWord == NULL , return 0; );

			if(pNextWord[0] == '\0')
			{
				return 0;
			}

			if(i == 3)
			{
				strcpy(g_MyName,pNextWord);
			}
		}
			
		pNextWord = GetNextSeparateStr(&pText);
		//Target Name is an optional config item.
		if(pNextWord)
		{
			strcpy(g_TargetName,pNextWord);
		}

		printf("use config: port %d,named %s.\n",pa_pInfo->addr.port,g_MyName);
		
		i = 0;//to count times of trys.
		while(!SockOpen(pa_pMainSock,UDP,pa_pInfo->addr.port))
		{
			printf("port binded , change and retry ...\n");
			pa_pInfo->addr.port ++;
			i ++;

			if(i>2)
			{
				printf("port binding retry exceed.\n");
				return 0;
			}
		}
		SockSetNonblock(pa_pMainSock);

		sta_ProcRes = 1;
		sta_LoopFlag = 0;
	}
	else
	{
		TK_EXCEPTION("KeyInfo type");
		return 0;
	}
	
	while( sta_LoopFlag )
	{
		DoProcessing( &ProcList );
		tkMsSleep(100);
	}

	if( pa_pInfo->type == KEY_INFO_TYPE_MAILSERVER )
	{
		SockClose( &Pop3Sock );
	}

	return sta_ProcRes;
}

BOOL 
KeyInfoTry(struct KeyInfoCache *pa_pInfoChache , uchar pa_type , struct Sock *pa_pMainSock)
{
	struct KeyInfo *pKeyInfo;
	BOOL res = 0;
	while(!res)
	{
		pKeyInfo = KeyInfoSelectA(pa_pInfoChache,pa_type);
		if( pKeyInfo == NULL )
		{
			break;
		}

		res = KeyInfoUse(pKeyInfo,pa_pInfoChache , pa_pMainSock);
	}

	if(res)
	{
		KeyInfoWorksFine(pa_pInfoChache,pKeyInfo->num);
	}

	return res;
}
