//------------------------------------------------------------------------------------------------------------------------------
#include "MThread.h"
//------------------------------------------------------------------------------------------------------------------------------
//add by liuqy 20100817 for 线程改造，主要为统计打印线程信息增加一个基类
bool							MThreadBase::s_bStopAllThread = false;
MThreadBase					*	MThreadBase::s_poFirstThread = NULL;
bool							MThreadBase::s_blStatisticsRate = false;	//线程利用率统计标志
#ifndef LINUXCODE
CRITICAL_SECTION					MThreadBase::s_stSection;
#else
pthread_mutex_t						MThreadBase::s_stSection;
#endif
bool				MThreadBase::s_blSectionFlag = false;

//建立临界区
void MThreadBase::rv_CreateSection()
{
	if(s_blSectionFlag)
		return;
	#ifndef LINUXCODE
		::InitializeCriticalSection(&s_stSection);
	#else	
		/*
		 *	GUOGUO 2009-08-11 设置临界区锁的属性为可递归锁,支持同一线程连续调用lock
		 */
		//	ADD
		pthread_mutexattr_t attr;
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		//	ADD END

		pthread_mutex_init(&s_stSection,&attr);

		//	ADD
		pthread_mutexattr_destroy(&attr);
		//	ADD END
	#endif
	s_blSectionFlag = true;
}
//删除临界区
void MThreadBase::rv_DeleteSection()
{
	if(!s_blSectionFlag)
		return;
	#ifndef LINUXCODE
		::DeleteCriticalSection(&s_stSection);
	#else
		pthread_mutex_destroy(&s_stSection);
	#endif
	
	s_blSectionFlag = false;
}
//加锁
void	MThreadBase::rv_LockSection()
{
	if(!s_blSectionFlag)
		return;
	#ifndef LINUXCODE
		::EnterCriticalSection(&s_stSection);
	#else
		pthread_mutex_lock(&s_stSection);
	#endif

}
//解锁
void MThreadBase::rv_UnlockSection()
{
	if(!s_blSectionFlag)
		return;
	#ifndef LINUXCODE
		::LeaveCriticalSection(&s_stSection);
	#else
		pthread_mutex_unlock(&s_stSection);
	#endif
	
}


MThreadBase::MThreadBase(): MCheckTimeOutObjectList("MThread")
{
	//将对象加入到链表
	if ( s_poFirstThread != NULL )
	{
		s_poFirstThread->m_mPreThread = this;
	}
	else
		rv_CreateSection();
	m_mNextThread = s_poFirstThread;
	m_mPreThread = NULL;
	s_poFirstThread = this;
	
	rv_Reset();
}
MThreadBase::~MThreadBase()
{
	rv_LockSection();
	//将对象从链表中删除
	if ( m_mPreThread != NULL )
	{
		m_mPreThread->m_mNextThread = m_mNextThread;
	}
	
	if ( m_mNextThread != NULL )
	{
		m_mNextThread->m_mPreThread = m_mPreThread;
	}
	
	if ( s_poFirstThread == this )
	{
		s_poFirstThread = m_mNextThread;
	}
	rv_UnlockSection();
	if(NULL == s_poFirstThread)
	{
		rv_DeleteSection();
	}
}
void MThreadBase::rv_Reset()
{
	m_blSuspend = false;	//线程挂起
	m_blEntrySuspend = true;	//线程进入挂起状态
	m_blBeginWorkFlag = false;	//线程开始工作标志
	m_blEntryWorkStatis = false;	//线程一个工作统计状态开始，需要调用停止状态，让其进行停止状态
	m_stBeginWorkTime = 0;	//线程开始工作时间，自1970年以来的秒
	m_ulThreadID =0;	//线程ID 
	m_uiLoopCount = 0 ;	//线程循环次数
	m_uiWorkCount = 0;	//工作次数
	m_uiRealWorkUSECCount = 0;	//线程实际工作时间统计(微秒)
	m_blProcessingFlag = false;	//正在处理标志 add by liuqy 20101020 
	//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
	m_oWaitEvent.Active();

}
//刷新检查，主要用于线程进入挂起状态,如果是线程第一次进行，则需要设置开始时间
void MThreadBase::Refresh(void)
{

	//第一次进入，即线程开始运行了
	if(!m_blBeginWorkFlag)
	{
		m_blBeginWorkFlag = true;
		//取线程ID
		//为安全，在此处不取线程ID号，由建立线程处取得（因为调用此函数的可能不是在线程中）
		//m_ulThreadID = GetCurrentThreadId();
		time(&m_stBeginWorkTime);
	}
	//需要统计线程使用率时，增加循环次数，没有结束时间统计的，去结束。
	if(s_blStatisticsRate)
	{
		m_uiLoopCount++;
		rv_FinishWork();
	}
	MCheckTimeOutObjectList::Refresh();
	//线程需要挂起
	while(m_blSuspend)
	{
		m_blEntrySuspend = true;
		if ( s_bStopAllThread || m_bStopCurThread )
		{
			m_blSuspend = false;
			break;
		}
		//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
		inner_stopactive();
		m_oWaitEvent.Wait();
//		m_oWaitEvent.Wait(3000);
		inner_setactive();
		//MThread::Sleep(200);

	}
	m_blEntrySuspend = false;
}
//开始线程利用率统计
void	MThreadBase::rv_StartStatisticsRate()
{
	MThreadBase				*	poThreadBase;

	if(s_blStatisticsRate)
		return;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//循环设置每个线程
	while (NULL != poThreadBase)
	{
		poThreadBase->m_oBeginStatisCounter.SetCurTickCount();
		poThreadBase->m_uiLoopCount = 0;
		poThreadBase->m_uiWorkCount = 0;
		poThreadBase->m_uiRealWorkUSECCount = 0;
		poThreadBase->m_blEntryWorkStatis = false;
		
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();
	s_blStatisticsRate = true;
	
}
//add by liuqy 20100917
//通过命令行将指定的线程进行挂起
bool	MThreadBase::rbl_SetSuspFrmTheadID(unsigned	long in_ulThreadID)
{
	MThreadBase				*	poThreadBase;
	bool	blRet = false;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//循环设置每个线程
	while (NULL != poThreadBase)
	{
		if(in_ulThreadID == poThreadBase->m_ulThreadID)
		{
			poThreadBase->rv_SetSuspend();
			blRet = true;
			break;
		}
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();
	return blRet;
}
//通过命令行将指定的线程进行挂起
bool	MThreadBase::rbl_ActSuspFrmThreadID(unsigned	long in_ulThreadID)
{
	MThreadBase				*	poThreadBase;
	bool	blRet = false;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//循环设置每个线程
	while (NULL != poThreadBase)
	{
		if(in_ulThreadID == poThreadBase->m_ulThreadID)
		{
			poThreadBase->rv_ActivateFromSuspend();
			blRet = true;
			break;
		}
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();
	return blRet;
	
}
//end add
//打印所有线程信息
void MThreadBase::rv_PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction)
{
	rv_PrintfInfo(in_oCustomSelfPRT, NULL, fpFunction);
}
//add by liuqy 20100830 for 为保证输出数据的正确性，需要让出CPU控制权
void MThreadBase::rv_CpuYield()
{
#if defined(WIN32) || defined(MS_WINDOWS)
			Sleep(0);
			Sleep(0);
			Sleep(0);
#else
			sched_yield();
			sched_yield();
			sched_yield();
#endif
//			MThread::Sleep(1);
}
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
long		MThreadBase::rl_GetThreadCount(const char * in_strName, long * out_plActivateCnt, long * out_plProcessingCount, 
										   long * out_plSuspendCnt, long * out_plOuttimeCnt)
{
	MThreadBase				*	poThreadBase;
	long lTotalActivate = 0, lTotalProcessing = 0, lTotalSuspend = 0, lTotalTimeout = 0, lRet = 0;
	long	lFindNameLen = 0;
	
	//用于查询某些线程名
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	rv_LockSection();
	poThreadBase = s_poFirstThread;

	while (NULL != poThreadBase)
	{
		//按服务名查询时，只查询前几个相同名称的打印
		if(0 < lFindNameLen)
		{
			if(0 != memcmp(in_strName, poThreadBase->m_strObjectName, lFindNameLen))
			{
				poThreadBase = poThreadBase->m_mNextThread;
				continue;
				
			}
		}
		lRet ++;
		if(!poThreadBase->m_blBeginWorkFlag || 0 == poThreadBase->m_ulThreadID)
		{
		}
		else
		{
			if(!poThreadBase->rbl_GetRunState())
			{
			}
			else
			{
				lTotalActivate ++;
				if(poThreadBase->m_blEntrySuspend)
				{
					lTotalSuspend++;
				}
				else
				{
					//modify by liuqy 20101026 将超时时间修改了超过15秒就算超时
					if(15000 <= poThreadBase->m_mRefreshTime.GetDuration())
					{
						lTotalTimeout++;
					}
					else
					{
						if(poThreadBase->m_blProcessingFlag)	
							lTotalProcessing++;
					}
				}
			}
		}
		
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();
	if(NULL != out_plActivateCnt)
		*out_plActivateCnt = lTotalActivate;
	if(NULL != out_plProcessingCount)
		*out_plProcessingCount = lTotalProcessing;
	if(NULL != out_plSuspendCnt)
		*out_plSuspendCnt = lTotalSuspend;
	if(NULL != out_plOuttimeCnt)
		*out_plOuttimeCnt = lTotalTimeout;
	return lRet;
}

//打印指定线程信息
//modify by liuqy 20101210 for 将打印的百分比提高比例小数点
void MThreadBase::rv_PrintfInfo(void * in_oCustomSelfPRT, const char * in_strName, DF_rv_PrintReportCallBack * fpFunction)
{
	MThreadBase				*	poThreadBase;
	char						tempbuf[256];
	char						szDateTime[20];
	char						szWorkState[20];
	char						szSrcFileLine[64];
	MDateTime	*			poDateTime;
	double		dCountRate, dTimeRate;
	__int64	i64BeginTime;

	long	lFindNameLen = 0;
	//add by liuqy 20110225 打印超时时间
	long lTime;
	long lTotalUnuse = 0, lTotalStop = 0, lTotalSuspend = 0, lTotalTimeout = 0, lTotalRun=0;
	
	//用于查询某些线程名
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	memset(tempbuf, 0, sizeof(tempbuf));
	
#ifndef	_LINUXTRYOFF
	try
	{
#endif
//		my_snprintf(tempbuf, 256, "|  线程ID        线程名称         开始时间     状态  循环次数  工作次数 利用率    统计时长     工作时长  利用率");
		//modify by liuqy 20100926 for 打印线程的附加数据
		my_snprintf(tempbuf, 256, "| 线程ID       线程名称         开始时间     状态  附加");

		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();
		my_snprintf(tempbuf, 256, "|-------- ------------------ -------------- ------");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();
#ifndef	_LINUXTRYOFF
	}
	catch(...)
	{
	}
#endif

	rv_LockSection();
	poThreadBase = s_poFirstThread;

	while (NULL != poThreadBase)
	{
		//按服务名查询时，只查询前几个相同名称的打印
		if(0 < lFindNameLen)
		{
			if(0 != memcmp(in_strName, poThreadBase->m_strObjectName, lFindNameLen))
			{
				poThreadBase = poThreadBase->m_mNextThread;
				continue;
				
			}
		}
		memset(szSrcFileLine, 0, sizeof(szSrcFileLine));

//		if(!poThreadBase->m_blBeginWorkFlag || 0 == poThreadBase->m_ulThreadID)
		if(0 == poThreadBase->m_ulThreadID)
		{
			strcpy(szWorkState, "未用");
			lTotalUnuse++;
		}
		else
		{
			if(!poThreadBase->rbl_GetRunState())
			{
				//add by liuqy 20110225 打印超时时间
				memset(szWorkState, 0, sizeof(szWorkState));
				lTime = poThreadBase->m_mRefreshTime.GetDuration();
				lTime /= 1000;
				if(lTime < 60)
					sprintf(szWorkState, "停%d秒", lTime);
				else
				if(lTime < 3600)
					sprintf(szWorkState, "停%d分%d", lTime/60, lTime%60);
				else
				{
					lTime /= 60;
					sprintf(szWorkState, "停%d时%d", lTime/60, lTime%60);
				}
				lTotalStop ++;
			}
			else
			{
				if(poThreadBase->m_blEntrySuspend)
				{
					strcpy(szWorkState, "挂起");
					lTotalSuspend++;
				}
				else
				{
					//modify by liuqy 20101026 将超时时间修改了超过15秒就算超时
					if(15000 <= poThreadBase->m_mRefreshTime.GetDuration())
					{
						memset(szWorkState, 0, sizeof(szWorkState));
						//add by liuqy 20110225 打印超时时间
						lTime = poThreadBase->m_mRefreshTime.GetDuration();
						lTime /= 1000;
						if(lTime < 60)
							sprintf(szWorkState, "超%d秒", lTime);
						else
						if(lTime < 3600)
							sprintf(szWorkState, "超%d分%d", lTime/60, lTime%60);
						else
						{
							lTime /= 60;
							sprintf(szWorkState, "超%d时%d分", lTime/60, lTime%60);
						}


						lTotalTimeout++;
					}
					else
					{
						strcpy(szWorkState, "运行");
						lTotalRun ++;
					}
				}
				if(poThreadBase->m_pstSrcFileLine)
				{
					if(0 < strlen(poThreadBase->m_pstSrcFileLine->szFileName))
					{
						sprintf(szSrcFileLine, "%s:%d", poThreadBase->m_pstSrcFileLine->szFileName, poThreadBase->m_pstSrcFileLine->ushFileLine);
					}
				}
			}

		}
#ifndef	_LINUXTRYOFF
		try
		{
#endif
			poDateTime = new MDateTime(poThreadBase->m_stBeginWorkTime);
#ifndef	_LINUXTRYOFF
		}catch(...)
		{
			poDateTime = NULL;
		}
#endif
		if(NULL != poDateTime)
		{
			sprintf(szDateTime, "%02d/%02d %02d:%02d:%02d", poDateTime->GetMonth(), poDateTime->GetDay(), poDateTime->GetHour(), poDateTime->GetMinute(), poDateTime->GetSecond());
			delete poDateTime;
		}
		if(0 != poThreadBase->m_uiLoopCount)
		{
			i64BeginTime = poThreadBase->m_oBeginStatisCounter.GetI64Duration();
			if(0 != poThreadBase->m_uiLoopCount)
			{
				dCountRate = (__int64)poThreadBase->m_uiWorkCount;
				dCountRate = dCountRate * 100 / (__int64)poThreadBase->m_uiLoopCount;
			}
			else
				dCountRate = 0;
			if(0 < i64BeginTime)
			{
				dTimeRate = (__int64)poThreadBase->m_uiRealWorkUSECCount ;
				dTimeRate = dTimeRate * 100 / i64BeginTime;
			}
			else
			{
				dTimeRate = 0;
				i64BeginTime = 0;
			}
			//add by liuqy 20100926 for 打印线程的附加数据
			my_snprintf(tempbuf,256,"|%8u %-18s %-14s %-7s%4d %s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
			//	 dCountRate,
			//	 dTimeRate, 
				 poThreadBase->m_lAddtionData, szSrcFileLine);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10I64d %8I64d %6d %14I64d %10I64d %6d", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
				poThreadBase->m_uiLoopCount, poThreadBase->m_uiWorkCount, iCountRate,
				i64BeginTime, poThreadBase->m_uiRealWorkUSECCount, iTimeRate);
				*/

		}
		else
		{
			//add by liuqy 20100926 for 打印线程的附加数据
			my_snprintf(tempbuf,256,"|%8u %-18s %-14s %-7s%4d %s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
			//		"未统计", 
			//		"未统计", 
					poThreadBase->m_lAddtionData, szSrcFileLine);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10s %8s %6s %14s %10s %6s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
				"未统计", 		"未统计", 				"未统计", 
				"未统计", 		"未统计", 		"未统计");
				*/

		}
		
	#ifndef	_LINUXTRYOFF
		try
		{
	#endif
			fpFunction(in_oCustomSelfPRT, tempbuf);
	#ifndef	_LINUXTRYOFF
		}
		catch(...)
		{
			assert(0);
		}
	#endif
		rv_CpuYield();
		
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();
		my_snprintf(tempbuf, 256, "|------------------------------------------------------------------------");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();
		my_snprintf(tempbuf, 256, "|  总线程数       运行     超时数    挂起数     停用      未用");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();
		my_snprintf(tempbuf, 256, "|  %8d       %4d      %4d      %4d      %4d      %4d", 
			lTotalUnuse + lTotalStop + lTotalSuspend + lTotalTimeout + lTotalRun, 
			lTotalRun, lTotalTimeout, lTotalSuspend, lTotalStop, lTotalUnuse			);
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();

}
//add by liuqy 20110505 for 激活所有的线程
void		MThreadBase::rv_ActivateAll()
{
	//add by liuqy 20110505 for 停止所有的线程，则需要去激活所有的线程
	MThreadBase				*	poThreadBase;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//循环设置每个线程
	while (NULL != poThreadBase)
	{
		poThreadBase->rv_ActivateFromSuspend();
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();

}

//进入工作中...，主要用于统计线程利用率时使用。工作完成时，需要调用rv_FinishWork
void	MThreadBase::rv_EntryWorking()
{
	m_blProcessingFlag = true;	//正在处理标志 add by liuqy 20101020 
	if(!s_blStatisticsRate)
		return;
	rv_FinishWork();
	m_uiWorkCount++;
	m_oWorkStatisUsecCounter.SetCurTickCount();
	m_blEntryWorkStatis = true;
}
//工作结束，与进入工作中(rv_EntryWorking)配对使用
void	MThreadBase::rv_FinishWork()
{
	m_blProcessingFlag = false;	//正在处理标志 add by liuqy 20101020 
	if(!s_blStatisticsRate || !m_blEntryWorkStatis)
		return;
	m_uiRealWorkUSECCount += m_oWorkStatisUsecCounter.GetI64Duration();
	m_blEntryWorkStatis = false;
}
//取当前线程是否在运行状态（线程挂起也是运行状态）
bool	MThread::rbl_GetRunState()
{
#ifndef LINUXCODE

		register int				errorcode;
		
		if ( m_hRecordData != NULL )
		{
			errorcode = ::WaitForSingleObject(m_hRecordData, 0);
			if ( errorcode != WAIT_FAILED )
			{
				return true;
			}
		}

#else

		if ( m_hRecordData != 0 )
		{
			int	iRet;

			iRet = pthread_kill(m_hRecordData, 0);	//	发送0信号,这通常是用来检查线程是否存在
			if(iRet == 0)
				return true;
			//if(iRet == ESRCH)
		}
#endif
		return false;
}

//..............................................................................................................................
MThread::MThread(void) 
{
#ifndef	LINUXCODE
	m_hRecordData = NULL;
#else
	m_hRecordData = 0;
#endif

	m_bStopCurThread = TRUE;
}
//..............................................................................................................................
MThread::~MThread()
{
	StopThread();
}
//..............................................................................................................................
int  MThread::StartThread(MString strName,tagMTheadFunction fpFunction,void * lpParam,bool bCheckAlive)
{
	s_bStopAllThread = false;
	
	StopThread();

	m_bStopCurThread = false;
	//add by liuqy 20100817 for 启动线程时，需要复位系统参数
	rv_Reset();

	#ifndef LINUXCODE

		register int				errorcode;
		
		m_hRecordData = (HANDLE)_beginthreadex(	0,
												0,
												(unsigned int (__stdcall *)(void *))fpFunction,
												lpParam,
												0,
												(unsigned int *)&errorcode	);
		if ( m_hRecordData == NULL )
		{
			return(MErrorCode::GetSysErr());
		}
		//modify by liuqy 20100817 for 保存线程ID号
		//SetSpObejctName((strName+"_pid:"+MString(errorcode)).c_str());
		m_ulThreadID = errorcode;
		SetSpObejctName(strName.c_str());
		//end mdofiy by liuqy
		if ( bCheckAlive == true )
		{
			inner_setactive();
		}
		
		return(1);

	#else

		/*
		 *	LINUX pthread_create创建的线程的堆栈是10MB
		 *	我们的程序通常用不到那么大,所以调整为2MB
		 *	这样可以减少逻辑地址
		 */
		pthread_attr_t thread_attr;
		size_t stack_size;

		if(pthread_attr_init(&thread_attr) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		//设置线程栈空间为2M.
		stack_size = 2048*1024;
		if(pthread_attr_setstacksize(&thread_attr, stack_size) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		if ( pthread_create(&m_hRecordData,&thread_attr,fpFunction,lpParam) != 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		//add by liuqy 20100817 for 保存线程ID号
		m_ulThreadID = m_hRecordData;
		
		SetSpObejctName(strName.c_str());

		if ( bCheckAlive == true )
		{
			inner_setactive();
		}
		
		return(1);

	#endif
}

//..............................................................................................................................
void MThread::StopThread(unsigned long lWaitTime)
{
	//delete by liuqy 20110519 for 只能在最后去设置m_bLocked,否则检查挂起时会出错误
	//	inner_stopactive(); // 线程停止后，m_bLocked 置为false, 否则就会打印很多的线程超时，modify by anzl 20110408
	m_bStopCurThread = true;
	//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
	m_oWaitEvent.Active();

	#ifndef LINUXCODE

		register int				errorcode;
		
		if ( m_hRecordData != NULL )
		{
			errorcode = ::WaitForSingleObject(m_hRecordData,lWaitTime);
			if ( errorcode == WAIT_TIMEOUT || errorcode == WAIT_FAILED )
			{
				::TerminateThread(m_hRecordData,0);
			}
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}

	#else

		if ( m_hRecordData != 0 )
		{
			/*
			 *	等待线程结束,这里是无限制等待,其实是有问题的,
			 *	在我所知的有2个线程是不会因为设置了s_bStopAllThread而退出的,
			 *	一个是LINUX的模拟IOCP线程----做了detach处理,问题不大
			 *	二个是servicemanager中_tagCtrlTerminalStruct::threadfunc( void * Param ) 指令处理线程,也为LINUX做了做了detach处理
			 *	所有的问题都解决,不会导致ptheead_join死等
			 *	
			 *	如果出现死等,停止不到线程,有2种可能,
			 *	1.线程不判断s_bStopAllThread或者m_bStopCurThread而结束
			 *	2.线程在判断退出标志写法不对.当然我觉得这情况的可能性太小了.
			 *	WINDOWS 的WaitForSingleObject处理了超时等待,
			 *	非常不幸的是在LINUX下面pthread_join没有超时等待的处理
			 *	变相的采取了,POSIX的pthread_kill函数,发送0信号检测线程的存在性
			 *	来达到超时的处理方式.
			 *
			 *				GUOGUO 2009-10-28
			 *
			 */
			int	i, iRet=0, smt = 1;

			for(i = 0; i < lWaitTime; i++)
			{
			

				if(iRet == 0)
				{
#ifdef _DEBUG
//	printf("______________________Thread(%08X)(%ld) Active\n", m_hRecordData, lWaitTime);fflush(stdout);
#endif
					//	线程还存在
					this->rv_CpuYield();
				}
				else if(iRet == ESRCH)
				{
#ifdef _DEBUG
//	printf("______________________Thread(%08X)(%ld) already exit\n", m_hRecordData, lWaitTime);fflush(stdout);
#endif
					//	线程已经自我了断了.
					break;
				}
				else
				{
					this->rv_CpuYield();
					continue;
				}
			}
			if(i == lWaitTime)
			{
#ifdef _DEBUG
	printf("______________________Thread(%s:%08X)(%ld) Timeout\n", m_strObjectName, m_hRecordData, lWaitTime);fflush(stdout);
#endif
				//	超时了
				//pthread_cancel(m_hRecordData);			//	KILL
				pthread_kill(m_hRecordData, 9);			//	anzl 20170728
			}

			m_hRecordData = 0;
		}
	#endif
	//add by liuqy 20110225 停止时，不能打印超时响应，需要将标志设置
	inner_stopactive();
}
//..............................................................................................................................
bool MThread::GetThreadStopFlag(void)
{
	Refresh();

	if ( s_bStopAllThread == true || m_bStopCurThread == true )
	{
		//add by liuqy 20110225 停止时，不能打印超时响应，需要将标志设置
		inner_stopactive();
		return(true);
	}
	else
	{
		return(false);
	}
}
//add by liuqy 20110323 for 试取停止标志，直接返回标志，不进行任何操作
bool MThread::TestGetThreadStopFlag()
{
	if ( s_bStopAllThread == true || m_bStopCurThread == true )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//end 
//..............................................................................................................................
// 停止本线程（设置本线程标志）
void MThread::StopCurThread(void)
{
	m_bStopCurThread = true;
	//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
	m_oWaitEvent.Active();

}

//..............................................................................................................................
void MThread::StopAllThread(void)
{
	s_bStopAllThread = true;
	//add by liuqy 20110505 for 停止所有的线程，则需要去激活所有的线程
	rv_ActivateAll();

}

//..............................................................................................................................
void MThread::Sleep(unsigned long lMSec)
{
	#ifndef LINUXCODE
		::Sleep(lMSec);
	#else
		//usleep(lMSec*1000);
		//	LINUX系统推荐使用select做延时,usleep相当的不精确.这点上面从man usleep可以看到
		//	而且它有个最大的毛病
		//	不过定时器这种东西要依靠时钟中断的,有一微妙的时钟中断么？没有。
		//	中断都是毫秒级的,就是说目前情况下,
		//	usleep(1)肯定不会只延迟1微秒，最少一个时钟嘀嗒
		//	并且从等待队列混入运行队列也要受点气,
		//	首先运行队列选择进程的时候,是有歧视政策的。
		//	对从其他cpu移民过来的歧视,优先考虑本cpu土著,
		//	usleep以后有可能受歧视。对上任进程的亲戚要照顾，
		//	同一进程的两个线程间的切换有加分，usleep以后肯定也享受不到。
		//	所以想依靠usleep实现短延时是非常不可靠的。
		//	usleep()有有很大的问题

		//	在一些Linux 
		//	usleep()会影响信号 
		//	在某些Linux下，当参数的值必须小于1 * 1000 * 1000也就是1秒，否则该函数会报错，并且立即返回。 
		//	大部分平台的帮助文档已经明确说了，该函数是已经被舍弃的函数。 

		//				阳凌(GUOGUO)
		struct timeval stdelay;
		lMSec = lMSec * 1000;
		stdelay.tv_sec = lMSec / 1000000;
		stdelay.tv_usec = lMSec % 1000000;
		select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &stdelay);
	#endif
}

int MThread::CreateThreadExt( size_t stack_size, tagMTheadFunction fpFunction, 
							void* lpParam, eng_tid_t& tid )
{	
#ifndef LINUXCODE

		HANDLE						handle;
		register int				errorcode;
		
		handle = (HANDLE)_beginthreadex( 	NULL,
											stack_size,
											(unsigned int (__stdcall *)(void *))fpFunction,
											lpParam,
											0,
											(unsigned int *)&errorcode	);
		if ( handle == NULL )
		{
			return(MErrorCode::GetSysErr());
		}
		
		tid = errorcode;

		return 0;
	#else

		/*
		 *	LINUX pthread_create创建的线程的堆栈是10MB
		 *	我们的程序通常用不到那么大,所以调整为2MB
		 *	这样可以减少逻辑地址
		 */
		pthread_t		handle;
		pthread_attr_t 	thread_attr;

		if(pthread_attr_init(&thread_attr) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		//设置线程栈空间为2M.
		stack_size = 2048*1024;
		if(pthread_attr_setstacksize(&thread_attr, stack_size) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		if ( pthread_create(&handle, &thread_attr, fpFunction, lpParam ) != 0 )
		{
			return(MErrorCode::GetSysErr());
		}

		//add by liuqy 20100817 for 保存线程ID号
		
		tid = handle;

		return 0;
	#endif
}

void MThread::DetachThread(void)
{
#ifndef LINUXCODE
#else
	pthread_detach(pthread_self());
#endif
}

void MThread::ExitThread(void)
{
#ifndef LINUXCODE
	_endthreadex(0);
#else
	pthread_exit(0);
#endif
}

//------------------------------------------------------------------------------------------------------------------------------
MThreadClass::MThreadClass(void)
{

}
//..............................................................................................................................
MThreadClass::~MThreadClass()
{

}
//..............................................................................................................................
void * __stdcall MThreadClass::threadfunction(void * lpIn)
{
	MThreadClass				*	lpmclassptr;

	assert(lpIn != NULL);
	lpmclassptr = (MThreadClass *)lpIn;

	return((void *)(lpmclassptr->Execute()));
}
//..............................................................................................................................
int  MThreadClass::StartThread(MString strName)
{
	return(MThread::StartThread(strName,threadfunction,this));
}
//------------------------------------------------------------------------------------------------------------------------------
