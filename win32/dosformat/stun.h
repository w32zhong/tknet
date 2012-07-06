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

#define STUN_DEFAULT_PORT 3478
#define STUN_MAX_VALUES 20

#define STUN_CHANGE_BOTH_IP_PORT  0x01
#define STUN_CHANGE_IP            0x02
#define STUN_CHANGE_PORT          0x03

struct StunHead
{
	ushort type;
	ushort length;
	uint transId1;//MagicCookie
	uint transId2;
	uint transId3;
	uint transId4;
};

struct StunAttribute
{
	ushort type;
	ushort length;
	uint   value[STUN_MAX_VALUES];
};

struct ChangeRequest
{
	struct StunHead head;
	ushort type;
	ushort length;
	uint  value;
};

int 
StunFormulateRequest(struct StunHead*);

int 
StunFormulateChangeRequest(struct ChangeRequest* , uchar);

BOOL
StunGetResult(void *, uint , uint , struct NetAddr* , struct NetAddr *);
