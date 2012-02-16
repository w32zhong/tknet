
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
