extern tkMutex g_BkgdMutex;
extern TK_THREAD( BackGround );

struct BackGroundArgs
{
	struct KeyInfoCache    *pInfoCache;
	struct ProcessingList  *pProcList;
	struct PeerData        *pPeerDataRoot;
};
