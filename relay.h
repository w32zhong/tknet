#define RELAY_MERGE_RES_WAITING     0
#define RELAY_MERGE_RES_NEW_RELAY   1
#define RELAY_MERGE_RES_MERGED      2

struct RelayProc
{
	uint            RelayID;
	struct Peer     peer0,peer1;
	struct Process  proc;
	struct Sock     *pSock;
	struct ListNode ln;
};

DECLARATION_STRUCT_CONSTRUCTOR( RelayProc )

extern uint g_Relays;

void
RelayModuleInit();

void 
RelayMuduleDestruction();

uchar 
RelayProcMerge(uint ,struct NetAddr ,struct ProcessingList *,struct Iterator *, struct Iterator *,struct Sock *);

void 
RelayProcTrace();

static __inline struct TkNetMsg*
ifReadTkMsg(struct Sock *pa_pSock)
{
	DEF_AND_CAST(pMsg,struct TkNetMsg,pa_pSock->RecvBuff);
	
	if(pa_pSock->RecvLen > 0)
	{
		return pMsg;
	}
	else
	{
		return NULL;
	}
}
