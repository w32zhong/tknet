#include "tknet.h"

#define PIPE_NAME_MAXLEN 23
//Now, the max len is defined by the 
//longest possible name string:
//"123.123.123.123/12345\0"

typedef void (*PipeFlowCallbk)(char* ,uint ,void* ,void*);

#define FLOW_CALLBK_FUNCTION( _fun_name ) \
	void _fun_name (char* pa_pData,uint pa_DataLen,void *pa_pFlowPa,void* pa_else)

struct pipe
{
	PipeFlowCallbk  FlowCallbk;
	void            *pFlowPa;//freed by pipe.
	char            name[PIPE_NAME_MAXLEN];
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

struct PipeReference
{
	struct PipeDirection *pDirection;
	struct ListNode  ln;
};

struct FindPipeByName
{
	char        *name;
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
