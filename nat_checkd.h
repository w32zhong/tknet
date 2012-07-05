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

#define CHECK_NAT_TRY_AGAIN   0
#define CHECK_NAT_FINE        1

#define CHECK_MAIL_BEGIN      0
#define CHECK_MAIL_SEE_IT     1
#define CHECK_MAIL_NOT_SEE    2
#define CHECK_MAIL_ERROR      3

struct CheckNATProc
{
	struct Process          proc;
	struct KeyInfoCache    *pKeyInfo;
	struct KeyInfo         *pFailedKey,*pTmpContentKey;
	struct pipe            *pCheckPipe;
	uchar                   STUNTryFlag;
	struct ProcessingList  *pProcList;
};

struct FindPossibleKeyInfoByNotNumPa
{
	int             NotNum;
	struct KeyInfo *pPossible;
};

struct CheckNATProc* 
CheckNATProcConsAndBegin(struct ProcessingList *,struct KeyInfoCache *);
