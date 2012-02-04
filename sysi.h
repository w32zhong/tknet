#include <string.h>
#include <stdio.h>

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
