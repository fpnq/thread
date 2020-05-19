//------------------------------------------------------------------------------------------------------------------------------
//单元名称：类型定义单元
//单元描述：主要为了兼容Windows和Linux操作系统、定义基本常量、定义公共的改造函数等
//创建日期：2007.3.14
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MTypeDefineH__
#define __MEngine_MTypeDefineH__
//------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <exception>
//------------------------------------------------------------------------------------------------------------------------------
#include "../typedef.h"
//------------------------------------------------------------------------------------------------------------------------------
#ifndef LINUXCODE
//..............................................................................................................................
//Windows操作系统
#define _WIN32_WINNT 0x0400
#include <windows.h>
//..............................................................................................................................
#else
//..............................................................................................................................
//Linux操作系统
//..............................................................................................................................
#endif
//------------------------------------------------------------------------------------------------------------------------------
//公共函数改造
//..............................................................................................................................
//拷贝字符串：为了修正标准strncpy函数在操作iCount时不能置0的问题
extern char * my_strncpy(char * strDest,const char * strSource,size_t iCount);
//..............................................................................................................................
//连接字符串：为了修正标准strncat函数在操作iCount时不能置0的问题
extern char * my_strncat(char * strDest,const char * strSource,size_t iCount);
//..............................................................................................................................
//打印字符串：为了修正标准snprintf函数在操作iCount时不能置0的问题
extern void my_snprintf(char * lpszBuffer,size_t lCount,const char * lpszFormat,...);

//..............................................................................................................................
// add by yuanjj for safe string formating 2014-03-08
/**
 *	string formating input
 */
extern df_s32_t my_snprintf_s( df_s8_t* buf, df_s32_t size, const df_s8_t* format, ... );
// add end 2014-03-08

//------------------------------------------------------------------------------------------------------------------------------
#define my_assert(str)		{ if ( !(str) ) { assert(0);	return(ERR_PUBLIC_SLOPOVER);	} }
//------------------------------------------------------------------------------------------------------------------------------
#ifndef LINUXCODE
//..............................................................................................................................
//Windows操作系统
#include <windows.h>
#include <io.h>
#include <process.h>

#define MSG_NOSIGNAL		0
//..............................................................................................................................
#else
//..............................................................................................................................
//Linux操作系统
#include "WinAPI.h"
#endif
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
