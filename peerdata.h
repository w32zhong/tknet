#define MAX_RELAYS_TO_SELECT_SEED 2

struct SeedPeer;

struct PeerData
{
	struct NetAddr     addr;
	struct BridgeProc  BdgProc;
	struct Treap       tpnd;
	uint               TreeLevel;
	char               NameID[PEER_NAME_ID_LEN];
	uchar              NATType;
	struct SeedPeer    *pSeedPeer;
};

struct SeedPeer
{
	struct ListNode ln;
	struct PeerData *pPD;
	uchar           Relays;
};

DECLARATION_STRUCT_CONSTRUCTOR( PeerData )

void 
PeerDataInsert(struct PeerData *,struct PeerData *);

struct PeerData*
PeerDataFind(struct PeerData *,char *);

void
PeerDataTrace(struct PeerData *);

void 
PeerDataDestroy(struct PeerData *,struct Iterator *);

void 
PeerDataSelectAsSeed(struct PeerData*  , struct Iterator *);

void 
PeerDataDele(struct PeerData *, struct Iterator *);

void 
PeerDataUpdateSeedInfo(struct PeerData *,uchar );

struct PeerData * 
SeedPeerSelectOne(struct Iterator *);
