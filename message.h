#define PEER_NAME_ID_LEN 16

#define TK_NET_DATA_LEN 64

#define TK_NET_BDG_MSG_FLAG  0x0

#define BRIDGE_MSG_INFO_UNKNOWN           0
#define BRIDGE_MSG_INFO_REGISTER          1
#define BRIDGE_MSG_INFO_CONNECT           2
#define BRIDGE_MSG_INFO_CONNECT_ADDR      3
#define BRIDGE_MSG_INFO_HELLO             4
#define BRIDGE_MSG_INFO_OK                5
#define BRIDGE_MSG_INFO_IAM_HERE          6
#define BRIDGE_MSG_ERR_NAMEID_EXIST       7

#define READ_MSG_OPT_ANY       0x0
#define READ_MSG_OPT_SPECIFIC  0x1

struct BridgeHelloStepPa
{
	BOOL           res;
	struct NetAddr ValidAddr;
};

struct BridgeClientProcPa
{
	char *pMyNameID;
	char *pTargetNameID;
	uchar MyNatType;
};

struct BridgeProc
{
	struct PeerData *pPeerDataRoot;
	struct Process  proc;
	struct Sock     *pSock;
	struct NetAddr  WaitAddr;
	struct ProcessingList *pProcList;
	void   *Else;
};

struct BridgeMsg
{
	uchar          info;
	struct NetAddr addr;
	uint           RelayID;
	char           NameID[PEER_NAME_ID_LEN];
	uchar          NATType;
};

struct TkNetMsg
{
	uchar           flag;
	union
	{
		struct BridgeMsg BdgMsg;
		char DataMsg[TK_NET_DATA_LEN];
	}msg;
};

DECLARATION_STRUCT_CONSTRUCTOR( BridgeProc )

void 
ConsAndStartBridgeServer(struct BridgeProc * , struct PeerData * , struct ProcessingList * , struct Sock *);

void 
FreeBridgeServer(struct BridgeProc *);

void 
BridgeMakeHelloProc(struct BridgeProc *,struct BridgeHelloStepPa * , struct Sock *,struct NetAddr *);

void 
BridgeMakeClientProc(struct BridgeProc *, struct Sock * ,struct NetAddr *, char * ,uchar );
