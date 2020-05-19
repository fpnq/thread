//------------------------------------------------------------------------------------------------------------------------------
//单元名称：进程单元
//单元描述：主要处理进程创建、停止等操作
//创建日期：2007.3.15
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MProcessH__
#define __MEngine_MProcessH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
//------------------------------------------------------------------------------------------------------------------------------
class MProcess
{
protected:
	#ifndef LINUXCODE
		HANDLE						m_hRecordData;
		unsigned long				m_lRecordId;
	#else
		pid_t						m_hRecordData;
	#endif
public:
	MProcess(void);
	virtual ~MProcess();
public:
	//创建进程
	int  CreateProcess(MString strFileName,MString strRunParam, bool bKill = true, bool bShow = false, const char * pszDir = NULL );
	//杀死进程
	void KillProcess(void);

	#ifndef LINUXCODE
		unsigned long GetProcessId();
	#else
		pid_t GetProcessId();
	#endif

public:
	//进程是否存活
	bool IsActive(void);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
