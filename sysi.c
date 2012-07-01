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

#include <stdlib.h>
#include "tknet.h"

uint g_allocs = 0;

void*
_tkmalloc( uint pa_len )
{
	g_allocs ++;
	return malloc( pa_len );
}

void
_tkfree( void* pa_mem )
{
	g_allocs --;
	free( pa_mem );
}

#if defined( __GNUC__ ) && defined( __linux__ )
#include <sys/time.h>
#include <unistd.h>

void 
tkInitRandom()
{
	unsigned int ticks;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	ticks = tv.tv_sec + tv.tv_usec;
	
	srand(ticks);
}

#endif 

#ifdef _MSC_VER
#include <time.h>

void 
tkInitRandom()
{
	srand((unsigned int)time(NULL));
}

#endif

uint 
tkGetRandom()
{
	return rand();
}

#include <stdio.h>

static FILE *sta_pLogFile[ LOG_FILES ];
static BOOL cst_ifEnableLogFile[ LOG_FILES ];

#define LOG_FOPEN( _num , _name , _mode ) \
	cst_ifEnableLogFile[ _num ] = 1; \
	sta_pLogFile[ _num ] = fopen( _name , _mode ); \
	res = res && (int)sta_pLogFile[ _num ]

#include <string.h>

int
tkLogInit()
{
	int res = 1;
	LOG_FOPEN( 0 , "tknet.exp" , "w" );
	LOG_FOPEN( 1 , "tknet.log" , "a" );
	return res;
}

void
tkLog( uint pa_LogNum , const char* pa_log )
{
	if( pa_LogNum < LOG_FILES && cst_ifEnableLogFile[pa_LogNum] )
	{
		fwrite( pa_log , 1 , strlen( pa_log ) , sta_pLogFile[pa_LogNum] );
		fflush( sta_pLogFile[pa_LogNum] );
	}
}

void
tkLogClose()
{
	uint i;
	for( i = 0 ; i < LOG_FILES ; i++ )
	{
		if( cst_ifEnableLogFile[i] )
			fclose( sta_pLogFile[i] );
	}
}

#include <stdarg.h>
void
tkFormatStr(char* pa_pDest, const char* pa_pFormat , ... )
{
	va_list args;
	va_start(args , pa_pFormat);
	vsprintf(pa_pDest,pa_pFormat,args);
	va_end(args);
}

#if defined( __GNUC__ ) && defined( __linux__ )

#include <sys/time.h>

static struct timeval 
sta_tim;

long 
tkMilliseconds()
{
	gettimeofday(&sta_tim, NULL);

	return sta_tim.tv_sec * 1000 + sta_tim.tv_sec / 1000;
}

void 
tkMsSleep( unsigned long pa_ms )
{
	usleep( pa_ms * 1000);
}

#elif defined(_MSC_VER)

long 
tkMilliseconds()
{
	return clock();
}

void 
tkMsSleep( unsigned long pa_ms )
{
	Sleep( pa_ms );
}

#endif

#if defined( __GNUC__ ) && defined( __linux__ )

void 
tkBeginThread(void* (*pa_thread)(void*) ,void *pa_else )
{
	pthread_t id;
	int res;
	res = pthread_create( &id , NULL , pa_thread , pa_else );
}

#elif defined(_MSC_VER)

void 
tkBeginThread(void* (*pa_thread)(void*) ,void *pa_else )
{
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)pa_thread , pa_else ,0,0);
}

#endif

void 
StrTraceFormat( char *in_str )
{
	int i;
	char *buff = (char*)malloc( strlen(in_str) + 1 );
	strcpy( buff , in_str );

	for(i=0;buff[i]!='\0';i++)
	{
		if(buff[i] == '\r')
		{
			buff[i] = 'R';
		}
		else if(buff[i] == '\n')
		{
			buff[i] = 'N';
		}
		else if(buff[i] == ' ')
		{
			buff[i] = '_';
		}
	}

	PROMPT(Usual,"%s",buff);
	free(buff);
}

struct pipe *g_pImportantPrompt = NULL;
struct pipe *g_pUsualPrompt     = NULL;
struct pipe *g_pDebugPrompt     = NULL;
const char   g_ImportantPromptName[] = "ImportantPrompt";
const char   g_UsualPromptName[]     = "UsualPrompt";
const char   g_DebugPromptName[]     = "DebugPrompt";

void
Prompt(struct pipe *pa_pPipe,const char *pa_pPipeName,
		char* pa_pFormat, ...)
{
	static char buff[PROMT_BUFFERSIZE];
	va_list args;
	va_start(args, pa_pFormat);
	vsnprintf(buff,PROMT_BUFFERSIZE,pa_pFormat,args);
	va_end(args);
	
	if(pa_pPipe == NULL)
	{
		pa_pPipe = PipeMap((char*)pa_pPipeName);
	}

	PipeFlow(pa_pPipe,buff,strlen(buff)+1,NULL);
}
