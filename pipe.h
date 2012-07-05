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

#include "tknet.h"

#define PIPE_NAME_MAXLEN 23
//Now, the max len is defined by the 
//longest possible name string:
//"123.123.123.123/12345\0"

#define FLOW_PAELSE_NAME_LEN 32

struct pipe;

typedef void (*PipeFlowCallbk)(char* ,uint ,struct pipe*,
		void* ,void*);

#define FLOW_CALLBK_FUNCTION( _fun_name ) \
	void _fun_name (char* pa_pData,uint pa_DataLen,struct pipe *pa_pPipe, \
			void *pa_pFlowPa,void* pa_else)

struct pipe
{
	PipeFlowCallbk  FlowCallbk;
	void            *pFlowPa;//freed by pipe.
	char            name[PIPE_NAME_MAXLEN];
	uint            id;
	struct ListNode ln;
	struct Iterator IDirection;
	struct Iterator IReference;
};

struct connection
{
	struct Iterator    *pIterator;
	struct connection  *pCounterPart;
	struct ListNode    ln;
};

struct FindPipeByName
{
	char        *name;
	struct pipe *found;
};

struct FindPipeByID
{
	uint        id;
	struct pipe *found;
};

struct PipeMap
{
	struct Iterator IPipe;
};

struct FlowCallbkPa
{
	char *pData;
	uint DataLen;
	void *Else;
};

struct FlowPaElse
{
	char PaName[FLOW_PAELSE_NAME_LEN];
	void *pPa;
};

struct IfPipeToPa
{
	struct pipe* pTo;
	BOOL         res;
};

void
MakeConnection(struct Iterator *,struct Iterator *);

void
CutConnections(struct Iterator *);

BOOL
PipeDirectOnlyTo(struct pipe *,struct pipe *);

BOOL
PipeDirectTo(struct pipe *,struct pipe *);

void
PipeDele(struct pipe *);

struct pipe*
PipeFindByName(char *);

struct pipe*
PipeMap(char *);

void
PipeTablePrint();

void
PipeFlow(struct pipe *,char *,uint ,void *);

void 
PipeModuleInit();

void 
PipeModuleUninit();

BOOL 
ifPipeTo(struct pipe *,struct pipe *);

struct pipe*
PipeFindByID(uint);
