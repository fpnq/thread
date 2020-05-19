//------------------------------------------------------------------------------------------------------------------------------
#include "MSystemInfo.h"
#include "MFile.h"
#include "../BaseUnit/MThread.h"

#ifndef LINUXCODE
#pragma comment( lib, "netapi32" )
#pragma comment(lib, "Psapi")
#include "Psapi.h"
#else
#include "MDll.h"
#endif
//------------------------------------------------------------------------------------------------------------------------------
char						MSystemInfo::static_strOsInformation[256] = "no initialize";
unsigned char			    MSystemInfo::static_PhyMac[6][6] = {0};
int						    MSystemInfo::static_nCpuCount = 0;
unsigned long				MSystemInfo::static_dCpuFrequency = 0;
unsigned long				MSystemInfo::static_PhyMemSize = 0;

#ifndef LINUXCODE
typedef struct _IO_COUNTERS 
{ 
	ULONGLONG ReadOperationCount; 
	ULONGLONG WriteOperationCount; 
	ULONGLONG OtherOperationCount;
	ULONGLONG ReadTransferCount; 
	ULONGLONG WriteTransferCount; 
	ULONGLONG OtherTransferCount;
} IO_COUNTERS, *PIO_COUNTERS;

DWORD ( __stdcall * MSystemInfo::PGetIfTable )( PMIB_IFTABLE, DWORD *, BOOL ) = NULL;
DWORD ( __stdcall * MSystemInfo::PGetTcpStatistics )( PMIB_TCPSTATS ) = NULL;
#endif
MDll						MSystemInfo::static_iphpDll;

/**
 *	增加一个EngineLib的版本号
 */
	//大版本升级，因为引擎库增加了内存页管理、红黑树索引
	//modify by liuqy 20110704 for 修改正读DBF库中关闭时，没有去关闭打开的文件
	//modify by liuqy 20110706 for 修改了队列检查是否为NULL的错误，和MFile::Seek中WIN参数修改,让他向后seek支持大于4G文件 
unsigned long GetPublicLibraryVersion()
{
	//modify by liuqy 20110711 for 修改写DBF记录时，当记录多于原记录时，写DBF库会内存越界操作问题
	// modify by zhongjb 20111111 for 统计每个通道的数据包发送情况，包括发包失败、重试、丢失
	//modify by liuqy 20111121 for MCheckTimeOutObjectList安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	//add by liuqy 20120911 for 取当前进程虚拟内存大小,及通道情况统计
	//add by liuqy 20130105 for 套接口关闭修改，服务器使用快速关闭，客户端的关闭使用shutdown后再关闭
	//add by liuqy 20130710 for 增加XML功能，使用CXMLPool
	//add by liuqy 20140219 for 优化为解决死锁 
	//add by liuqy 20160401 for 优化关闭与发送频率时，程序会core问题
	return(((unsigned long)101 << 16) + 11);
}


/************************************************************************/
/* 增加一个计算FileTime差值的方法，供CPU使用率计算所用.
/* 20131101 chenxc.
/************************************************************************/
#ifndef LINUXCODE
static __int64 FileTimeCmp(const FILETIME fileTime1, const FILETIME fileTime2)
{
	__int64 * pTime1 = (__int64 *)(&fileTime1);
	__int64 * pTime2 = (__int64 *)(&fileTime2);
	
	return (*pTime1 - *pTime2);
}

#endif
// add by liuqy 20110124 for 取队列中数据的个数 B001

//..............................................................................................................................
MSystemInfo::MSystemInfo(void)
{

}
//..............................................................................................................................
MSystemInfo::~MSystemInfo()
{

}

int MSystemInfo::Instance()
{
	Release();
#ifndef LINUXCODE	
	if( static_iphpDll.LoadDll( "iphlpapi.dll" ) < 0 )
	{
		return -1;
	}

	PGetIfTable = (DWORD ( __stdcall * )( PMIB_IFTABLE, DWORD *, BOOL ) )static_iphpDll.GetDllFunction( "GetIfTable" );
	PGetTcpStatistics = (DWORD ( __stdcall * )( PMIB_TCPSTATS ))static_iphpDll.GetDllFunction( "GetTcpStatistics" );
	if( !PGetTcpStatistics || !PGetIfTable )
	{
		return -2;
	}
#endif
	return 1;
}

void MSystemInfo::Release()
{
	static_iphpDll.CloseDll();
#ifndef LINUXCODE
	PGetIfTable = NULL;
	PGetTcpStatistics = NULL;
#endif
}

char*	MSystemInfo::__BasePath(char *in)
{
	int	len, i;

	if(!in)
		return NULL;

	len = strlen(in);
	for(i = len-1; i >= 0; i--)
	{
		if(in[i] == '\\' || in[i] == '/')
		{
			in[i + 1] = 0;
			break;
		}
	}
	return in;
}

char*	MSystemInfo::__BaseName(char *in)
{
	int	len, i;

	if(!in)
		return NULL;

	len = strlen(in);
	for(i = len - 1; i >= 0; i--)
	{
		if(in[i] == '\\' || in[i] == '/')
		{
			in[i] = 0;
			break;
		}
	}
	return in + i + 1;
}

//..............................................................................................................................
MString MSystemInfo::GetApplicationPath(void * hModule)
{
	char					*lpszPath;
	char					szPath[MAX_PATH];
	MString					strPath;
	int						iRet;

	memset(szPath, 0, sizeof(szPath));
	#ifndef LINUXCODE

		iRet = ::GetModuleFileName((HMODULE)hModule, szPath, MAX_PATH);
		if(iRet <= 0)
		{
			strPath.Empty();
			return strPath;
		}
		else
		{
			lpszPath = __BasePath(szPath);
			strPath.Format("%s", lpszPath);
			return strPath;
		}

	#else

		if(!hModule)
		{
			iRet =  readlink("/proc/self/exe", szPath, MAX_PATH);
			if(iRet <= 0)
			{
				strPath.Empty();
				return strPath;
			}
			else
			{
				lpszPath = __BasePath(szPath);
				strPath.Format("%s", lpszPath);
				return strPath;
			}
		}
		else
		{
			//	LINUX版本,这里通过hModule来获取自身的名字或者路径的方法是
			//	参考MDll.cpp里面LoadDll会把this指针给MDll对象,到时候直接通过对象的来获取名字或者路径
			//	hModule == MDll(this)
			class MDll	*pModule;
			pModule = (class MDll *)hModule;
			strncpy(szPath, pModule->GetDllSelfPath(), sizeof(szPath));
			if(strlen(szPath) == 0)
			{
				strPath.Empty();
				return strPath;
			}
			else
			{
				lpszPath = __BasePath(szPath);
				strPath.Format("%s", lpszPath);
				return strPath;
			}
		}
			
	#endif
}

//..............................................................................................................................
MString MSystemInfo::GetApplicationName( void * hModule )
{
	char					*lpszName;
	char					szName[MAX_PATH];
	MString					strName;
	int						iRet;

	memset(szName, 0, sizeof(szName));
	#ifndef LINUXCODE

		iRet = ::GetModuleFileName((HMODULE)hModule, szName, MAX_PATH);
		if(iRet <= 0)
		{
			strName.Empty();
			return strName;
		}
		else
		{
			lpszName = __BaseName(szName);
			strName.Format("%s", lpszName);
			return strName;
		}


	#else

		if(!hModule)
		{
			iRet =  readlink("/proc/self/exe", szName, MAX_PATH);
			if(iRet <= 0)
			{
				strName.Empty();
				return strName;
			}
			else
			{
				lpszName = __BaseName(szName);
				strName.Format("%s", lpszName);
				return strName;
			}
		}
		else
		{
			//	LINUX版本,这里通过hModule来获取自身的名字或者路径的方法是
			//	参考MDll.cpp里面LoadDll会把this指针给MDll对象,到时候直接通过对象的来获取名字或者路径
			//	hModule == MDll(this)
			class MDll	*pModule;
			pModule = (class MDll *)hModule;
			strncpy(szName, pModule->GetDllSelfPath(), sizeof(szName));
			if(strlen(szName) == 0)
			{
				strName.Empty();
				return strName;
			}
			else
			{
				lpszName = __BaseName(szName);
				strName.Format("%s", lpszName);
				return strName;
			}
		}

	#endif
}

//..............................................................................................................................
void MSystemInfo::GetBaseStaticInfo()
{

#ifndef LINUXCODE
	SYSTEM_INFO			systeminfo;
	MEMORYSTATUS		memstatus;
	OSVERSIONINFO		osversion;
	unsigned long		ulPrequency[2];
	unsigned long		ulCurSave[2];
	
	//cpu数量
	GetSystemInfo( &systeminfo );
	static_nCpuCount = systeminfo.dwNumberOfProcessors;
	
	//cpu频率
	ulCurSave[0] = GetPriorityClass( GetCurrentProcess() );
	ulCurSave[1] = GetThreadPriority( GetCurrentThread() );

	SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	__asm
	{
		__emit 0x0f
		__emit 0x31
		mov [ulPrequency], eax
		mov [ulPrequency+4], edx
	}
	MThread::Sleep( 512 );
	__asm
	{
		__emit 0x0f
		__emit 0x31
		sub eax, [ulPrequency]
		sbb edx, [ulPrequency+4]
		mov [ulPrequency], eax
		mov [ulPrequency+4], edx
	}

	SetPriorityClass( GetCurrentProcess(), ulCurSave[0] );
	SetThreadPriority( GetCurrentThread(), ulCurSave[1] );

	static_dCpuFrequency = __int64((ulPrequency[0]+(((__int64)ulPrequency[1])<<32)))>>19;

	/**
	 *	ADD BY GUOGUO 20110127
	 *		运行的机器在2GB-4GB的物理内存,需要增加一个编译参数参数才能让GlobalMemoryStatus 正确的物理内存大小
	 *		但是始终,GlobalMemoryStatus最多只支持小于4GB的内存,
	 *		为了更好的支持超过4GB内存的信息提取.采取GlobalMemoryStatusEx 函数来达到目的.
	 *		VC6.0 需要安装比较新的SDK开发包,才有GlobalMemoryStatusEx库函数.
	 *		我自己相对比较懒,就难得去下载SDK开发包了.MSDN上面说这个函数包含在
	 *		kernel32.lib中,我猜想在kernel32.dll里面应该也有这个函数.
	 *		准备直接LoadDll kernel32.dll来调用这个函数,有一点需要特别注意
	 *		支持的操作系统是
	 *		Windows NT/2000: Requires Windows 2000 or later.
	 *		Windows 95/98: Unsupported.
	 *		Windows2000 Pro版本,我不是很确定支持它.不过没关系,如果取不到这个函数地址,就采取传统的GlobalMemoryStatus函数
	 *		这个值的错误,并不影响SM的运行.
	 *
	 *
	 */

	/**	下面的结构是从MSDN COPY过来的,没有安装SDK就没这个结构体,为了能使用.我就自己定义一个.
	 *	为了避免与有安装了SDK的机器上面结构体重复定义,我换个名字,成员这些都是一样的.
	 */
	typedef struct _GUOGUO {
	  DWORD dwLength; 
	  DWORD dwMemoryLoad; 
	  DWORDLONG ullTotalPhys; 
	  DWORDLONG ullAvailPhys; 
	  DWORDLONG ullTotalPageFile; 
	  DWORDLONG ullAvailPageFile; 
	  DWORDLONG ullTotalVirtual; 
	  DWORDLONG ullAvailVirtual; 
	  DWORDLONG ullAvailExtendedVirtual;
	} GUOGUO, *LPGUOGUO; 

	do {
		HINSTANCE	hInst;
		GUOGUO		mem;
		BOOL (CALLBACK *pFuncGlobalMemoryStatusEx)(LPGUOGUO);

		/**	其实可以考虑使用这个函数 GetModuleHandle,因为kernel32.dll是不可能被FreeLib的*/
		/**	所以使用GetModuleHandle,相对是比较安全的,这个DLL不存在被unmap情况*/
		/**	用LoadLib也不存在性能的问题,多次加载,对于LoadLib只是增加引用计数 MSDN说的*/
		hInst = LoadLibrary("kernel32.dll");
		if(hInst)
		{
			pFuncGlobalMemoryStatusEx = (BOOL (CALLBACK*)(LPGUOGUO))GetProcAddress(hInst, "GlobalMemoryStatusEx");
			if(pFuncGlobalMemoryStatusEx)
			{
				mem.dwLength = sizeof(mem);
				pFuncGlobalMemoryStatusEx(&mem);
				static_PhyMemSize = mem.ullTotalPhys >> 20;
			}
			else
			{
				memstatus.dwLength = sizeof( memstatus );
				GlobalMemoryStatus( &memstatus );
				static_PhyMemSize = memstatus.dwTotalPhys>>20;
			}
			FreeLibrary(hInst);
		}
		else
		{
			memstatus.dwLength = sizeof( memstatus );
			GlobalMemoryStatus( &memstatus );
			static_PhyMemSize = memstatus.dwTotalPhys>>20;
		}
	}while(0);

	//操作系统信息
	osversion.dwOSVersionInfoSize = sizeof( osversion );
	if( GetVersionEx( &osversion ) == 0 )
	{
		my_snprintf( static_strOsInformation, 256, "获取操作系统信息发生错误[ERR：%d]", GetLastError() );
	}
	else if( osversion.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		my_snprintf( static_strOsInformation, 256, "Windows 2000/NT %d.%d B%d %s", osversion.dwMajorVersion, osversion.dwMinorVersion, osversion.dwBuildNumber ,osversion.szCSDVersion );
	}
	else if (osversion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		my_snprintf( static_strOsInformation, 256,"Windows 95/98 %d.%d B%d %s", osversion.dwMajorVersion, osversion.dwMinorVersion, osversion.dwBuildNumber, osversion.szCSDVersion );
	}
	else
	{
		my_snprintf( static_strOsInformation, 256, "Win3.1 %d.%d B%d %s", osversion.dwMajorVersion, osversion.dwMinorVersion, osversion.dwBuildNumber, osversion.szCSDVersion );
	}		

#else
	char							temppath[256];
	MFile							fileptr;
	MString							tempstring;
	register int					i;
	char							tempbuf[8192];
	char						*	tempptr;
	char							changebuf[50];
	register int					k;
	register int					errorcode;
	MString							readstring;
	int								famliy = 0,mode = 0,steping = 0;

	int								sock;
	struct sockaddr					sa;
//	struct ifreq					ifr;

	//获取内存容量
	if ( fileptr.OpenRead("/proc/meminfo") < 0 )
	{
		static_PhyMemSize = 0;	
	}
	else
	{
		fileptr.Read(tempbuf,8192);
		if ( (tempptr = strstr(tempbuf,"Mem")) == NULL )
		{
			static_PhyMemSize = 0;	
		}
		else
		{
			while ( (* tempptr) != ':' && tempptr < (tempbuf + 8192) )
			{
				tempptr ++;	
			}
				
			tempptr ++;
			
			while ( (* tempptr) == ' ' && tempptr < (tempbuf + 8192) )
			{
				tempptr ++;	
			}
				
			k = 0;
			while ( (* tempptr) != ' ' && k < 49 )
			{
				changebuf[k] = * tempptr;
				tempptr ++;
				k ++;
			}
			k = 0;
				
			static_PhyMemSize = strtol(changebuf,NULL,10) / 1024;//	以MB为单位
		}
		fileptr.Close();	
	}

	//获取操作系统信息
	if ( fileptr.OpenRead("/proc/version") < 0 )
	{
		my_snprintf( static_strOsInformation,256,"获取操作系统信息发生错误[/proc/version不存在]");
	}
	else if ( (errorcode = fileptr.Read( static_strOsInformation,256)) < 0 )
	{
		fileptr.Close();
		my_snprintf( static_strOsInformation,256,"获取操作系统信息发生错误[读取数据错误]");
	}
	else
	{
		for( i=0;i<errorcode;i++ )
		{
			if ( static_strOsInformation[i] == '(' || static_strOsInformation[i] == ')' || static_strOsInformation[i] == '#' )
			{
				static_strOsInformation[i] = 0;
				break;
			}
		}
			
		fileptr.Close();
	}

		//获取CPU信息		
	errorcode = 0;		
	if ( fileptr.OpenRead("/proc/cpuinfo") < 0 )
	{
		//无法获取CPU信息
		static_nCpuCount = 0;
	}
	else
	{
		while ( fileptr.ReadString(&readstring) > 0 )
		{
			if ( readstring.StringPosition("processor") >= 0 )
			{
				errorcode ++;
				if ( errorcode >= 32 )
				{
					break;
				}
					
				famliy = 0;
				mode = 0;
				steping = 0;
			}
			else if ( readstring.StringPosition("cpu MHz") >= 0 )
			{
//				if ( errorcode >= 0 && errorcode < insize )
				if ( errorcode >= 0 && errorcode < 32)
				{
					tempstring = readstring.GetMatchBackString(":");
					tempstring.TrimLeft();
					tempstring.TrimRight();
					static_dCpuFrequency += tempstring.ToULong();
				}
			}
		}
		static_nCpuCount = errorcode;
		static_dCpuFrequency /= static_nCpuCount;
		fileptr.Close();
	}	

	//网卡地址
/*	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock >= 0)
	{
		strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
		ifr.ifr_name[IFNAMSIZ - 1] = 0;

		if (ioctl(sock, SIOCGIFHWADDR, &ifr) >=0 )
		{
			memcpy(&sa, &ifr.ifr_addr, sizeof(sa));
			memcpy(static_PhyMac[0], sa.sa_data, sizeof(mac));
		}
	}*/
#endif

}

//------------------------------------------------------------------------------------------------------------------------------
int	MSystemInfo::GetCpuNumber()
{
	if( static_nCpuCount == 0 )
	{
		GetBaseStaticInfo();
	}
	
	return static_nCpuCount;
}

//------------------------------------------------------------------------------------------------------------------------------
unsigned long MSystemInfo::GetCpuFrequency()
{
	if( static_dCpuFrequency == 0 )
	{
		GetBaseStaticInfo();
	}
	return static_dCpuFrequency;
}

//------------------------------------------------------------------------------------------------------------------------------
unsigned long MSystemInfo::GetPhyMemSize()
{
	if( static_PhyMemSize == 0 )
	{
		GetBaseStaticInfo();
	}
	return static_PhyMemSize;
}

//------------------------------------------------------------------------------------------------------------------------------
MString MSystemInfo::GetOsInformation()
{
	MString osinfo = static_strOsInformation;

	if( osinfo == "no initialize" )
	{
		GetBaseStaticInfo();
	}
	return static_strOsInformation;
}

//------------------------------------------------------------------------------------------------------------------------------
void MSystemInfo::GetMacAddress( void * pOut, int n )
{
#ifndef	LINUXCODE	//	GUOGUO 20090723
	int		nIndex = min( n, 6 );
	bool	bNeedGet = true;

	for( register int i = 0; i < 6; ++i )
	{
		if( static_PhyMac[0][i] != 0 )
		{
			bNeedGet = false;
			break;
		}
	}

	if( bNeedGet )
	{
		NCB					ncb; 
		UCHAR				uRetCode;
		LANA_ENUM			lana_enum;

#pragma pack( 1 )
		typedef struct _ASTAT_
		{
			ADAPTER_STATUS adapt;
    		NAME_BUFFER NameBuff [30];
		}ASTAT, * PASTAT;
#pragma pack()

		ASTAT				Adapter;
		char				tempmac[6] = {0};

		memset( &ncb, 0, sizeof(NCB) );
		ncb.ncb_command = NCBENUM;
		ncb.ncb_buffer = (unsigned char *) &lana_enum;
		ncb.ncb_length = sizeof(LANA_ENUM);
		uRetCode = Netbios( &ncb );
		if( uRetCode == NRC_GOODRET )
		{
			int				mCount = 0;
			for( int lana=0; lana<lana_enum.length; lana++ )
			{
				ncb.ncb_command = NCBRESET;
				ncb.ncb_lana_num = lana_enum.lana[lana];
				uRetCode = Netbios( &ncb );
				if( uRetCode != NRC_GOODRET )
					continue;

				memset( &ncb, 0, sizeof(ncb) );
				ncb.ncb_command = NCBASTAT;
				ncb.ncb_lana_num = lana_enum.lana[lana]; 
				strcpy( (char* )ncb.ncb_callname, "*" );
				ncb.ncb_buffer = (unsigned char *) &Adapter;
				ncb.ncb_length = sizeof(Adapter);
				uRetCode = Netbios( &ncb );
				if( uRetCode != NRC_GOODRET )
					continue;

				memset( tempmac, 0, 6 );
				if( memcmp( tempmac, Adapter.adapt.adapter_address, 6 ) == 0 )
				{
					continue;
				}

				memset( tempmac, 0xFF, 6 );
				if( memcmp( tempmac, Adapter.adapt.adapter_address, 6 ) == 0 )
				{
					continue;
				}
				if( mCount >= 6 )
				{
						break;
				}
				memcpy( static_PhyMac[mCount++], Adapter.adapt.adapter_address, 6 );
			}
		}
	}

	--nIndex;
	memcpy( pOut, static_PhyMac[nIndex], 6 );
#endif
}


//hey add 201706

char * MSystemInfo::skip_token(const char *p)
{
	while (isspace(*p)) p++;
	while (*p && !isspace(*p)) p++;
	return (char *)p;
}



long MSystemInfo::cpu_percentages(int cnt, int *out, long *new_valeu, long *old, long *diffs)
{
	register int i;
	register long change;
	register long total_change;
	register long *dp;
	long half_total;

	total_change = 0;
	dp = diffs;

	for (i = 0; i < cnt; i++)
	{
		if ((change = *new_valeu - *old) < 0)
		{
			change = (int)((unsigned long)*new_valeu-(unsigned long)*old);
		}
		total_change += (*dp++ = change);
		*old++ = *new_valeu++;
	}

	if (total_change == 0)
	{
		total_change = 1;
	}

	half_total = total_change / 2l;
	for (i = 0; i < cnt; i++)
	{
		*out++ = (int)((*diffs++ * 1000 + half_total) / total_change);
	}
	return(total_change);
}

char MSystemInfo::GetCurrentPidCpuPercent()
{
	char chcpu=0;

#ifndef LINUXCODE
#else

	static unsigned long current_time = 0;
	static unsigned long old_time = 0;
	static struct timeval lasttime = { 0, 0 };
	static struct timeval timediff = { 0, 0 };
	static long elapsed_msecs;
	MFile	fileptr;
	int state = 0;
	int fd = 0;
	int len = 0;
	double pcpu;
	char buffer[4096] = {0};
	char *p = NULL, *q = NULL;
	unsigned long elapsed = 0;
	struct timeval thistime = {0, 0};
	gettimeofday(&thistime, 0);
	timersub(&thistime, &lasttime, &timediff);
	elapsed_msecs = timediff.tv_sec * 1000 + timediff.tv_usec / 1000;
	lasttime = thistime;
	elapsed = timediff.tv_sec * HZ + (timediff.tv_usec * HZ) / 1000000;
	if(elapsed <= 0)
	{
		elapsed = 1;
	}

	sprintf(buffer, "/proc/%d/task/%d/stat", getpid(), getpid());


	if ( fileptr.OpenRead(buffer) < 0 )
	{
		return(0);	
	}
	else
	{
		len = fileptr.Read(buffer,4096);
		fileptr.Close();
	}

	if(len > 0)
		buffer[len] = '\0';
	else return 0;
	if ((p = strchr(buffer, '(')) == NULL)
	{
		return 0;
	}
	if ((q = strrchr(++p, ')')) == NULL)
	{
		return 0;
	}
	*q = '\0';
	p = q+1;
    while (isspace(*p)) p++;
	switch (*p++)			
	{
		case 'R': state = 1; break;
		case 'S': state = 2; break;
		case 'D': state = 3; break;
		case 'Z': state = 4; break;
		case 'T': state = 5; break;
		case 'W': state = 6; break;
		case '\0': return 0;
	}
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);		
	p = skip_token(p);
	
	current_time = strtoul(p, &p, 10);		
	current_time += strtoul(p, &p, 10); 	
	if ((pcpu = (current_time - old_time)/(double)elapsed) < 0.0)
	{
		pcpu = 0;
	}
	if(current_time != old_time)
		old_time = current_time;
	chcpu = pcpu * 100;
#endif
	return chcpu;

}


//end

//------------------------------------------------------------------------------------------------------------------------------
char MSystemInfo::GetCpuPercent()
{
#ifndef LINUXCODE
	
	// 静态变量，用以存储上次获取的数据.
	static FILETIME pre_userTime;
	static FILETIME pre_kernelTime;
	static FILETIME pre_idleTime;
	
	// 动态加载GetSysytemTimes方法.
	long ( __stdcall *GetSystemTimes )( LPFILETIME, LPFILETIME, LPFILETIME );
	GetSystemTimes = (long(__stdcall*)(LPFILETIME,LPFILETIME,LPFILETIME))::GetProcAddress( ::GetModuleHandle( "kernel32.dll" ),"GetSystemTimes" );
	if ( GetSystemTimes == NULL )
	{
		return(0);
	}
	
	// 获取当前数据.
	FILETIME cur_userTime;
	FILETIME cur_kernelTime;
	FILETIME cur_idleTime;
	GetSystemTimes(&cur_idleTime, &cur_kernelTime, &cur_userTime);
	
	// 计算差值.
	__int64 iIdleTime = FileTimeCmp(cur_idleTime, pre_idleTime);
	__int64 iKernelTime = FileTimeCmp(cur_kernelTime, pre_kernelTime);
	__int64 iUserTime = FileTimeCmp(cur_userTime, pre_userTime);
	
	// 将当前值记录到静态变量.
	pre_idleTime = cur_idleTime;
	pre_kernelTime = cur_kernelTime;
	pre_userTime = cur_userTime;
	
	// 计算最后结果.
	__int64 iSysTime = iKernelTime + iUserTime;
	if(0 == iSysTime)
	{
		return (0);
	}
	return (char)( (1 - (double)iIdleTime / iSysTime) * 100 + 0.5 );

#else
	MFile					fileptr;
	char					tempbuf[8192];
	char				*	tempptr;
	char				*	nextptr;
	static	int				total_old = 0;
	int						total_new = 0;
	static	int				idle_old = 0;
	int						idle_new = 0;
	int						total = 0, idle = 0;
	int			errorcode;
	int						k;
	static MCounter	m_cpu_count;
	static long cpu_time[5] = {0};
	static int  cpu_state[5] = {0};
	static long cpu_old[5] = {0};
	static long cpu_diff[5] = {0};
	static char	cpu_last_time = 0;
	if( m_cpu_count.GetDuration() > 1000 )
	{
		m_cpu_count.SetCurTickCount();
		if ( fileptr.OpenRead("/proc/stat") < 0 )
		{
			return(0);	
		}
		else
		{
			errorcode = fileptr.Read(tempbuf,8192);
			fileptr.Close();	


			//hey add 201706
			if(errorcode <= 0)
				return 0;
			tempbuf[errorcode] = '\0';
			tempptr = skip_token(tempbuf);
			/* "cpu" */
			cpu_time[0] = strtoul(tempptr, &tempptr, 0);
			cpu_time[1] = strtoul(tempptr, &tempptr, 0);
			cpu_time[2] = strtoul(tempptr, &tempptr, 0);
			cpu_time[3] = strtoul(tempptr, &tempptr, 0);
			cpu_time[4] = strtoul(tempptr, &tempptr, 0);
			cpu_percentages(5, cpu_state, cpu_time, cpu_old, cpu_diff);
			cpu_last_time = (char)((cpu_state[0]+5)/10);
			return cpu_last_time;
		}
	}
	else
		return cpu_last_time;
#endif
}

//------------------------------------------------------------------------------------------------------------------------------
char MSystemInfo::GetMemPercent()
{
	
#ifndef LINUXCODE
	int				nErrorCode = 0;
	MEMORYSTATUS	memstatus;
		
	memstatus.dwLength = sizeof( memstatus );

	GlobalMemoryStatus( &memstatus );
	
	nErrorCode = memstatus.dwMemoryLoad;

	return nErrorCode;
#else
	MFile				fileptr;
	char				tempbuf[8192];
	char			*	tempptr;
	//char			*	nextptr;
	double				user,total;
		
	if ( fileptr.OpenRead("/proc/meminfo") < 0 )
	{
		return(0);	
	}
	else
	{
		fileptr.Read(tempbuf,8192);
		fileptr.Close();
			
		if ( (tempptr = strstr(tempbuf,"MemTotal:")) == NULL )
		{
			return(0);
		}
		else
		{
			tempptr += 9;
			total = strtod(tempptr, NULL);
		}
		if ( (tempptr = strstr(tempbuf,"MemFree:")) == NULL )
		{
			return(0);
		}
		else
		{
			tempptr += 8;
			user = strtod(tempptr, NULL);
		}
		return((unsigned char)((total - user) * 100 / total));
	}
#endif
}

//------------------------------------------------------------------------------------------------------------------------------
int MSystemInfo::GetMemUnused()
{
#ifndef LINUXCODE
	int dwRet;
	/**
	 *	这个地方,对于大于2GB的内存也存在问题.但是似乎好象没有CODE调用它.但是代码还是准备好了处理大于2GB的问题
	 */
	MEMORYSTATUS	memstatus;
	/**
	 *	ADD BY GUOGUO 20110127
	 *		运行的机器在2GB-4GB的物理内存,需要增加一个编译参数参数才能让GlobalMemoryStatus 正确的物理内存大小
	 *		但是始终,GlobalMemoryStatus最多只支持小于4GB的内存,
	 *		为了更好的支持超过4GB内存的信息提取.采取GlobalMemoryStatusEx 函数来达到目的.
	 *		VC6.0 需要安装比较新的SDK开发包,才有GlobalMemoryStatusEx库函数.
	 *		我自己相对比较懒,就难得去下载SDK开发包了.MSDN上面说这个函数包含在
	 *		kernel32.lib中,我猜想在kernel32.dll里面应该也有这个函数.
	 *		准备直接LoadDll kernel32.dll来调用这个函数,有一点需要特别注意
	 *		支持的操作系统是
	 *		Windows NT/2000: Requires Windows 2000 or later.
	 *		Windows 95/98: Unsupported.
	 *		Windows2000 Pro版本,我不是很确定支持它.不过没关系,如果取不到这个函数地址,就采取传统的GlobalMemoryStatus函数
	 *		这个值的错误,并不影响SM的运行.
	 *
	 *
	 */

	/**	下面的结构是从MSDN COPY过来的,没有安装SDK就没这个结构体,为了能使用.我就自己定义一个.
	 *	为了避免与有安装了SDK的机器上面结构体重复定义,我换个名字,成员这些都是一样的.
	 */
	typedef struct _GUOGUO {
	  DWORD dwLength; 
	  DWORD dwMemoryLoad; 
	  DWORDLONG ullTotalPhys; 
	  DWORDLONG ullAvailPhys; 
	  DWORDLONG ullTotalPageFile; 
	  DWORDLONG ullAvailPageFile; 
	  DWORDLONG ullTotalVirtual; 
	  DWORDLONG ullAvailVirtual; 
	  DWORDLONG ullAvailExtendedVirtual;
	} GUOGUO, *LPGUOGUO; 

	do {
		HINSTANCE	hInst;
		GUOGUO		mem;
		BOOL (CALLBACK *pFuncGlobalMemoryStatusEx)(LPGUOGUO);

		/**	其实可以考虑使用这个函数 GetModuleHandle,因为kernel32.dll是不可能被FreeLib的*/
		/**	所以使用GetModuleHandle,相对是比较安全的,这个DLL不存在被unmap情况*/
		/**	用LoadLib也不存在性能的问题,多次加载,对于LoadLib只是增加引用计数 MSDN说的*/
		hInst = LoadLibrary("kernel32.dll");
		if(hInst)
		{
			pFuncGlobalMemoryStatusEx = (BOOL (CALLBACK*)(LPGUOGUO))GetProcAddress(hInst, "GlobalMemoryStatusEx");
			if(pFuncGlobalMemoryStatusEx)
			{
				mem.dwLength = sizeof(mem);
				pFuncGlobalMemoryStatusEx(&mem);
				dwRet = mem.ullAvailPhys >> 20;
			}
			else
			{		
				memstatus.dwLength = sizeof( memstatus );
				GlobalMemoryStatus( &memstatus );
				dwRet = memstatus.dwAvailPhys>>20;
			}
			FreeLibrary(hInst);
		}
		else
		{
			memstatus.dwLength = sizeof( memstatus );
			GlobalMemoryStatus( &memstatus );
			dwRet = memstatus.dwAvailPhys>>20;
		}
	}while(0);
	
	return dwRet;
#else
	MFile				fileptr;
	char				tempbuf[8192];
	char			*	tempptr;
	char			*	nextptr;
	double				user,total;
		
	if ( fileptr.OpenRead("/proc/meminfo") < 0 )
	{
		return(0);	
	}
	else
	{
		fileptr.Read(tempbuf,8192);
		fileptr.Close();
			
		if ( (tempptr = strstr(tempbuf,"Mem:")) == NULL )
		{
			return(0);
		}
		else
		{
			tempptr += 4;
			total = strtod(tempptr,&nextptr) + 1;
			tempptr = nextptr;
			user = strtod(tempptr,&nextptr);		
			return (long)(total - user)>>20;
		}
	}

#endif
}

//------------------------------------------------------------------------------------------------------------------------------
int MSystemInfo::GetSysProcessInfo( int * pProCount, unsigned long * PIoRead, unsigned long * pIoWrite )
{
#ifndef LINUXCODE
	HANDLE			hProcess;
	HANDLE			hToken;
	HANDLE			procSnap;
	bool			bEnable = true;
	BOOL			bRet;
	MDll			dll;
	PROCESSENTRY32	procEntry;


	IO_COUNTERS		IoCounters;
	
	BOOL ( __stdcall * PGetProcessIoCounters )( HANDLE, PIO_COUNTERS ) = ( BOOL ( __stdcall * )( HANDLE, PIO_COUNTERS ))GetProcAddress( GetModuleHandle( "kernel32" ), "GetProcessIoCounters" );

	if( pProCount )
	{
		*pProCount = 0;
	}
	if( PIoRead )
	{
		* PIoRead = 0;
	}
	if( pIoWrite )
	{
		* pIoWrite = 0;
	}
	if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
	    if( !LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid ) )
		{
			bEnable = false;
		}
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		if( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
		{
			bEnable = false;
		}
		CloseHandle( hToken );
	}
	if( !bEnable )
	{
		return -1;
	}

	procSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( procSnap == INVALID_HANDLE_VALUE )
    {
        return -2;
    }
    //

	if( pProCount )
	{
		memset( &procEntry, 0, sizeof( procEntry ) );

		procEntry.dwSize = sizeof( PROCESSENTRY32 );
		bRet = Process32First( procSnap, &procEntry );
		while( bRet )
		{
			++(*pProCount);
			//
			hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false, procEntry.th32ProcessID );
			if ( hProcess )
			{
				memset( &IoCounters, 0, sizeof( IoCounters ) );
				if( PGetProcessIoCounters )
				{
					PGetProcessIoCounters( hProcess, &IoCounters );
				}
				if( PIoRead )
				{
					//*PIoRead += IoCounters.ReadOperationCount;
					*PIoRead += (IoCounters.ReadTransferCount / 1024);
				}
				if( pIoWrite )
				{
					//*pIoWrite += IoCounters.WriteOperationCount;
					*pIoWrite += (IoCounters.WriteTransferCount / 1024);
				}
				CloseHandle( hProcess );
			}
			bRet = Process32Next( procSnap, &procEntry );
		}
		CloseHandle( procSnap );
	}
#else
	FILE *			fpipe = NULL;
	int				nProCount = 0;
	char			tempbuf[8192];
	unsigned long   ulTotalRead = 0;
	unsigned long   ulTotalWrite = 0;
	char			*saveptr = NULL;

	if( pProCount )
	{
		*pProCount = 0;
	}
	if( PIoRead )
	{
		* PIoRead = 0;
	}
	if( pIoWrite )
	{
		* pIoWrite = 0;
	}

	//获得进程总数
	nProCount = 0;
	fpipe = popen( "ps -e", "r" );
	if( fpipe != NULL )
	{	
		while( fgets( tempbuf, 256, fpipe ) )
		{	
			++nProCount;
		}
		if( nProCount )
		{
			--nProCount;
		}
		pclose( fpipe );
	}
	if( pProCount )
	{
		*pProCount = nProCount;
	}

	/*
	 *	首先通过stat函数,来获取自己所在的的设备的st_dev号
	 *	为了避免多此调用,我们只是第一次获取一下就OK了
	 *	我们也默许了,是统计应用程序所在的分区磁盘的IO情况,
	 *	如果要进一步说,他都还不是真的IO,仅仅是内核的文件系统缓冲区的读写情况
	 */
	static int IsFirst = true;
	struct stat buf;
	static int	ma = 0;
	static int	mi = 0;
	unsigned long nr = 0;
	unsigned long nw = 0;
	if(IsFirst)
	{
		if(stat(GetApplicationPath().c_str(), &buf) != 0)
		{
			return 1;
		}
		else
		{
			/*
			 *	计算主设备号和子设备号
			 */
			ma = major(buf.st_dev);
			mi = minor(buf.st_dev);
			IsFirst = false;
		}
	}

	/*
	 *	 下面根据主设备号和子设备号在/proc/diskstatus文件中搜索
	 *		LINUX的/proc文件系统格式变化很快.这一点要注意
	 *	第1列 : 磁盘主设备号(major)
		第2列 : 磁盘次设备号(minor)
		第3列 : 磁盘的设备名(name)
		第4列 : 读请求总数(rio)
		第5列 : 合并的读请求总数(rmerge)
		第6列 : 读扇区总数(rsect)
		第7列 :   读数据花费的时间，单位是ms.(从__make_request到 end_that_request_last)(ruse)
		第8列 :   写请求总数(wio)
		第9列 :   合并的写请求总数(wmerge)
		第10列 : 写扇区总数(wsect)
		第11列 : 写数据花费的时间，单位是ms. (从__make_request到 end_that_request_last)(wuse)
		第12列 : 现在正在进行的I/O数(running),等于I/O队列中请求数
		第13列 : 系统真正花费在I/O上的时间，除去重复等待时间(aveq)
		第14列 : 系统在I/O上花费的时间(use)。
	 */
	/**
	 *	strtok 不是线程安全,改用POSIX 建议的strtok_r
	 */
	FILE	*fp;
	char	*p;
	fp = fopen("/proc/diskstats", "r");
	if(fp)
	{
		memset(tempbuf, 0, sizeof(tempbuf));
		while(fgets(tempbuf, 1024, fp))
		{
			p = strtok_r(tempbuf, " \t\r\n", &saveptr);			//	1
			if(p)
			{
				//	主设备号
				if(ma != atol(p))
				{
					memset(tempbuf, 0, sizeof(tempbuf));
					continue;
				}
				else
					p = strtok_r(NULL, " \t\r\n", &saveptr);	//	2
			}
			if(p)
			{
				//	子设备号
				if(mi != atol(p))
				{
					memset(tempbuf, 0, sizeof(tempbuf));
					continue;
				}
				else
					p = strtok_r(NULL, " \t\r\n", &saveptr);	//	3
			}
			p = strtok_r(NULL, " \t\r\n", &saveptr);	//	4
			p = strtok_r(NULL, " \t\r\n", &saveptr);	//	5
			p = strtok_r(NULL, " \t\r\n", &saveptr);	//	6
			if(p)
			{
				*PIoRead = (strtoul(p, NULL, 10)) / 2;	//	默认一个扇区是512字节
			}
			p = strtok_r(NULL, " \t\r\n", &saveptr);
			p = strtok_r(NULL, " \t\r\n", &saveptr);
			p = strtok_r(NULL, " \t\r\n", &saveptr);
			p = strtok_r(NULL, " \t\r\n", &saveptr);
			if(p)
			{
				*pIoWrite = strtoul(p, NULL, 10) / 2;	//	默认一个扇区是512字节
			}
			break;
		}
		fclose(fp);
	}

#endif
	return 1;
}

//------------------------------------------------------------------------------------------------------------------------------
int MSystemInfo::GetTcpLinkInfo( int * pTcpCout, unsigned long * pSendNum, unsigned long * pRecvNum )
{
#ifndef LINUXCODE
	DWORD			dwSize = 0;
	char *			pBuf = NULL;
	MIB_TCPSTATS	tcpstats;
	int				nMac = 0;
	PMIB_IFTABLE	pTable = NULL;
	unsigned long   ulInBytes = 0;
	unsigned long   ulOutBytes = 0;
	char			defaultbuf[DEFAULTBUFSIZE];


//	DWORD ( __stdcall * PGetIfTable )( PMIB_IFTABLE, DWORD *, BOOL ) = NULL;
//	DWORD ( __stdcall * PGetTcpStatistics )( PMIB_TCPSTATS ) = NULL;
	
	if( pTcpCout )
	{
		*pTcpCout = 0;
	}
	if( pSendNum )
	{
		*pSendNum = 0;
	}
	if( pRecvNum )
	{
		*pRecvNum = 0;
	}

#ifndef	_LINUXTRYOFF
	try
	{
#endif
/*		if( dll.LoadDll( "iphlpapi.dll" ) < 0 )
		{
			return -1;
		}

		PGetIfTable = (DWORD ( __stdcall * )( PMIB_IFTABLE, DWORD *, BOOL ) )dll.GetDllFunction( "GetIfTable" );
		PGetTcpStatistics = (DWORD ( __stdcall * )( PMIB_TCPSTATS ))dll.GetDllFunction( "GetTcpStatistics" );
		if( !PGetTcpStatistics )
		{
			return -2;
		}
*/
		if( PGetTcpStatistics == NULL || PGetIfTable == NULL )
		{
			Instance();
		}
		
		if( PGetTcpStatistics == NULL || PGetIfTable == NULL )
		{
			return -2;
		}

		PGetIfTable( NULL, &dwSize, false );
		if( dwSize > DEFAULTBUFSIZE )
		{
			//modify by liuqy 20150630 需要将new 更换为malloc 
			//pBuf = new char[dwSize];
			pBuf = (char*)malloc(dwSize);
			if( pBuf == NULL )
			{
				return -3;
			}
		}
		else
		{
			pBuf = defaultbuf;
		}
		pTable = reinterpret_cast< PMIB_IFTABLE >(pBuf);
		PGetIfTable( pTable, &dwSize, false );
		if( pTcpCout )
		{
			*pTcpCout = pTable->dwNumEntries;
		}

		for( int i = 0; i < pTable->dwNumEntries; ++i )
		{
			if ( pTable->table[i].dwType != MIB_IF_TYPE_LOOPBACK )
			{
				ulInBytes += pTable->table[i].dwInOctets;
				ulOutBytes += pTable->table[i].dwOutOctets;
//				if( nMac < 6 )
//				{
//					memcpy( static_PhyMac[nMac], pTable->table[i].bPhysAddr, 6 );
//				}
			}
		}
		if( pBuf != defaultbuf )
		{
			//modify by liuqy 20150630 需要将new 更换为malloc 
			//delete []pBuf;
			free(pBuf);
			pBuf = NULL;
		}
		
		memset( &tcpstats, 0, sizeof( tcpstats ) );
		PGetTcpStatistics( &tcpstats );

		if( pTcpCout )
		{
			*pTcpCout = tcpstats.dwNumConns;
		}
		if( pSendNum )
		{
			*pSendNum = ulOutBytes;
		}

		if( pRecvNum )
		{
			*pRecvNum = ulInBytes;
		}
#ifndef	_LINUXTRYOFF
	}
	catch( ... )
	{
		Release();
		return -1;
	}
#endif
#else
	MFile					fileptr;
	MString					tempstring;
	int						i, j, k;
	unsigned long			recvamount = 0;
	unsigned long			sendamount = 0;
	int						tcpcount = 0;
	char					tempbuf[4096];
	FILE *					fpipe = NULL;
	char					*tempptr, *nextptr;

	if( pTcpCout )
	{
		*pTcpCout = 0;
	}
	if( pSendNum )
	{
		*pSendNum = 0;
	}
	if( pRecvNum )
	{
		*pRecvNum = 0;
	}

	if ( fileptr.OpenRead("/proc/net/dev") < 0 )
	{
		return(0);	
	}
	else
	{
		fileptr.Read(tempbuf,8192);
		fileptr.Close();
			
		if ( (tempptr = strstr(tempbuf,"eth0:")) == NULL )
		{
			return(0);
		}
		else
		{
			tempptr += 5;

			/*
			 *	64位的LINUX操作系统,/proc/net/dev  文件系统
			 *	上面显示的字节,不会是0-0xFFFFFFFF 进行循环使用了
			 *	可以显示超过4GB的字节,为了兼容32位我们让它还是按照32位的方式回滚方式处理
			 *				--GUOGUO 2010-11-19
			 */
			// hey change 20170816 
//#ifndef LINUXCODE64
//			recvamount = strtoul(tempptr,&nextptr,10);
//#else
			unsigned long long	xr = 0;
			xr = strtoull(tempptr,&nextptr,10);
			recvamount = (unsigned long)(xr % 0xFFFFFFFF);
//#endif
			tempptr = nextptr;

			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			strtoul(tempptr,&nextptr,10); tempptr = nextptr;
			//hey chuange 20170816
//#ifndef LINUXCODE64
//			sendamount = strtoul(tempptr,&nextptr,10);
//#else
			unsigned long long	xs = 0;
			xs = strtoull(tempptr,&nextptr,10);
			sendamount = (unsigned long)(xs % 0xFFFFFFFF);
//#endif
			tempptr = nextptr;
		}
	}


	//获得tcp连接数量
#if 0
	tcpcount = 0;
	fpipe = popen( "netstat -n", "r" );
	if( fpipe != NULL )
	{	
		while( fgets( tempbuf, 256, fpipe ) )
		{	
			if( !bcmp( tempbuf, "tcp", 3) )
			{
				++tcpcount;
			}
		}
		pclose( fpipe );
	}
#endif
	tcpcount = 0;
	//	获取文件大小
	{
		FILE	*fp;
		char	szBuf[1024 + 1];
		fp = fopen("/proc/net/tcp", "rb");
		if(fp != NULL)
		{
			while(fgets(szBuf, 1024, fp))
			{
				tcpcount++;
			}
			fclose(fp);
		}
	}
	
	if( pTcpCout )
	{
		*pTcpCout = tcpcount;
	}
	if( pSendNum )
	{
		*pSendNum = sendamount;
	}

	if( pRecvNum )
	{
		*pRecvNum = recvamount;
	}

#endif

	return 1;
}
//add by liuqy 20120911 for 取当前进程虚拟内存大小,出错误返回0
unsigned long MSystemInfo::GetCurrPrcVirMemSize( )
{
	unsigned long ulRet = 0;
#ifndef LINUXCODE
	//windows 
	PROCESS_MEMORY_COUNTERS pmc;
	if(GetProcessMemoryInfo(   GetCurrentProcess(),   &pmc,   sizeof(pmc))   )
	{
		ulRet = pmc.PagefileUsage;
	}
#else
	//linux
#endif
	return ulRet;
}
//取当前进行的读写数据量（KB）
unsigned long  MSystemInfo::GetCurrPrcIORdWrtKB(unsigned long * out_pulRdKB, unsigned long * out_pulWrtKB)
{
	unsigned long ulRet = 0;
	if(out_pulRdKB)
		*out_pulRdKB = 0;
	if(out_pulWrtKB)
		*out_pulWrtKB = 0;

#ifndef LINUXCODE
	IO_COUNTERS IoCounters;
	BOOL ( __stdcall * PGetProcessIoCounters )( HANDLE, PIO_COUNTERS ) = ( BOOL ( __stdcall * )( HANDLE, PIO_COUNTERS ))GetProcAddress( GetModuleHandle( "kernel32" ), "GetProcessIoCounters" );

	memset( &IoCounters, 0, sizeof( IoCounters ) );
	if(PGetProcessIoCounters)
		PGetProcessIoCounters(GetCurrentProcess(), &IoCounters);
	if(out_pulRdKB)
		*out_pulRdKB += (IoCounters.ReadTransferCount / 1024);
	if( out_pulWrtKB )
	{
		*out_pulWrtKB += (IoCounters.WriteTransferCount / 1024);
	}
	ulRet = (IoCounters.ReadTransferCount / 1024) + (IoCounters.WriteTransferCount / 1024);
#else
#endif
	return ulRet;
}
