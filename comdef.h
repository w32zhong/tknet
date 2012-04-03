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

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL int
#endif

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

#define DEF_STRUCT_CONSTRUCTOR( _type_name , _assignments ) \
        void \
        _type_name ## Cons( struct _type_name * out_cons) \
        { \
                _assignments \
        }

#define DECLARATION_STRUCT_CONSTRUCTOR( _type_name ) \
            void _type_name ## Cons( struct _type_name * );

#define DEF_AND_CAST( _to_var , _type_tag_casting_to , _from_var ) \
	_type_tag_casting_to * _to_var = ( _type_tag_casting_to *)( _from_var )

#define MEMBER_OFFSET( _type_tag , _member ) (int)(&( (( _type_tag *)0)-> _member ))

#define GET_STRUCT_ADDR( _member_addr , _type_tag , _member_name ) \
	( _type_tag *)((int)( _member_addr ) - MEMBER_OFFSET( _type_tag , _member_name ) )

#define TK_EXCEPTION( _describe ) \
	_descr_exception_( __FILE__ ": " #_describe " exception at line " \
	, __LINE__ )

static __inline void
_descr_exception_(const char* pa_presay ,uint pa_l)
{
	char LineBuff[16];
	tkLog(0,pa_presay);
	tkFormatStr(LineBuff,"%d",pa_l);
	tkLog(0,LineBuff);
	tkLog(0,".\n");
}

#define VCK( _cdt , _statments ) \
	if( _cdt ) \
	{ \
		TK_EXCEPTION( _cdt ); \
		_statments ; \
	}do{}while(0)


struct Bys
{
	const char* pBytes;
	uint  len;
};

#define BYS( _obj ) _Bys( (char*)& ( _obj ) , sizeof( _obj ) )

static __inline struct Bys
_Bys( char* pa_p , uint pa_size )
{
	struct Bys res;
	res.pBytes = pa_p;
	res.len = pa_size;

	return res;

}

static __inline struct Bys 
StrBys(const char* in_str)
{
	struct Bys res;
	res.pBytes = in_str;
	res.len = strlen(in_str);

	return res;
}
