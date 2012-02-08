#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( PeerData ,
		out_cons->addr.port = 0;
		out_cons->addr.IPv4 = 0;
		BridgeProcCons(&out_cons->BdgProc);
		TreapCons(&out_cons->tpnd);
		out_cons->TreeLevel = 0;
		strcpy(out_cons->NameID,"unnamed");
		out_cons->NATType = NAT_T_UNKNOWN;
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

	for( i = 0 ; i < pData->TreeLevel ; i++ )
	{
		printf("  ");
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

	GetAddrText(&pData->addr,AddrBuff);

	printf("%s(%s) %c,%d,%d,%d.\n",pData->NameID,AddrBuff,PosFlagChar,
				pData->TreeLevel,pData->NATType,pData->tpnd.RanPriority);

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
