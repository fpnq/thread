//------------------------------------------------------------------------------------------------------------------------------
//单元名称：共享内存单元
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
#ifndef __MEngine_MShareMemH__
#define __MEngine_MShareMemH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
#include "MMutex.h"
//------------------------------------------------------------------------------------------------------------------------------
class MShareMem
{
protected:
	#ifndef LINUXCODE
		HANDLE							m_hRecordData;
		void						*	m_lpData;
	#else
		int								m_hRecordData;
		void						*	m_lpData;
	#endif
protected:
	int									m_iSize;
	bool								m_bCreate;
	MMutex								m_mMutex;
protected:
	__inline int  inner_createkeyfromstring(const char * strIn);
public:
	MShareMem(void);
	virtual ~MShareMem();
public:
	//创建共享内存
	int  Create(MString strName,unsigned long lSize,bool bAnyAccess = false);
	//打开共享内存
	int  Open(MString strName);
	//关闭共享内存
	void Close(void);
public:
	//读取，写入数据到共享内存
	int  Read(unsigned long lOffset,char * lpOut,unsigned long lSize);
	int  Write(unsigned long lOffset,const char * lpIn,unsigned long lSize);
public:
	//获取共享内存长度
	int  GetSize(void);
};
//------------------------------------------------------------------------------------------------------------------------------
class MShareMemMt
{
protected:
	
#ifndef LINUXCODE
		HANDLE							m_hRecordData;
		void						*	m_lpData;
#else
		int								m_hRecordData;
		void						*	m_lpData;
#endif

protected:
	int									m_iSize;
	bool								m_bCreate;
	MMutex								m_mMutex;
protected:
	__inline int  inner_createkeyfromstring(const char * strIn);
public:
	MShareMemMt(void);
	virtual ~MShareMemMt();
public:
	//创建共享内存
	int  Create(MString strName,unsigned long lSize);
	//打开共享内存
	int  Open(MString strName);
	//关闭共享内存
	void Close(void);
public:
	//读取，写入数据到共享内存
	int  Read(unsigned long lOffset,char * lpOut,unsigned long lSize);
	int  Write(unsigned long lOffset,const char * lpIn,unsigned long lSize);
public:
	//获取共享内存长度
	int  GetSize(void);
};

#endif
//------------------------------------------------------------------------------------------------------------------------------
