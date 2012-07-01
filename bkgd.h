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
#define BKGD_CMD_MAX_LEN 128

extern BOOL            g_ifStdinToCmd;
extern tkMutex         g_BkgdMutex;
extern                 TK_THREAD( BackGround );
extern char            g_TargetName[];
extern struct NetAddr  g_BdgPeerAddr;

struct BackGroundArgs
{
	struct KeyInfoCache    *pInfoCache;
	struct ProcessingList  *pProcList;
	struct PeerData        *pPeerDataRoot;
	struct BridgeProc      *pBdgClientProc;
	struct Sock            *pMainSock;
};

BOOL 
ifBkgdStunProc();
