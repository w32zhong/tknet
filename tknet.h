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

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include "head.h"
#include "sysi.h"

#include "comdef.h"

#include "list.h"
#include "tree.h"
#include "treap.h"

//#define TK_CONFIG_SOCK_SSL_ENABLE
#include "sock.h"

#include "netaddr.h"
#include "netproc.h"

#include "popmail.h"
#include "base64.h"
#include "smtpmail.h"

#include "stun.h"
#include "nattype.h"

struct PeerData;
#include "bdgmsg.h"
#include "relay.h"
#include "peerdata.h"

#include "keyinfo.h"
#include "bkgd.h"

void 
ON_CONNECT();

void 
tkNetConnect(const char *);

int 
tkNetMain(int ,char **);
