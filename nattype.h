
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


#define NAT_T_FULL_CONE       0x00
#define NAT_T_RESTRICTED      0x01
#define NAT_T_PORT_RESTRICTED 0x02
#define NAT_T_SYMMETRIC       0x03
#define NAT_T_UNKNOWN         0x04

struct STUNProc
{
	struct Process proc;
	struct Sock    *pSock;
	int            HostIPVal;
	ushort         HostPort;
	uint           MagicCookieTemp;
	struct NetAddr MapAddrTemp;
	struct NetAddr ServerAddrTemp;
	uchar          NatTypeRes;
};

void 
MakeProtoStunProc( struct STUNProc * ,struct Sock * , const char * , ushort );
