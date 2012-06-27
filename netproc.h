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

#define PS_STATE_FIRST_TIME  0x00
#define PS_STATE_NORMAL      0x01
#define PS_STATE_OVERTIME    0x02
#define PS_STATE_LAST_TIME   0x03

#define PS_CALLBK_RET_ABORT    (0x00|0x80)
#define PS_CALLBK_RET_GO_ON    (0x01|0x80)
#define PS_CALLBK_RET_DONE     (0x02|0x80)
#define PS_CALLBK_RET_REDO     (0x03|0x80)
#define PS_CALLBK_RET_TO_END   (0x04|0x80)

struct ProcessingList
{
	struct Iterator  IUndergoProcess;
};

struct Process
{
	struct Iterator  IProcessHead;
	struct Iterator  IProcessNow;
	struct ListNode  UndergoLN;
	long             CurrentStepStartTime;
	uchar            CurrentStepRetrys;
	BOOL             isCurrentStepFirstTime;
	uchar            steps;

	void (*NotifyCallbk)( struct Process * );
};

typedef uchar (*StepCallbk)( struct Process* , uchar, struct Iterator*, struct Iterator*);

#define STEP( _step_name ) \
	const char ProcessStepName ## _step_name [] = # _step_name ; \
	uchar ProcessStep ## _step_name (struct Process* pa_pProc , uchar pa_state , struct Iterator* pa_pINow, struct Iterator* pa_pIForward )

#define EXTERN_STEP( _step_name ) \
	extern const char ProcessStepName ## _step_name []; \
	uchar ProcessStep ## _step_name (struct Process* , uchar , struct Iterator*, struct Iterator*);

#define PROCESS_ADD_STEP( _pProc , _step , _WaitLevel ) \
	ProcessAddStep( _pProc , & ( ProcessStep ## _step ) , &( _WaitLevel .pInterval) , &( _WaitLevel .pRetrys) , ProcessStepName ## _step )

#define TKNET_WAIT_LEVELS  5
#define TKNET_CONDITIONS   3

struct WatiLevel
{
	long   *pInterval;
	uchar  *pRetrys;
};

struct ProcessStep
{
	uchar            FlagNum;
	long             **ppWaitClocks;
	uchar            **ppMaxRetrys;
	StepCallbk       StepDo;
	struct ListNode  ProcStepLN;
	const char       *pName;
};

struct FindStepWithFlagNumPa
{
	uchar             FlagNumToFind;
	struct Iterator   IFound;
};

struct FindStepByNamePa
{
	const char           *pNameToFind;
	struct ProcessStep   *pFound;
};

DECLARATION_STRUCT_CONSTRUCTOR( Process )
DECLARATION_STRUCT_CONSTRUCTOR( ProcessingList )

static __inline uchar
FlagNum(uchar pa_FlagNum)
{
	return pa_FlagNum;
}

uchar
FlagName(struct Process * ,const char *);
	
void 
ProcessAddStep( struct Process* , StepCallbk , long** , uchar** , const char* );

void 
ProcessConsAndSetSteps( struct Process* , struct Process* );

void 
ProcessStart( struct Process * , struct ProcessingList* );

void 
ProcessSafeStart( struct Process *, struct ProcessingList *, struct Iterator *, struct Iterator *);

void 
DoProcessing( struct ProcessingList* );

void 
ProcessFree( struct Process* );

void
ProcessTraceSteps( struct Process* );

BOOL
ProcessDisattach( struct Process* pa_pProc , struct ProcessingList *pa_pProcList );
//if you don't want to free the process
//after abortion, call this function to stop
//this process from executing. When disattached,
//the process will start from begining next time
//it is started by ProcessStart().

void
ProcessingListTrace(struct ProcessingList *);

extern struct WatiLevel g_WaitLevel[TKNET_WAIT_LEVELS];

void
ProcessSetCondition( uint );
