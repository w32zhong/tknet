struct PeerData
{
	struct NetAddr     addr;
	struct BridgeProc  BdgProc;
	struct Treap       tpnd;
	uint               TreeLevel;
	char               NameID[PEER_NAME_ID_LEN];
	uchar              NATType;
};

DECLARATION_STRUCT_CONSTRUCTOR( PeerData )

void 
PeerDataInsert(struct PeerData *,struct PeerData *);

struct PeerData*
PeerDataFind(struct PeerData *,char *);

void
PeerDataTrace(struct PeerData *);

void 
PeerDataDestroy(struct PeerData *);
