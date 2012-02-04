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
