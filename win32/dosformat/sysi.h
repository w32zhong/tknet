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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

extern unsigned int g_allocs;

void*
_tkmalloc( unsigned int );

void
_tkfree( void* );

#define tkmalloc( _type_tage ) \
    ( _type_tage *)_tkmalloc( sizeof ( _type_tage ) )

#define tkfree( _mem ) \
    _tkfree( _mem ); \
    _mem = NULL

void 
tkInitRandom();

unsigned int 
tkGetRandom();

#define LOG_FILES 2

int
tkLogInit();

void
tkLog( unsigned int , const char* );

void 
tkLogLenDat(unsigned int , const char* , unsigned int );
//this log function doesn't expect any '\0' in data to write.

void
tkLogClose();

void 
tkFormatStr(char* , const char* , ... );

long 
tkMilliseconds();

void 
tkMsSleep( unsigned long );
	
void 
tkBeginThread(void* (*)(void*) ,void * );

#define TK_THREAD( _thread_name ) void* _thread_name (void *pa_else) 

#if defined( __GNUC__ ) && defined( __linux__ )

#include <pthread.h>

#define tkMutex pthread_mutex_t
#define MutexInit( _mutex ) pthread_mutex_init( _mutex, NULL)
#define MutexLock( _mutex ) pthread_mutex_lock( _mutex)
#define MutexUnlock( _mutex ) pthread_mutex_unlock( _mutex)
#define MutexDelete( _mutex ) pthread_mutex_destroy( _mutex)

#elif defined(_MSC_VER)

#define tkMutex CRITICAL_SECTION
#define MutexInit( _mutex ) InitializeCriticalSection( _mutex)
#define MutexLock( _mutex ) EnterCriticalSection( _mutex)
#define MutexUnlock( _mutex ) LeaveCriticalSection( _mutex)
#define MutexDelete( _mutex ) DeleteCriticalSection( _mutex)

#endif

void 
StrTraceFormat( char* );

extern struct pipe *g_pImportantPrompt;
extern struct pipe *g_pUsualPrompt;
extern struct pipe *g_pDebugPrompt;
extern const char   g_ImportantPromptName[]; 
extern const char   g_UsualPromptName[];
extern const char   g_DebugPromptName[];

#define PROMPT( _pipe , ... ) \
	Prompt( g_p ## _pipe ## Prompt , g_ ## _pipe ## PromptName , __VA_ARGS__)

#define PROMT_BUFFERSIZE 1024

void
Prompt(struct pipe *,const char *,char* , ...);
