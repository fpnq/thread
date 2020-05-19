//------------------------------------------------------------------------------------------------------------------------------
//单元名称：线程单元
//单元描述：主要处理线程创建、停止等操作
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
#ifndef __MEngine_MThreadH__
#define __MEngine_MThreadH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MMutex.h"

#ifndef	LINUXCODE
	typedef 	unsigned int			eng_tid_t;
#else
	typedef		pthread_t				eng_tid_t;	// pthread_t is not thread id, but the program used it as thread id
#endif

//下面定义了线程函数
typedef void * (__stdcall * tagMTheadFunction)(void *);
/************************************************************************
 *	功能：定义打印输出报告回调函数
 *	参数：in_oCustomSelfPRT	输入-用户打印时传入的指针，一般调用类打印功能时传入的this，这是只是原封返回
 *		in_pcszMsg		输入-打印的报告信息
 *	返回：无
 ************************************************************************/
#ifndef DF_rv_PrintReportCallBack
typedef void DF_rv_PrintReportCallBack(void * in_oCustomSelfPRT, const char * in_pcszMsg);
#endif
//..........................................................................................................................
//注意：线程函数应该按照如下方式书写
/*
void * __stdcall threadfunction(void *in)
{
	初始化一些局部或类变量，一般参数是传this进来

	//本线程没有停止和全局线程没有停止
	while ( GetThreadStopFlag() == false )
	{
		处理线程工作1
		//注意：该命令主要用于调试：SetSpObjectAddtionData(1);

		处理线程工作2
		//注意：该命令主要用于调试：SetSpObjectAddtionData(2);

		处理线程工作3
		//注意：该命令主要用于调试：SetSpObjectAddtionData(3);
	}
	  
	return(0);
}
*/
//add by liuqy 20100816 for 线程改造，主要为统计打印线程信息增加一个基类
class MThreadBase : public MCheckTimeOutObjectList
{
public:
	MThreadBase();
	~MThreadBase();
public:
	//取当前线程是否进入挂起状态
	bool			rbl_IsEntrySuspend(){
		//非加锁状态时，都表示已经挂起
		return (m_blEntrySuspend || !m_bLocked);
	};
	//设置线程挂起状态
	void			rv_SetSuspend(){
		m_blSuspend = true;
	};
	//激活挂起的线程
	void			rv_ActivateFromSuspend(){
		m_blSuspend = false;
		//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
		m_oWaitEvent.Active();
		rv_CpuYield(); 
		m_blSuspend = false;
		m_oWaitEvent.Active();
	};
	//进入工作中...，主要用于统计线程利用率时使用。工作完成时，需要调用rv_FinishWork
	void			rv_EntryWorking();
	//工作结束，与进入工作中(rv_EntryWorking)配对使用
	void			rv_FinishWork();
	//取当前线程是否在运行状态（线程挂起也是运行状态）
	virtual bool	rbl_GetRunState() = 0;		
	//add by liuqy 20101210 for 取工作中的状态
	//取当前线程是否进入了工作中
	bool			rbl_GetEntryWorkingFlag(){
		return m_blEntryWorkStatis;
	}
	//add by liuqy 20100830 for 为保证输出数据的正确性，需要让出CPU控制权
	static void rv_CpuYield();

public:
	//打印所有线程信息
	static void rv_PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction);
	//打印指定线程信息
	static void rv_PrintfInfo(void * in_oCustomSelfPRT, const char * in_strName, DF_rv_PrintReportCallBack * fpFunction);
	//停止线程利用率统计
	static void			rv_StopStatisticsRate(){
		s_blStatisticsRate = false;
	};
	//开始线程利用率统计
	static void			rv_StartStatisticsRate();
	//add by liuqy 20100827 for 增加取状态
	//取线程利用率统计情况
	static bool			rbl_GetStatisticsRate(){
		return s_blStatisticsRate;
	};
	//add by liuqy 20100917
	//通过命令行将指定的线程进行挂起
	static bool		rbl_SetSuspFrmTheadID(unsigned	long in_ulThreadID);
	//通过命令行将指定的线程进行挂起
	static bool		rbl_ActSuspFrmThreadID(unsigned	long in_ulThreadID);
	//add by liuqy 20110505 for 激活所有的线程
	static void		rv_ActivateAll();

	//add by liuqy 20101020 for 增加统计当前线程数及处理线程数
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
	static long		rl_GetThreadCount(const char * in_strName = NULL, long * out_plActivateCnt = NULL, long * out_plProcessingCount = NULL,
		long * out_plSuspendCnt = NULL, 
		long * out_plOuttimeCnt = NULL);	//取总线程数量
	//add by liuqy 20171213 for 线程停止后，必须复位相关参数
	void		rv_Reset();
	//end add by liuqy 20171213
	
protected:
	static MThreadBase		*	s_poFirstThread;
	static bool					s_blStatisticsRate;	//线程利用率统计标志
#ifndef LINUXCODE
	static 	CRITICAL_SECTION					s_stSection;
#else
	static 	pthread_mutex_t						s_stSection;
#endif
	static	bool				s_blSectionFlag;
	//建立临界区
	static void rv_CreateSection();
	//删除临界区
	static void rv_DeleteSection();
	//加锁
	static void	rv_LockSection();
	//解锁
	static void rv_UnlockSection();
protected:
	MThreadBase				*	m_mNextThread;
	MThreadBase				*	m_mPreThread;
	//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
	MWaitEvent					m_oWaitEvent;

	bool						m_blSuspend;	//线程挂起
	bool						m_blEntrySuspend;	//线程进入挂起状态
	bool						m_blBeginWorkFlag;	//线程开始工作标志

	bool						m_blEntryWorkStatis;	//线程一个工作统计状态开始，需要调用停止状态，让其进行停止状态
	MExactitudeCount			m_oWorkStatisUsecCounter;	//线程工作统计，统计结束时，需要将取得的值增加到实际工作时间中。
	MExactitudeCount			m_oBeginStatisCounter;	//线程开始工作统计时间
	time_t						m_stBeginWorkTime;	//线程开始工作时间，自1970年以来的秒
	//modify by liuqy 20131031 for 将线程号移到MCheckTimeOutObjectList中
	//unsigned	long			m_ulThreadID;	//线程ID
	unsigned __int64			m_uiLoopCount;	//线程循环次数
	unsigned __int64			m_uiWorkCount;	//工作次数
	unsigned __int64			m_uiRealWorkUSECCount;	//线程实际工作时间统计(微秒)
	static bool							s_bStopAllThread;
	bool						m_blProcessingFlag;	//正在处理标志 add by liuqy 20101020 
protected:
	//刷新检查，主要用于线程进入挂起状态
	void Refresh(void);
	bool								m_bStopCurThread;
	//复位设置



};
//end add by liuqy 
//注意：为了兼容Windows操作系统和Linux操作系统，去掉了暂停、恢复线程操作
//..........................................................................................................................
//modify by liuqy 20100816 for 线程改造，主要为统计打印线程信息增加一个基类,用代码实现线程挂起（非操作系统的挂起），记录线程工作情况
class MThread : public MThreadBase
{
	//将停止标志移动到基础类中
	/*
protected:
	static bool							s_bStopAllThread;
protected:
	bool								m_bStopCurThread;
	*/
protected:

	#ifndef LINUXCODE
		HANDLE							m_hRecordData;
	#else
		pthread_t						m_hRecordData;
	#endif
public:
	MThread(void);
	virtual ~MThread();
public:
	// 启动、停止线程
	int  StartThread(MString strName,tagMTheadFunction fpFunction,void * lpParam,bool bCheckAlive = true);
	void StopThread(unsigned long lWaitTime = 5000);

public:
	// 获取线程是否停止标志 注意线程需要处理一些操作，如刷新时间，挂起时，函数将不退出等待
	bool GetThreadStopFlag(void);
	//add by liuqy 20110323 for 试取停止标志，直接返回标志，不进行任何操作
	bool TestGetThreadStopFlag();
	// 停止本线程（设置本线程标志）
	void StopCurThread(void);

	//取当前线程是否在运行状态（线程挂起也是运行状态）
	bool	rbl_GetRunState();
public:
	// 停止该Modual的所有线程
	static void StopAllThread(void);

	// 休眠函数
	static void Sleep(unsigned long lMSec);


	static void ExitThread(void);
	static void DetachThread(void);
	static int  CreateThreadExt( size_t stack_size, tagMTheadFunction
				fpFunction, void* lpParam, eng_tid_t& tid );
};
//------------------------------------------------------------------------------------------------------------------------------
//下面是线程类的另外一个封装，新的线程类采用继承该线程的方式
class MThreadClass : public MThread
{
protected:
public:
	MThreadClass(void);
	virtual ~MThreadClass();
protected:
	static void * __stdcall threadfunction(void * lpIn);
public:
	//启动线程，停止线程使用基类的
	int  StartThread(MString strName);
public:
	//下面这个类是继承时必须要写的类，是线程的循环体，写法同基类（见上面）
	virtual int  Execute(void) = 0;
};
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
