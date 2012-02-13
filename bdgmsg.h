#define PEER_NAME_ID_LEN 16

#define TK_NET_DATA_LEN 64

#define TK_NET_BDG_MSG_FLAG     0

#define BDG_READ_OPT_ANY             0
#define BDG_READ_OPT_ADDR_FILTER     1
#define BDG_READ_OPT_MSG_FILTER      2

#define BRIDGE_MSG_INFO_UNKNOWN           0

#define BRIDGE_MSG_INFO_HELLO_BDG         1
#define BRIDGE_MSG_INFO_IAM_HERE          2

#define BRIDGE_MSG_INFO_REGISTER          3
#define BRIDGE_MSG_ERR_NAMEID_EXIST       4
#define BRIDGE_MSG_INFO_RGST_OK           5

#define BRIDGE_MSG_INFO_WAITING           6
#define BRIDGE_MSG_INFO_ECHO              7

#define BRIDGE_MSG_INFO_CONNECT           8
#define BRIDGE_MSG_INFO_CONNECT_BEGIN     9
#define BRIDGE_MSG_ERR_NO_NAMEID          10

#define BRIDGE_MSG_INFO_PUNCHING          11
#define BRIDGE_MSG_INFO_PUNCHING_FINISH   12

#define BRIDGE_MSG_INFO_CONNECT_ADDR      13
#define BRIDGE_MSG_ERR_CONNECT_ADDR       14

#define BRIDGE_MSG_INFO_HELLO             15
#define BRIDGE_MSG_INFO_ESTABLISHED       16

#define BRIDGE_MSG_ERR_NO_SEED_TO_RELAY   17
#define BRIDGE_MSG_ERR_ERROR              18
#define BRIDGE_MSG_INFO_ACKNOWLEDGE       19


#define CONNECT_DECISION_FLAG_BEGIN          1
#define CONNECT_DECISION_FLAG_DIRECT         2
#define CONNECT_DECISION_FLAG_ERR            3
#define CONNECT_DECISION_FLAG_A_SIDE_RELAY   4
#define CONNECT_DECISION_FLAG_B_SIDE_RELAY   5

struct BridgeHelloStepPa
{
	BOOL           res;
	struct NetAddr addr;
};

struct BridgeClientProcPa
{
	char *pMyNameID;
	char *pTargetNameID;
};

struct Peer
{
	struct NetAddr addr;
	uchar          NATType;
};

struct BridgeProc
{
	struct PeerData *pPeerDataRoot;
	struct Iterator *pSeedPeerCache;
	struct Process  proc;
	struct Sock     *pSock;
	struct ProcessingList *pProcList;
	struct Peer     s,a,b,sx;
	uint            DecisionRelayID;
	uchar           DecisionFlag;
	struct NetAddr  DecisionPunAddr,DecisionConAddr;
	struct NetAddr  MultiSendTo;
	uchar           MultiSendInfo;
	uchar           ErrCode;
	void   *Else;
};


struct BridgeMsg
{
	uchar          info;
	struct NetAddr addr;
	uint           RelayID;
	char           NameID[PEER_NAME_ID_LEN];
	uchar          NATType;
	uchar          Relays;
	uchar          ErrCode;
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
DECLARATION_STRUCT_CONSTRUCTOR( Peer )

void 
ConsAndStartBridgeServer(struct BridgeProc * , struct PeerData * , struct ProcessingList * , struct Sock *,struct Iterator *);

void 
FreeBridgeServer(struct BridgeProc *);

void 
BridgeClientTryBdgServerProc(struct BridgeProc *,struct BridgeHelloStepPa * , struct Sock *);

void 
BridgeMakeClientProc(struct BridgeProc *, struct Sock * ,struct NetAddr *, char * ,uchar , char*);

void 
FreeBdgClientProc(struct BridgeProc *);

void
SetPeerByPeerData(struct Peer *,struct PeerData *);

#define BDG_ADDR( _name , _p_bdg_proc ) \
	&( _p_bdg_proc-> _name .addr )

#define IF_NEAD_RELAY( _NAT0 , _NAT1 ) ( _NAT0 + _NAT1 >= 5)

struct BridgeMsg*
BdgMsgRead(struct Process * , uchar , uchar , struct NetAddr *);

void
BdgMsgWrite(struct Process * ,struct BridgeMsg * , struct NetAddr *);

struct PeerData*
NewPeerDataWithBdgProc(struct NetAddr ,uchar ,char *,struct BridgeProc *);

void 
BdgSubServerProcInit();

EXTERN_STEP( BdgClientTryBdgServer )

EXTERN_STEP( BridgeMain )
EXTERN_STEP( BdgBeginSubServer )
EXTERN_STEP( BdgConnectRequireServer )
EXTERN_STEP( BdgConnectRequireReply )
EXTERN_STEP( BdgConnectDecision )
EXTERN_STEP( BdgPunchingServer )
EXTERN_STEP( BdgConnectAddrServer )
EXTERN_STEP( BdgErrReturnServer )

EXTERN_STEP( BdgClientRegister )
EXTERN_STEP( BdgClientWait )
EXTERN_STEP( BdgClientConnectRequire )
EXTERN_STEP( BdgClientDoConnectAddr )
EXTERN_STEP( BdgClientMultiSendNotify )
