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

EXTERN_STEP( ProtoPOP3Connect );

EXTERN_STEP( ProtoPOP3FirstRecv );

EXTERN_STEP( ProtoPOP3User );

EXTERN_STEP( ProtoPOP3Password );
