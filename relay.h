struct RelayProc
{
	uint            RelayID;
	struct Peer     peer0,peer1;
	struct Process  proc;
	struct Sock     *pSock;
	struct ListNode ln;
};

DECLARATION_STRUCT_CONSTRUCTOR( RelayProc )

extern uint g_BdgRelaysNow;

void
RelayModuleInit();

void 
RelayMuduleDestruction();

uchar 
RelayProcMerge(uint ,struct NetAddr ,struct ProcessingList *,struct Iterator *, struct Iterator *,struct Sock *);

void 
RelayProcTrace();
