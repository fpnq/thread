//------------------------------------------------------------------------------------------------------------------------------
//单元名称：系统信息单元单元
//单元描述：
//创建日期：2007.4.23
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngineLib_MSystemInfoH__
#define __MEngineLib_MSystemInfoH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MString.h"
#include "MDll.h"

#ifndef LINUXCODE

#include <tlhelp32.h>
#include <Iprtrmib.h>
#include <nb30.h>

#else
//hey add 201706
#include "MDateTime.h"
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>   
#include <unistd.h>   
#include <sys/time.h>
#include <sys/param.h>
#include <ctype.h>
#endif

#define DEFAULTBUFSIZE	81920

//------------------------------------------------------------------------------------------------------------------------------
class MSystemInfo
{
protected:
	static char						static_strApplicationPath[256];
	static char						static_strApplicationName[256];
	static char						static_strOsInformation[256];
	static unsigned char			static_PhyMac[6][6];
	static int						static_nCpuCount;
	static unsigned long			static_dCpuFrequency;
	static unsigned long			static_PhyMemSize;	
#ifndef LINUXCODE
	static DWORD ( __stdcall * PGetIfTable )( PMIB_IFTABLE, DWORD *, BOOL );
	static DWORD ( __stdcall * PGetTcpStatistics )( PMIB_TCPSTATS );
#endif
	static MDll						static_iphpDll;
protected:
	//hey add 201706
	static	long	cpu_percentages(int cnt, int *out, long *new_valeu, long *old, long *diffs);
	//hey add 201706
	static char *skip_token(const char *p);
	//end
	//end
	static void GetBaseStaticInfo();
	__inline	static char *__BasePath(char *);
	__inline	static char *__BaseName(char *);
public:
	MSystemInfo(void);
	virtual ~MSystemInfo();
	static int Instance();
	static void Release();
public:
	static int	GetCpuNumber();						//cpu个数	
	static char GetCurrentPidCpuPercent();
	static unsigned long GetCpuFrequency();			//cpu平均频率 Mhz
	static unsigned long GetPhyMemSize();			//物理内存大小
	static void GetMacAddress( void *, int );			
	static MString GetApplicationPath( void * = NULL);		//获取所在的目录
	static MString GetApplicationName( void * = NULL );		//获取应用程序模块名称
	static MString GetOsInformation();

	static char GetCpuPercent();						//cpu使用率	
	static char GetMemPercent();						//内存使用率
	static int GetMemUnused();						//内存剩余量 M
													//当前进程个数，io读\写量(Kb)
	static int GetSysProcessInfo( int *, unsigned long * , unsigned long * );
													//当前tcp连接数,网络发送、接收字节
	static int GetTcpLinkInfo( int *, unsigned long * , unsigned long * );
	//add by liuqy 20120911 for 取当前进程虚拟内存大小, 出错误返回0
	static unsigned long GetCurrPrcVirMemSize();
	//取当前进行的读写数据量（KB）
	static unsigned long  GetCurrPrcIORdWrtKB(unsigned long * out_pulRdKB, unsigned long * out_pulWrtKB);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
