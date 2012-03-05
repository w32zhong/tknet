
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


#include "tknet.h"

#define OPT_MAPADDR_RES     0
#define OPT_CHANGEADDR_RES  1

struct NetAddr sta_MapAddrRes;
struct NetAddr sta_ChangeAddrRes;

int 
StunFormulateRequest(struct StunHead* out_head)
{
	uint MagicCookie = tkGetRandom();
	out_head->type   = htons(0x0001);
	out_head->length = 0;
	out_head->transId1 = htonl(MagicCookie);
	out_head->transId2 = tkGetRandom();
	out_head->transId3 = tkGetRandom();
	out_head->transId4 = tkGetRandom();

	return MagicCookie;
}

int 
StunFormulateChangeRequest(struct ChangeRequest* out_head , uchar pa_opt)
{
	uint MagicCookie = tkGetRandom();
	out_head->head.type   = htons(0x0001);
	out_head->head.length = htons(0x0008);
	out_head->head.transId1 = htonl(MagicCookie);
	out_head->type = htons(0x0003);
	out_head->length = htons(0x0004);

	switch( pa_opt )
	{
	case STUN_CHANGE_BOTH_IP_PORT:
		out_head->value = htonl(0x6);
		break;

	case STUN_CHANGE_IP:
		out_head->value = htonl(0x4);
		break;
	
	case STUN_CHANGE_PORT:
		out_head->value = htonl(0x2);
		break;
	
	default:
		TK_EXCEPTION("STUN option");
	}

	return MagicCookie;
}

static void 
SetPortRes( uchar pa_ResTypeOpt , ushort pa_port )
{
	if(pa_ResTypeOpt == OPT_MAPADDR_RES)
	{
		VCK( sta_MapAddrRes.port != 0 && sta_MapAddrRes.port != pa_port ,;);
		sta_MapAddrRes.port = pa_port;
	}
	else if(pa_ResTypeOpt == OPT_CHANGEADDR_RES)
	{
		VCK( sta_ChangeAddrRes.port != 0 && sta_ChangeAddrRes.port != pa_port ,;);
		sta_ChangeAddrRes.port = pa_port;
	}
	else
	{
		TK_EXCEPTION("ResTypeOpt");
	}
}

static void 
SetIPRes( uchar pa_ResTypeOpt , uint pa_IPVal )
{
	if(pa_ResTypeOpt == OPT_MAPADDR_RES)
	{
		VCK( sta_MapAddrRes.IPv4 != 0 && sta_MapAddrRes.IPv4 != pa_IPVal ,;);
		sta_MapAddrRes.IPv4 = pa_IPVal;
	}
	else if(pa_ResTypeOpt == OPT_CHANGEADDR_RES)
	{
		VCK( sta_ChangeAddrRes.IPv4 != 0 && sta_ChangeAddrRes.IPv4 != pa_IPVal ,;);
		sta_ChangeAddrRes.IPv4 = pa_IPVal;
	}
	else
	{
		TK_EXCEPTION("ResTypeOpt");
	}
}

static void 
StunGetValue(ushort pa_attr, uint pa_i , int pa_val , uint pa_MagicCookie)
{
	if(pa_attr == 0x0001)
	{
		//MAPPED-ADDRESS
		switch( pa_i )
		{
		case 0:
			VCK( pa_val>>16 != 0x1 , break; );//not IPv4
			SetPortRes(OPT_MAPADDR_RES,(ushort)(pa_val & 0xffff));
			break;
		case 1:
			SetIPRes(OPT_MAPADDR_RES,pa_val);
			break;

		default:
			break;
		}
	}
	else if( pa_attr == 0x8020 || pa_attr == 0x0020 )
	{
		//XOR-MAPPED-ADDRESS
		switch( pa_i )
		{
		case 0:
			VCK( pa_val>>16 != 0x1 , break; );//not IPv4
			pa_val ^= pa_MagicCookie >> 16;
			SetPortRes(OPT_MAPADDR_RES,(ushort)(pa_val & 0xffff));
			break;
		case 1:
			pa_val ^= pa_MagicCookie;
			SetIPRes(OPT_MAPADDR_RES,pa_val);
			break;

		default:
			break;
		}
	}
	else if( pa_attr == 0x0005 )
	{
		//CHANGED-ADDRESS
		switch( pa_i )
		{
		case 0:
			VCK( pa_val>>16 != 0x1 , break; );//not IPv4
			SetPortRes(OPT_CHANGEADDR_RES,(ushort)(pa_val & 0xffff));
			break;
		case 1:
			SetIPRes(OPT_CHANGEADDR_RES,pa_val);
			break;

		default:
			break;
		}
		
		printf("CHANGED-ADDRESS attribute recved\n");
	}
	else if( pa_attr == 0x0009 )
	{
		//ERROR-CODE
		TK_EXCEPTION("STUN err return");
	}
}

static void 
StunGetAttributes(void *in_data , uint *out_acc , uint pa_MagicCookie)
{
	uint i,cont,AttrLen;
	DEF_AND_CAST( pAttr , struct StunAttribute , in_data );

	AttrLen = ntohs(pAttr->length);
	cont = AttrLen / 4;
	VCK( (AttrLen % 4 != 0) || 
		 cont > STUN_MAX_VALUES , return; );

	*out_acc += sizeof(short) + sizeof(short);
	for( i = 0 ; i < cont ; i++ )
	{
		StunGetValue( ntohs(pAttr->type) , i , ntohl(pAttr->value[i]) ,pa_MagicCookie);
		*out_acc += sizeof(int);
	}
}

static uint 
StunGetAttrLenFromHead( void *in_data , uint pa_MagicCookie )
{
	DEF_AND_CAST( pHead , struct StunHead , in_data );

	//VCK( ntohl(pHead->transId1) == pa_MagicCookie , ; );
	VCK( (pHead->transId1) == pa_MagicCookie , ; );
	//I don't know why not use the line commented instead.

	VCK( ntohs(pHead->type) != 0x101 , return 0; );

	return ntohs( pHead->length );
}

BOOL
StunGetResult(void *in_data, uint pa_len , uint pa_MagicCookie , 
		struct NetAddr *out_MapAddrRes , struct NetAddr *out_ChangeAddrRes)
{
	uint now; //offset from head (in bytes)
	uint AttrLen = StunGetAttrLenFromHead(in_data , pa_MagicCookie);
	
	memset(&sta_MapAddrRes , 0 , sizeof(sta_MapAddrRes));
	memset(&sta_ChangeAddrRes , 0 , sizeof(sta_ChangeAddrRes));
	
	VCK(AttrLen != pa_len - sizeof(struct StunHead) , return 0; );

	now = sizeof(struct StunHead);
	while( now < pa_len )
	{
		StunGetAttributes((char*)in_data + now,  &now , pa_MagicCookie);
	}

	VCK( sta_MapAddrRes.IPv4 == 0 ||
		sta_MapAddrRes.port == 0 ,;);
	
	*out_MapAddrRes = sta_MapAddrRes;

	VCK( sta_ChangeAddrRes.IPv4 == 0 ||
		sta_ChangeAddrRes.port == 0 ,;);
	
	*out_ChangeAddrRes = sta_ChangeAddrRes;

	return 1;
}
