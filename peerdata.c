#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( PeerData ,
		out_cons->addr.port = 0;
		out_cons->addr.IPv4 = 0;
		BridgeProcCons(&out_cons->BdgProc);
		TreapCons(&out_cons->tpnd);
		out_cons->TreeLevel = 0;
		strcpy(out_cons->NameID,"unnamed");
		out_cons->NATType = NAT_T_UNKNOWN;
		out_cons->pSeedPeer = NULL;
		)

static BOOL
ifPeerDataTreapNodeEqual(struct BinTreeNode* pa_pBinNode0 ,
		struct BinTreeNode* pa_pBinNode1 , void* pa_else)
{
	struct PeerData* p0 = GET_STRUCT_ADDR( pa_pBinNode0 , struct PeerData , tpnd.btnd );
	struct PeerData* p1 = GET_STRUCT_ADDR( pa_pBinNode1 , struct PeerData , tpnd.btnd );

	return ( 0 == strcmp(p0->NameID,p1->NameID) );
}

static BOOL 
PeerDataTreapNodeCompare(struct BinTreeNode* pa_pBinNode0 ,
		struct BinTreeNode* pa_pBinNode1 , void* pa_else)
{
	struct PeerData* p0 = GET_STRUCT_ADDR( pa_pBinNode0 , struct PeerData , tpnd.btnd );
	struct PeerData* p1 = GET_STRUCT_ADDR( pa_pBinNode1 , struct PeerData , tpnd.btnd );
	uint i;
	for( i = 0; i < PEER_NAME_ID_LEN ; i++)
	{
		if(p0->NameID[i] == p1->NameID[i])
		{
			continue;
		}
		else
		{
			return p0->NameID[i] > p1->NameID[i];
		}
	}

	return 0;
}

void 
PeerDataInsert(struct PeerData *pa_pPD,struct PeerData *pa_pRoot)
{
	TreapInsert(&pa_pPD->tpnd,&pa_pRoot->tpnd,&PeerDataTreapNodeCompare,NULL);
}

struct PeerData*
PeerDataFind(struct PeerData *pa_pRoot,char *pa_pNameToFind)
{
	struct PeerData ToFind;
	struct BinTreeNode *pFound;
	strcpy(ToFind.NameID,pa_pNameToFind);

	pFound = BinTreeFind(&(pa_pRoot->tpnd.btnd),&ToFind.tpnd.btnd,
			&PeerDataTreapNodeCompare,&ifPeerDataTreapNodeEqual,NULL);

	if(pFound)
	{
		return GET_STRUCT_ADDR( pFound , struct PeerData , tpnd.btnd );
	}
	else
	{
		return NULL;
	}
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( TracePeerData )
{
	struct Branch* pBranch =
		GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Branch , ln);

	struct PeerData* pData =
		GET_STRUCT_ADDR( pBranch->pChild , struct PeerData , tpnd.btnd.tnd);

	uint  i;
	char PosFlagChar;
	struct BinTreeNode *pBinFather;
	char AddrBuff[32];
	char SeedStr[16];

	for( i = 0 ; i < pData->TreeLevel ; i++ )
	{
		printf("--");
	}

	pBinFather = GET_STRUCT_ADDR(pData->tpnd.btnd.tnd.pFather, struct BinTreeNode , tnd);

	if(pBinFather->RightChild == &(pData->tpnd.btnd))
	{
		PosFlagChar = 'R';
	}
	else if(pBinFather->LeftChild == &(pData->tpnd.btnd))
	{
		PosFlagChar = 'L';
	}
	else
	{
		PosFlagChar = '?';
	}

	if(pData->pSeedPeer != NULL)
	{
		sprintf(SeedStr,"(Seed,%d)",(uint)pData->pSeedPeer->Relays);
	}
	else
	{
		SeedStr[0] = '\0';
	}

	GetAddrText(&pData->addr,AddrBuff);

	printf("%s(%s) %c,Level%d,NAT%d%s,RAN%d.\n",pData->NameID,AddrBuff,PosFlagChar,
				pData->TreeLevel,pData->NATType,SeedStr,pData->tpnd.RanPriority);

	return pa_pINow->now == pa_pIHead->last;
}

static void 
PeerDataSetTreeLevel(struct TreeNode* pa_pTnd , uint pa_level)
{
	struct PeerData *pData = 
		GET_STRUCT_ADDR( pa_pTnd , struct PeerData , tpnd.btnd.tnd );

	pData->TreeLevel = pa_level;
}

void
PeerDataTrace(struct PeerData *pa_pRoot)
{
	TreeGetNodesLevel(&(pa_pRoot->tpnd.btnd.tnd),&PeerDataSetTreeLevel);
	Traversal(&(pa_pRoot->tpnd.btnd.tnd),&PreorderDFS,&TracePeerData,NULL);
}

static
DEF_FREE_TREE_ELEMENT_CALLBACK_FUNCTION( FreePeerData , struct PeerData , tpnd.btnd.tnd ,;)

void 
PeerDataDestroy(struct PeerData *pa_pRoot)
{
	Traversal(&(pa_pRoot->tpnd.btnd.tnd),&PostorderDFS,&FreePeerData,NULL);
}

void 
PeerDataSelectAsSeed(struct PeerData* pa_pPD , struct Iterator *pa_pISeedList)
{
	struct SeedPeer *pSP = tkmalloc(struct SeedPeer);
	
	ListNodeCons(&pSP->ln);
	pSP->pPD = pa_pPD;
	pSP->Relays = 0;
	
	AddOneToListTail(pa_pISeedList,&pSP->ln);

	pa_pPD->pSeedPeer = pSP;
}

static 
DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION(FreeSeedPeer,struct SeedPeer,ln,;)

void 
PeerDataDele(struct PeerData *pa_pPD, struct Iterator *pa_pISeedList)
{
	struct Iterator *pIHead,IForward,INow;
	TreapDragOut(&pa_pPD->tpnd);
	
	if( pa_pPD->pSeedPeer == NULL )
	{
		goto free_PD;
	}

	pIHead = pa_pISeedList;
	INow = GetIterator(&pa_pPD->pSeedPeer->ln);
	IForward = GetIterator(INow.now->next);

	FreeSeedPeer(pIHead,&INow,&IForward,NULL);

free_PD:
	
	tkfree(pa_pPD);
}

void 
PeerDataUpdateSeedInfo(struct PeerData *pa_pPD,uchar pa_relays)
{
	if( pa_pPD->pSeedPeer == NULL )
	{
		return;
	}

	pa_pPD->pSeedPeer->Relays = pa_relays;
}

struct FindSeedPeerOfLeastRelaysPa
{
	struct SeedPeer *pFound;
	uchar           LeastRelays;
};

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindSeedPeerOfLeastRelays)
{
	struct SeedPeer *pSP = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct SeedPeer,ln);
	DEF_AND_CAST(pFSPOLRPa ,struct FindSeedPeerOfLeastRelaysPa ,pa_else);

	if( pSP->Relays <= pFSPOLRPa->LeastRelays )
	{
		pFSPOLRPa->pFound = pSP;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

struct PeerData * 
SeedPeerSelectOne(struct Iterator *pa_pISeedList)
{
	struct FindSeedPeerOfLeastRelaysPa FSPOLRPa;
	FSPOLRPa.LeastRelays = 0;

	while(1)
	{
		FSPOLRPa.pFound = NULL;
		
		ForEach(pa_pISeedList,&FindSeedPeerOfLeastRelays,&FSPOLRPa);
		
		if(FSPOLRPa.pFound)
		{
			return FSPOLRPa.pFound->pPD;
		}
		
		FSPOLRPa.LeastRelays ++;
	}
}
