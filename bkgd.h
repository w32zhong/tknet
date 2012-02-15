extern tkMutex g_BkgdMutex;
extern TK_THREAD( BackGround );
extern char g_TargetName[];
extern struct NetAddr g_BdgPeerAddr;

struct BackGroundArgs
{
	struct KeyInfoCache    *pInfoCache;
	struct ProcessingList  *pProcList;
	struct PeerData        *pPeerDataRoot;
	struct BridgeProc      *pBdgClientProc;
};

void
BkgdEnterSubProcess();

void
BkgdLeaveSubProcess();

char*
BkgdGetBackGroundMsg();
