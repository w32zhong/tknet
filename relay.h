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

#define RELAY_MERGE_RES_WAITING     0
#define RELAY_MERGE_RES_NEW_RELAY   1
#define RELAY_MERGE_RES_MERGED      2

struct RelayProc
{
	uint            RelayID;
	struct Peer     peer0,peer1;
	struct Process  proc;
	struct Sock     *pSock;
	struct ListNode ln;
};

DECLARATION_STRUCT_CONSTRUCTOR( RelayProc )

extern uint g_Relays;

void
RelayModuleInit();

void 
RelayMuduleDestruction();

uchar 
RelayProcMerge(uint ,struct NetAddr ,struct ProcessingList *,struct Iterator *, struct Iterator *,struct Sock *);

void 
RelayProcTrace();
