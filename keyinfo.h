#define KEY_INFO_MAX_LEN MAX_MAIL_CONTENT_LEN

#define KEY_INFO_TYPE_MAILSERVER  0x01
#define KEY_INFO_TYPE_BRIDGEPEER  0x02
#define KEY_INFO_TYPE_STUNSERVER  0x03
#define KEY_INFO_TYPE_SMTPSERVER  0x04

#define KEY_INFO_VALID_UNSURE  0x04
#define KEY_INFO_VALID_NOT     0x05
#define KEY_INFO_VALID_WORKS   0x06

struct KeyInfoCache
{
	int KeyInfoNumbers;
	struct Iterator IKeyInfo;
};

struct KeyInfo
{
	int      num;
	uchar    valid;
	uchar    type;
	char     text[KEY_INFO_MAX_LEN];
	struct NetAddr  addr;
	struct ListNode ln;
};

void 
KeyInfoReadFile( struct KeyInfoCache * , const char * );

void 
KeyInfoWriteFile( struct KeyInfoCache * , const char * );

void 
KeyInfoTrace( struct KeyInfoCache * );

void 
KeyInfoFree(struct KeyInfoCache *);

struct KeyInfo*
KeyInfoSelectA( struct KeyInfoCache * , uchar  );

void
KeyInfoWorksFine( struct KeyInfoCache * , int  );

void
KeyInfoUpdate( struct KeyInfoCache * );

BOOL
KeyInfoUse( struct KeyInfo * , struct KeyInfoCache * );

BOOL 
KeyInfoTry(struct KeyInfoCache * , uchar );

char*
GetNextSeparateStr(char ** );

DECLARATION_STRUCT_CONSTRUCTOR( KeyInfoCache )

struct FindKeyInfoByTypePa
{
	uchar TypeToFind;
	struct KeyInfo *found;
};

struct FindKeyInfoByNumPa
{
	int NumToFind;
	struct KeyInfo *found;
};

struct FindKeyInfoByValidPa
{
	uchar valid;
	struct KeyInfo *found;
};

struct FindKeyInfoByAddrPa
{
	struct NetAddr addr;
	struct KeyInfo *found;
};

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByType);

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByNum);

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByValid);

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByAddr);
