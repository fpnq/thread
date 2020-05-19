//------------------------------------------------------------------------------------------------------------------------------
//单元名称：线程单元
//单元描述：主要处理线操作类
//创建日期：20111213
//创建人员：liuqy
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __CTHREAD_BASE___H__
#define __CTHREAD_BASE___H__
//------------------------------------------------------------------------------------------------------------------------------
//#include "./my_listnode.h"
#include "./MMutex.h"
#include "./MDateTime.h"
#include <time.h>
#include "./CRdWrtLock.h"

//------------------------------------------------------------------------------------------------------------------------------

#define  EVERY_LINK_NODE_SIZE	0xff


/************************************************************************
 *	功能：定义打印输出报告回调函数
 *	参数：in_oCustomSelfPRT	输入-用户打印时传入的指针，一般调用类打印功能时传入的this，这是只是原封返回
 *		in_pcszMsg		输入-打印的报告信息
 *	返回：无
 ************************************************************************/
#ifndef DF_rv_PrintReportCallBack
typedef void DF_rv_PrintReportCallBack(void * in_oCustomSelfPRT, const char * in_pcszMsg);
#endif

class CManageThreadBase ;

class CThreadBase 
{
	friend class CManageThreadBase;

public:
	//当前线程使用方案一运行，还是方案二运行
	CThreadBase(bool in_blExecRun0Flag, const char * strObjectName);
	//线程使用方案一运行
	CThreadBase(const char * strObjectName);

	virtual ~CThreadBase();
public:
	//以下为用户线程需要实现的函数, 当线程为方案一时，方案二的函数为实现体为空即可，当使用方案二时，方案一的函数体为空即可。
	
	//以下线程方案一，即工作很简单，由系统去完成等待事件处理，即系统会循环调用NeedRun0Again,如果返回为true则会调用Run0

	//需要运行吗，只有返回true，才会去调用Run函数，返回为false，线程会睡眠200毫秒，当需要立即运行时，请调用SetRunEvent
	virtual	bool	NeedRun0Again() = 0;
	//线程真正处理运行，注意该函数内不能有循环或等待事件，只做一次，就返回，系统会通过再次调用进入工作
	virtual void	Run0() = 0;
	
	//以下为线程方案二，即事件与等待时间由用户管理,系统不等待会循环调用Run1，所以用户实现的Run1最好在200毫秒内返回，否则系统会报该线程超时
	//线程真正处理运行，该函数内可以有循环（非死循环）或等待事件，但不要超过200毫秒，注意除开等待用户在真正做事之前(即等待到需要做事事件)，请调用EntryWorking
	virtual void	Run1() = 0;
public:
	//以下为线程运行需要的函数

	//运行方案一中：有立即运行要求
	virtual	void	SetRunEvent();

	//运行方案二，需要去做事了
	//进入工作中...，主要用于统计线程利用率时使用。工作完成时，需要调用FinishWork
	void			EntryWorking();

	void			SetSrcFileLineName(char * in_pszSrcFileName,	unsigned short	in_ushSrcFileLine );	//源文件的行号


public:
	//线程运行状态
	typedef enum
	{
		RS_None	= 0,	//未启动状态
		RS_Stop,		//线程已经停止了
		RS_Run ,		//线程运行状态
		RS_Suspend,		//线程已经挂起了
		RS_Starting,	//线程正在启动中
	}eRunState;

	//如下是对操作的控制操作
	//启动一个线程
	int  StartThread(const char * in_pszName = NULL);

	// 停止当前线程,让线程自动停止
	void StopThread(void);
	//杀死当前线程,线程可能已经死亡了
	void Kill(unsigned long lWaitTime);
	//线程是否正在行情中
	bool IsRunning();

	//取当前线程是否进入挂起状态
	bool			IsEntrySuspend();
	//设置线程挂起状态
	void			SetSuspend();
	//激活挂起的线程
	void			ActivateFromSuspend();

	//取当前线程是否在运行状态（线程挂起也是运行状态）
	virtual CThreadBase::eRunState	GetRunState();		


	//返回当前线程是否已经停止了
	bool GetThreadStopFlag(void);

	//注销注册的资源
	virtual void Unregister();


	// 休眠函数
	static void Sleep(unsigned long lMSec);

	//让出CPU控制权一下
	static void CpuYield();

	//设置对象名称，以便于提示查找
	void SetSpObejctName(const char * strObjectName);

	//设置对象附加数据，主要便于调试
	void SetObjectAddtionData(unsigned long lIn);

	//设置对象附加说明，主要便于调试,数据长度为16字节
	void SetObjectAddtionData(const char * in_pszNote);
	//	获取附加数据
	unsigned long GetObjectAddtionData();
	//	获取附加数据,只用于获取存入字符串时使用
	const char * GetObjectAddtionDataStr();

	void Release();
	//取当前线程ID
	unsigned	long	GetThreadID();

protected:
	//停止线程利用率统计
	void			StopStatisticsRate();
	//开始线程利用率统计
	void			StartStatisticsRate();
	//取线程利用率统计情况
	bool			GetStatisticsRate();

	//工作结束，与进入工作中(EntryWorking)配对使用
	void			FinishWork();

	//让线程挂起时，停止运转，让线程去等待一个事件
	MWaitEvent					m_oWaitEvent;

	bool						m_blStatisticsRate;	//线程利用率统计标志

	bool						m_blSuspend;	//线程挂起
	bool						m_blEntrySuspend;	//线程进入挂起状态

	bool						m_blEntryWorkStatis;	//线程一个工作统计状态开始，需要调用停止状态，让其进行停止状态
	MExactitudeCount			m_oWorkStatisUsecCounter;	//线程工作统计，统计结束时，需要将取得的值增加到实际工作时间中。
	MExactitudeCount			m_oBeginStatisCounter;	//线程开始工作统计时间
	time_t						m_stBeginWorkTime;	//线程开始工作时间，自1970年以来的秒

	unsigned	long			m_ulThreadID;	//线程ID
	unsigned __int64			m_uiLoopCount;	//线程循环次数
	unsigned __int64			m_uiWorkCount;	//工作次数
	unsigned __int64			m_uiRealWorkUSECCount;	//线程实际工作时间统计(微秒)

	char						m_szObjectName[32];
	char						m_szAddtionData[16];
	MCounter					m_mRefreshTime;
	unsigned long				m_lAddtionData;

	unsigned short				m_ushSrcFileLine;	//源文件的行号
	char						m_szSrcFileName[32];	//源文件名


	bool						m_blProcessingFlag;	//正在处理标志 add by liuqy 20101020 
	bool						m_blExecRun0Flag;	//运行标志，即使用方案1，false为方案2
	bool						m_blRunFlag;
	bool						m_blRunningFlag;	//正在运行中

	CManageThreadBase	*		m_poManageThread;	//线程管理
#ifndef LINUXCODE
	HANDLE							m_hRecordData;
#else
	pthread_t						m_hRecordData;
#endif

protected:

	//刷新检查，主要用于线程进入挂起状态
	void					Refresh(void);
	bool								m_bStopCurThread;
	//复位设置
	void		Reset();
	static unsigned int __stdcall MyThreadRun(void * lpIn);


};

class CManageThreadBase 
{
public:
	CManageThreadBase ();
	~CManageThreadBase ();


	//将当前线程注册到管理中
	int		Register(const char * in_pszManagerName, CThreadBase * in_poThread);
	//从管理类中注销指定线程
	bool	Unregister(CThreadBase * in_poThread);

	//查找当前线程对象
	CThreadBase * FindCurrentThread();

	//设置当前线程文件行号
	void			SetCurThrdSrcFileLine(char * in_pszSrcFileName, unsigned short	in_ushSrcFileLine );


	//开始线程利用率统计
	void	StartStatisticsRate();
	//停止线程利用率统计
	void	StopStatisticsRate();


	//打印所有线程信息
	void PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction);
	//打印指定应用线程信息,注意名称将使用like方式进行前匹配
	void PrintfInfoFrmManageName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszManagerName);
	//打印指定线程名的线程信息,注意名称将使用like方式进行前匹配
	void PrintfInfoFrmName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszThreadName, const char * in_pszManagerName=NULL);
	//打印超时线程信息
	bool ChkTimeOut(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction,
										  long in_lTimeOutSecond);

	//将指定的线程进行挂起
	bool		SetSuspFrmTheadID(unsigned	long in_ulThreadID);
	//将指定的线程进行激活
	bool		ActiveFrmThreadID(unsigned	long in_ulThreadID);
	//取指定线程的运行状态（线程挂起也是运行状态）
	virtual CThreadBase::eRunState	GetRunStateFrmThreadID(unsigned	long in_ulThreadID);	
	//指定的线程是否在运行中
	virtual	bool		IsRunFrmThreadID(unsigned	long in_ulThreadID);
	//指定的线程是否被挂起
	virtual	bool		IsSuspFrmThreadID(unsigned	long in_ulThreadID);

	//将指定的线程名称进行挂起,管理名称可以不填,注意名称将使用like方式进行前匹配
	bool		SetSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//将指定的线程名称进行激活,管理名称可以不填,注意名称将使用like方式进行前匹配
	bool		ActiveFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);

	//指定的线程名称是否都在运行中
	virtual	bool		IsRunFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//指定的线程名称是否都被挂起
	virtual	bool		IsSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);

	//将指定管理名称的线程进行挂起,注意名称将使用like方式进行前匹配
	bool		SetSuspFrmManagerName(const char * in_pszManagerName);
	//将指定管理名称的线程进行激活,注意名称将使用like方式进行前匹配
	bool		ActiveFrmManagerName(const char * in_pszManagerName);
	//指定的线程名称是否都在运行中
	virtual	bool		IsRunFrmManagerName(const char * in_pszManagerName );
	//指定的线程名称是否都被挂起
	virtual	bool		IsSuspFrmManagerName(const char * in_pszManagerName);

	//挂起所有的线程
	void		SuspendAll();
	//激活所有的线程
	void		ActivateAll();

	//所有的线程否都在运行中
	bool		IsRunAll();
	//除开自己的所有线程是否都被挂起
	bool		IsSuspUnselfAll();
	//除开自己的所有挂起线程
	void		SuspendUnselfAll();

	//杀死除自己之外的所有线程
	void		KillThreadUnselfAll(unsigned long lWaitSecond);
	//关闭所有的线程
	void		StopAllThread(void);
	//将指定管理名称的线程关闭,注意名称将使用like方式进行前匹配
	bool		StopFrmManagerName(const char * in_pszManagerName);
	//将指定线程名称的线程关闭,注意名称将使用like方式进行前匹配
	bool		StopFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//将指定的线程进行关闭
	bool		StopFrmThreadID(unsigned	long in_ulThreadID);

	//取注册线程的个数
	long	GetCount();
	//取指定位置的线程ID
	unsigned long GetThreadID(unsigned long in_ulPosID);

	//取指定位置的线程名称
	const char * GetThreadNameFrmPos(unsigned long in_ulPosID);
	//取指定线程的线程名称
	const char *  GetThreadNameFrmThrdID(unsigned long in_ulPosID);
	//取指定位置的线程应用名称
	const char * GetThreadAppNameFrmPos(unsigned long in_ulPosID);
	//取指定线程的线程应用名称
	const char *  GetThreadAppNameFrmThrdID(unsigned long in_ulPosID);

	/*****************************
	 * 函数名：rl_GetThreadCount
	 * 功能：取线程个数
	 * 参数：in_strName	输入-查询的线程名称（可以是前几个字相同）
	 *		out_plActivateCnt	输出-激活的线程数量(除停止和没有使用的)
	 *		out_plProcessingCount	输出-当前正在执行处理的线程个数
	 *		out_plSuspendCnt	输出-挂机线程个数
	 *		out_plOuttimeCnt	输出-超时线程数
	 * 返回：线程个数
	 *****************************/
	long		rl_GetThreadCount(const char * in_strName = NULL, long * out_plActivateCnt = NULL, long * out_plProcessingCount = NULL,
		long * out_plSuspendCnt = NULL, 
		long * out_plOuttimeCnt = NULL);	//取总线程数量





	//检查是否存在私有数据句柄
	bool	rbl_IsExistThrdPrvDataHandle();


	//线程私有数据句柄
#ifndef  LINUXCODE
	DWORD		GetThrdPrvDataHandle() ;
#else
	pthread_key_t	GetThrdPrvDataHandle() ;
#endif


protected:
	typedef struct _TLINKNODE
	{
		TMyListNode		stNode;		//链节点
		CThreadBase	*	poThread;	//指定的线程
		char			szName[32];
	}TLinkNode;
	TMyListNode			m_stHeadNode;
	char				m_szErrMsg[512];

	MCriticalSection		m_oMutex;
	
	unsigned short			m_ushThreadCount;

	//线程私有数据句柄
#ifndef  LINUXCODE
	DWORD						m_ThrdPrvDataHandle;	
#else
	pthread_key_t				m_ThrdPrvDataHandle;
	bool						m_blCrtThrdPrvDataHndl;
#endif



public:
	virtual const char * GetErrMsg(){return m_szErrMsg;};


};

//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
