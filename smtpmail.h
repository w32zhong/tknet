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

#define SMTP_PROTO_USRNAME_MAX_LEN POP3_PROTO_USRNAME_MAX_LEN
#define SMTP_PROTO_PASSWORD_MAX_LEN POP3_PROTO_PASSWORD_MAX_LEN
#define SMTP_PROTO_MAIL_ADDR_MAX_LEN 64 

struct SMTPProc
{
	struct Process  proc;
	struct Sock     Sock;
	int             HostIPVal;
	ushort          HostPort;
	BOOL            ifEnableSSL;
	char            UsrName[SMTP_PROTO_USRNAME_MAX_LEN];
	char            PassWord[SMTP_PROTO_PASSWORD_MAX_LEN];
	char            MailAddr[SMTP_PROTO_MAIL_ADDR_MAX_LEN];
	char            SendBuff[MAX_MAIL_CONTENT_LEN];
};

void 
SMTPProcMake( struct SMTPProc * , const char * , ushort  , BOOL  , const char * , const char * , const char * , const char * );
