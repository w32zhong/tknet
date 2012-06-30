struct SessionMaintainProcess
{
	struct Process  proc;
	struct Sock     *pSock;
	struct NetAddr  addr;
	struct pipe     *pPipe;
	BOOL            ifAlive;
};

void
SessionStart(struct NetAddr ,struct Sock *,struct ProcessingList *,struct Iterator* ,struct Iterator *);

#define SES_DAT_FLAG      TK_NET_BDG_MSG_FLAG + 1
#define SES_MAINTAIN_FLAG TK_NET_BDG_MSG_FLAG + 2