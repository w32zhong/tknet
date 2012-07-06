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

#define KEY_INFO_MAX_LEN MAX_MAIL_CONTENT_LEN

#define KEY_INFO_TYPE_CONFIG      0x00
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

uchar
KeyInfoReadFile( struct KeyInfoCache * , const char * );

void 
KeyInfoWriteFile( struct KeyInfoCache * , const char * );

void 
KeyInfoTrace( struct KeyInfoCache * );

void 
KeyInfoFree(struct KeyInfoCache *);

void
KeyInfoWorksFine( struct KeyInfoCache * , int  );

struct KeyInfo*
KeyInfoFindByType( struct KeyInfoCache *, uchar );
//different from KeyInfoSelectA() which takes 'valide' into consideration,
//this function only use type as filter.

void
KeyInfoUpdate( struct KeyInfoCache * );

BOOL
KeyInfoUse( struct KeyInfo * , struct KeyInfoCache * , struct Sock* );

BOOL 
KeyInfoTry(struct KeyInfoCache * , uchar , struct Sock* );

BOOL 
KeyInfoDoubleCheckNAT(struct KeyInfoCache *, struct Sock *);

char*
GetNextSeparateStr(char ** );

struct KeyInfo*
NewKeyInfoFromStrLine(char *);

struct KeyInfo*
KeyInfoInsert(struct KeyInfo *,struct KeyInfoCache *);
//insert a key info, if a same address info existed, return it.

void 
KeyInfoDele(struct KeyInfo *,struct KeyInfoCache *);

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
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByNum);

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByValid);

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindKeyInfoByAddr);

extern char g_TargetName[];
extern char g_MyName[];
extern BOOL g_ifConfigAsFullCone;
