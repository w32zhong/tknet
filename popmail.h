
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


#define MAX_MAIL_CONTENT_LEN 256 
#define POP3_PROTO_USRNAME_MAX_LEN 64
#define POP3_PROTO_PASSWORD_MAX_LEN 64

struct NetInfoMail
{
	int    num;
	char   content[MAX_MAIL_CONTENT_LEN];
	struct ListNode ln;
};

struct POP3Proc
{
	struct Process  proc;
	struct Sock     *pSock;
	int             HostIPVal;
	ushort          HostPort;
	BOOL            ifEnableSSL;
	char            UsrName[POP3_PROTO_USRNAME_MAX_LEN];
	char            PassWord[POP3_PROTO_PASSWORD_MAX_LEN];
	struct Iterator IMailsHead;
	struct Iterator IRetrieveNow;
	uint            Retrieves;
	char            MailContentBuff[SOCK_RECV_BUFF_LEN];
	BOOL			ifEnterSucc;
};


void 
MakeProtoPOP3Proc( struct POP3Proc* , const char* , ushort  , BOOL , const char* , const char* );

void 
POP3ProcFree( struct POP3Proc* );

BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( TraceMail );

EXTERN_STEP( ProtoPOP3Connect )

EXTERN_STEP( ProtoPOP3FirstRecv )

EXTERN_STEP( ProtoPOP3User )

EXTERN_STEP( ProtoPOP3Password )
