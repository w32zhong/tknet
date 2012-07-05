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

struct SessionMaintainProcess
{
	struct Process  proc;
	struct Sock     *pSock;
	struct NetAddr  addr;
	struct pipe     *pPipe;
	BOOL            ifAlive;
};

void
SessionStart(struct NetAddr ,struct Sock *,struct ProcessingList *,struct Iterator* ,struct Iterator *);

#define SES_DAT_FLAG      TK_NET_BDG_MSG_FLAG + 1
#define SES_MAINTAIN_FLAG TK_NET_BDG_MSG_FLAG + 2
#define SES_CMD_FLAG      TK_NET_BDG_MSG_FLAG + 3
#define SES_CHAT_FLAG     TK_NET_BDG_MSG_FLAG + 4

void
MkCmdModePipe();

void
MkChatModePipe();
