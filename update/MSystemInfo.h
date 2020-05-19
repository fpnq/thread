//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ�ϵͳ��Ϣ��Ԫ��Ԫ
//��Ԫ������
//�������ڣ�2007.4.23
//������Ա��¬��Զ
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
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
	static int	GetCpuNumber();						//cpu����	
	static char GetCurrentPidCpuPercent();
	static unsigned long GetCpuFrequency();			//cpuƽ��Ƶ�� Mhz
	static unsigned long GetPhyMemSize();			//�����ڴ��С
	static void GetMacAddress( void *, int );			
	static MString GetApplicationPath( void * = NULL);		//��ȡ���ڵ�Ŀ¼
	static MString GetApplicationName( void * = NULL );		//��ȡӦ�ó���ģ������
	static MString GetOsInformation();

	static char GetCpuPercent();						//cpuʹ����	
	static char GetMemPercent();						//�ڴ�ʹ����
	static int GetMemUnused();						//�ڴ�ʣ���� M
													//��ǰ���̸�����io��\д��(Kb)
	static int GetSysProcessInfo( int *, unsigned long * , unsigned long * );
													//��ǰtcp������,���緢�͡������ֽ�
	static int GetTcpLinkInfo( int *, unsigned long * , unsigned long * );
	//add by liuqy 20120911 for ȡ��ǰ���������ڴ��С, �����󷵻�0
	static unsigned long GetCurrPrcVirMemSize();
	//ȡ��ǰ���еĶ�д��������KB��
	static unsigned long  GetCurrPrcIORdWrtKB(unsigned long * out_pulRdKB, unsigned long * out_pulWrtKB);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
