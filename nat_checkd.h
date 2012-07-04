#define CHECK_NAT_TRY_AGAIN   0
#define CHECK_NAT_FINE        1

#define CHECK_MAIL_BEGIN      0
#define CHECK_MAIL_SEE_IT     1
#define CHECK_MAIL_NOT_SEE    2
#define CHECK_MAIL_ERROR      3

struct CheckNATProc
{
	struct Process          proc;
	struct KeyInfoCache    *pKeyInfo;
	struct KeyInfo         *pFailedKey,*pTmpContentKey;
	struct pipe            *pCheckPipe;
	uchar                   STUNTryFlag;
	struct ProcessingList  *pProcList;
};

struct FindPossibleKeyInfoByNotNumPa
{
	int             NotNum;
	struct KeyInfo *pPossible;
};

struct CheckNATProc* 
CheckNATProcConsAndBegin(struct ProcessingList *,struct KeyInfoCache *);
