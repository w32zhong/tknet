/*
*      This file is part of the tknet project. 
*    which be used under the terms of the GNU General Public 
*    License version 3.0 as published by the Free Software
*    Foundation and appearing in the file LICENSE.GPL included 
*    in the packaging of this file.  Please review the following 
*    information to ensure the GNU General Public License 
*    version 3.0 requirements will be met: 
*    http://www.gnu.org/copyleft/gpl.html
*
*    Copyright  (C)   2012   Zhong Wei <clock126@126.com>  .
*/ 

#define PEER_NAME_ID_LEN  16

#define TK_NET_DATA_LEN   128

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

#define BRIDGE_MSG_INFO_WAIT_RELAY        20

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
	char           *pMyNameID;
	const char    **ppTargetNameID;
	struct NetAddr  DirectConnectAddr;
	BOOL            ifSkipRegister;
	BOOL            ifFastSendWait;//everytime client go to send "waiting" 
			//message letting server begain the connection decision,
			//it is too slow. We use this flag to let client send immediately
			//after he returns to the wait step.
	BOOL             ifConnected;//if server client are disconnected for some reasons,
			//we set this flag to zero, then the main loop will know this situation,
			//and may try to connect another server.
};

struct Peer
{
	char           NameID[PEER_NAME_ID_LEN];
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
	char            PunAddrNameID[PEER_NAME_ID_LEN];
	char            ConAddrNameID[PEER_NAME_ID_LEN];
	struct NetAddr addr;
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
	ushort          UsrDatLen;//Only used when msg union stands for UsrDat
	                          //(flag is not TK_NET_BDG_MSG_FLAG).
	union
	{
		struct BridgeMsg  BdgMsg;
		char              UsrDat[TK_NET_DATA_LEN];
	}msg;
};

DECLARATION_STRUCT_CONSTRUCTOR( BridgeProc )
DECLARATION_STRUCT_CONSTRUCTOR( Peer )

void 
ConsAndStartBridgeServer(struct BridgeProc * , struct PeerData * , 
		struct ProcessingList * , struct Sock *,struct Iterator *);

void 
FreeBridgeServer(struct BridgeProc *);

void 
FreeSubBridgeServerTemplate();

void 
BridgeClientTryBdgServerProc(struct BridgeProc *,struct BridgeHelloStepPa * , struct Sock *);

struct BridgeClientProcPa*
BridgeMakeClientProc(struct BridgeProc *, struct Sock *,struct ProcessingList *,
		struct NetAddr *, char * ,uchar , const char** ,BOOL);

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

typedef void (*CONNECT_CALLBK)(struct NetAddr,struct Sock*,struct ProcessingList*, 
		struct Iterator*,struct Iterator*);

extern CONNECT_CALLBK g_ConnectionNotify;

#define ON_CONNECT() \
	OnConnect(struct NetAddr pa_addr,struct Sock *pa_pSock,struct ProcessingList *pa_pProcList \
			,struct Iterator* pa_pINow,struct Iterator *pa_pIForward)

static __inline void 
PeerTrace( struct Peer *pa_pPeer )
{
	char AddrText[32];
	GetAddrText(&pa_pPeer->addr,AddrText);
	PROMPT(Usual,"%s(%s)",pa_pPeer->NameID,AddrText);
}

static __inline void 
BetweenMacro( struct Peer *pa_pPeer0, struct Peer *pa_pPeer1 )
{
	PROMPT(Usual,"between ");
	PeerTrace( pa_pPeer0 );
	PROMPT(Usual," and ");
	PeerTrace( pa_pPeer1 );
	PROMPT(Usual," . \n");
}
