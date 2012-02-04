#include "headers.h"

struct NetAddr sta_addr;

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
SetPortRes( ushort pa_port )
{
	VCK( sta_addr.port != 0 && sta_addr.port != pa_port ,;);
	sta_addr.port = pa_port;
}

static void 
SetIPRes( uint pa_IPVal )
{
	VCK( sta_addr.IPv4 != 0 && sta_addr.IPv4 != pa_IPVal ,;);
	sta_addr.IPv4 = pa_IPVal;
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
			SetPortRes((ushort)(pa_val & 0xffff));
			break;
		case 1:
			SetIPRes(pa_val);
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
			SetPortRes((ushort)(pa_val & 0xffff));
			break;
		case 1:
			pa_val ^= pa_MagicCookie;
			SetIPRes(pa_val);
			break;

		default:
			break;
		}
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

struct NetAddr
StunGetPublicNetAddr(void *in_data, uint pa_len , uint pa_MagicCookie)
{
	uint now; //offset from head (in bytes)
	uint AttrLen = StunGetAttrLenFromHead(in_data , pa_MagicCookie);
	memset(&sta_addr , 0 , sizeof(sta_addr));
	
	VCK(AttrLen != pa_len - sizeof(struct StunHead) , goto ret; );

	now = sizeof(struct StunHead);
	while( now < pa_len )
	{
		StunGetAttributes((char*)in_data + now,  &now , pa_MagicCookie);
	}

	VCK( sta_addr.IPv4 == 0 ||
		sta_addr.port == 0 ,;);

ret:
	return sta_addr;
}
