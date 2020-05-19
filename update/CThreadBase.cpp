//------------------------------------------------------------------------------------------------------------------------------
#include "CThreadBase.h"
#include "../../UnitCode/string/my_string.h"
#include <new>

#ifdef	LINUXCODE
#ifndef my_snprintf
#define my_snprintf snprintf
#endif
#endif

CThreadBase::CThreadBase(const char * strObjectName)
{
	m_blExecRun0Flag = true;
#ifndef	LINUXCODE
	m_hRecordData = NULL;
#else
	m_hRecordData = 0;
#endif
	SetSpObejctName(strObjectName);
	Reset();
	m_poManageThread = NULL;
	
	
}
//当前线程使用方案一运行，还是方案二运行
CThreadBase::CThreadBase(bool in_blExecRun0Flag, const char * strObjectName)
{
	m_blExecRun0Flag = in_blExecRun0Flag;
#ifndef	LINUXCODE
	m_hRecordData = NULL;
#else
	m_hRecordData = 0;
#endif
	SetSpObejctName(strObjectName);
	Reset();
	m_poManageThread = NULL;

}
CThreadBase::~CThreadBase()
{
	Release();
	Unregister();
	m_poManageThread = NULL;
}
void CThreadBase::Release()
{
	if(RS_Stop < GetRunState())
	{
		Kill(50);
	}
}
//取当前线程ID
unsigned	long	CThreadBase::GetThreadID()
{
	return m_ulThreadID;
}

//注销注册的资源
void CThreadBase::Unregister()
{
	if(NULL != m_poManageThread)
		m_poManageThread->Unregister(this);
}

bool CThreadBase::IsEntrySuspend()
{
	//非运行状态时，都表示已经挂起
	return (m_blEntrySuspend || !m_blRunningFlag);
}
//设置线程挂起状态
void			CThreadBase::SetSuspend()
{
	m_blSuspend = true;
}
//激活挂起的线程
void		CThreadBase::ActivateFromSuspend()
{
	m_blSuspend = false;
	//让线程挂起时，停止运转，让线程去等待一个事件
	m_oWaitEvent.Active();
	CpuYield(); 
	CpuYield(); 
	m_blSuspend = false;
	m_oWaitEvent.Active();
}
//取当前线程是否在运行状态（线程挂起也是运行状态）
CThreadBase::eRunState	CThreadBase::GetRunState()
{
	//没有启动
	if(0 == m_ulThreadID && !m_blRunningFlag)
	{
		return RS_None;

	}
	
	if(!m_blRunningFlag )
	{
		if(m_blRunFlag)
			return RS_Starting;
		return RS_Stop;
	}

#ifndef LINUXCODE

		register int				errorcode;
		
		if ( m_hRecordData != NULL )
		{
			errorcode = ::WaitForSingleObject(m_hRecordData, 0);
			if ( errorcode == WAIT_FAILED )
			{
				return RS_Stop;
			}
		}

#else

		if ( m_hRecordData != 0 )
		{
			int	iRet;

			iRet = pthread_kill(m_hRecordData, 0);	//	发送0信号,这通常是用来检查线程是否存在
			if(iRet != 0)
				return RS_Stop;
		}
#endif


	if(m_blEntrySuspend)
		return RS_Suspend;

	if(!m_blRunFlag)
		return RS_None;
	return RS_Run;

}
//源文件的行号
void			CThreadBase::SetSrcFileLineName(char * in_pszSrcFileName, unsigned short	in_ushSrcFileLine )
{
	int lLen;

	m_ushSrcFileLine = in_ushSrcFileLine;

	const char * pszFile = rpsz_RemovePathFileName(in_pszSrcFileName);
	lLen = strlen(pszFile);
	if(lLen >= sizeof(m_szSrcFileName))
		lLen = sizeof(m_szSrcFileName)-1;
	memcpy(m_szSrcFileName, in_pszSrcFileName, lLen);
	m_szSrcFileName[lLen] = 0;
}


void CThreadBase::Reset()
{
	m_blRunFlag = false;
	m_blRunningFlag = false;
	m_blStatisticsRate = false;	//线程利用率统计标志
	m_blSuspend = false;	//线程挂起
	m_blEntrySuspend = true;	//线程进入挂起状态
	m_blEntryWorkStatis = false;	//线程一个工作统计状态开始，需要调用停止状态，让其进行停止状态
	m_stBeginWorkTime = 0;	//线程开始工作时间，自1970年以来的秒
	m_ulThreadID =0;	//线程ID 
	m_uiLoopCount = 0 ;	//线程循环次数
	m_uiWorkCount = 0;	//工作次数
	m_uiRealWorkUSECCount = 0;	//线程实际工作时间统计(微秒)
	m_blProcessingFlag = false;	//正在处理标志 add by liuqy 20101020 
	//add by liuqy 20110505 for 让线程挂起时，停止运转，让线程去等待一个事件
	m_oWaitEvent.Active();
	memset(m_szAddtionData, 0, sizeof(m_szAddtionData));
	m_lAddtionData = 0;

}
//停止线程利用率统计
void	CThreadBase::StopStatisticsRate()
{
	m_blStatisticsRate = false;
};
//取线程利用率统计情况
bool	CThreadBase::GetStatisticsRate()
{
	return m_blStatisticsRate;
};
//进入工作中...，主要用于统计线程利用率时使用。工作完成时，需要调用FinishWork
void	CThreadBase::EntryWorking()
{
	if(!m_blRunFlag)
		return ;
	m_blProcessingFlag = true;	//正在处理标志 add by liuqy 20101020 
	if(!m_blStatisticsRate || m_blEntryWorkStatis)
		return;
	FinishWork();
	m_uiWorkCount++;
	m_oWorkStatisUsecCounter.SetCurTickCount();
	m_blEntryWorkStatis = true;
}

//工作结束，与进入工作中(EntryWorking)配对使用
void		CThreadBase::FinishWork()
{
	m_blProcessingFlag = false;	//正在处理标志 add by liuqy 20101020 
	if(!m_blStatisticsRate || !m_blEntryWorkStatis)
		return;
	m_uiRealWorkUSECCount += m_oWorkStatisUsecCounter.GetI64Duration();
	m_blEntryWorkStatis = false;

}
//刷新检查，主要用于线程进入挂起状态,如果是线程第一次进行，则需要设置开始时间
void CThreadBase::Refresh(void)
{
	if(!m_blRunFlag)
		return ;
	//第一次进入，即线程开始运行了
	if(0 == m_stBeginWorkTime)
	{
		time(&m_stBeginWorkTime);
	}
	FinishWork();
	//需要统计线程使用率时，增加循环次数，没有结束时间统计的，去结束。
	if(m_blStatisticsRate)
	{
		m_uiLoopCount++;
	}
	m_mRefreshTime.SetCurTickCount();
	//线程需要挂起
	while(m_blSuspend)
	{
		m_mRefreshTime.SetCurTickCount();
		m_blEntrySuspend = true;
		if (  !m_blRunFlag )
		{
			m_blSuspend = false;
			break;
		}
		m_oWaitEvent.Wait(200);
	}
	m_blEntrySuspend = false;
}

// 休眠函数
void CThreadBase::Sleep(unsigned long lMSec)
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

		struct timeval stdelay;
		lMSec = lMSec * 1000;
		stdelay.tv_sec = lMSec / 1000000;
		stdelay.tv_usec = lMSec % 1000000;
		select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &stdelay);
	#endif

}

//让出CPU控制权一下
void CThreadBase::CpuYield()
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
}

//设置对象名称，以便于提示查找
void CThreadBase::SetSpObejctName(const char * strObjectName)
{
	long lLen;
	if(NULL == strObjectName)
		return;
	lLen = strlen(strObjectName);
	if(0 == lLen)
		return ;
	if(lLen > sizeof(m_szObjectName)-1)
		lLen = sizeof(m_szObjectName)-1;
	memcpy(m_szObjectName, strObjectName, lLen);
	m_szObjectName[lLen] = 0;

}

//设置对象附加数据，主要便于调试
void CThreadBase::SetObjectAddtionData(unsigned long lIn)
{
	m_lAddtionData = lIn;

}
//设置对象附加说明，主要便于调试,数据长度为16字节
void CThreadBase::SetObjectAddtionData(const char * in_pszNote)
{
	memcpy(m_szAddtionData, in_pszNote, sizeof(m_szAddtionData));
	m_szAddtionData[sizeof(m_szAddtionData)-1] = 0;
}
//	获取附加数据,只用于存入字符串时使用
const char * CThreadBase::GetObjectAddtionDataStr()
{
	return m_szAddtionData;
}
//	获取附加数据
unsigned long CThreadBase::GetObjectAddtionData()
{
	return m_lAddtionData;

}


//..............................................................................................................................
bool CThreadBase::GetThreadStopFlag(void)
{
	return !m_blRunFlag;
}
//线程是否正在行情中
bool CThreadBase::IsRunning()
{
	return m_blRunningFlag ;
}
//..............................................................................................................................
// 停止本线程（设置本线程标志）
void CThreadBase::StopThread(void)
{
	m_blRunFlag = false;
	m_oWaitEvent.Active();
}

unsigned int __stdcall CThreadBase::MyThreadRun(void * lpIn)
{
	CThreadBase				*	poSelf;

	poSelf = (CThreadBase *)lpIn;
	poSelf->m_blRunningFlag = true;
	if(NULL != poSelf->m_poManageThread)
	{
		if(poSelf->m_poManageThread->rbl_IsExistThrdPrvDataHandle())
		{

#ifndef  LINUXCODE
			TlsSetValue(poSelf->m_poManageThread->GetThrdPrvDataHandle(), (LPVOID)poSelf);
#else
			pthread_setspecific(poSelf->m_poManageThread->GetThrdPrvDataHandle(), (void*)poSelf);
#endif

		}
	}
	while(poSelf->m_blRunFlag)
	{
		poSelf->Refresh();
		if(poSelf->m_blExecRun0Flag)
		{
			if(poSelf->NeedRun0Again())
			{
				poSelf->EntryWorking();
				poSelf->Run0();
			}
			else
			{
				poSelf->m_oWaitEvent.Wait(200);
			}
		}
		else
		{
			poSelf->Run1();
		}
	}
	poSelf->m_blRunningFlag = false;
	return 0;
}
//运行方案一中：有立即运行要求
void	CThreadBase::SetRunEvent()
{
	m_oWaitEvent.Active();
}
int  CThreadBase::StartThread(const char * in_pszName)
{

	StopThread();
	//前一个线程如果没有停止，则等20次就去强杀
	CpuYield();
	
	if(RS_Stop < GetRunState())
	{
		Kill(1000);
	}
	Reset();

	//线程没有注册，不能启动
	if(NULL == m_poManageThread)
	{
		return -10;
	}
	m_blRunFlag = true;
	register int				errorcode;
	#ifndef LINUXCODE

		
		m_hRecordData = (HANDLE)_beginthreadex(	0,
												0,
												MyThreadRun,
												this,
												0,
												(unsigned int *)&errorcode	);
		if ( m_hRecordData == NULL )
		{

			errorcode = ::GetLastError();
			if(0 < errorcode)
				errorcode *= -1;
			if(0 == errorcode)
				errorcode = -1;
			m_blRunFlag = false;

			return errorcode;
		}
		//保存线程ID号
		m_ulThreadID = errorcode;

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
			errorcode = errno;
			if(0 < errorcode)
				errorcode *= -1;
			if(0 == errorcode)
				errorcode = -1;
			m_blRunFlag = false;
			return errorcode;
		}

		//设置线程栈空间为2M.
		stack_size = 2048*1024;
		if(pthread_attr_setstacksize(&thread_attr, stack_size) != 0)
		{
#ifndef LINUXCODE
			errorcode = ::GetLastError();
#else
			errorcode = errno;
#endif
			if(0 < errorcode)
				errorcode *= -1;
			if(0 == errorcode)
				errorcode = -1;
			m_blRunFlag = false;
			return errorcode;
		}

		if ( pthread_create(&m_hRecordData,&thread_attr, (void*(*)(void*))MyThreadRun, this) != 0 )
		{
#ifndef LINUXCODE
			errorcode = ::GetLastError();
#else
			errorcode = errno;
#endif
			if(0 < errorcode)
				errorcode *= -1;
			if(0 == errorcode)
				errorcode = -1;
			m_blRunFlag = false;
			return errorcode;
		}
		//保存线程ID号
		m_ulThreadID = m_hRecordData;
		

	#endif
		if(NULL != in_pszName && 0 < strlen(in_pszName))
			SetSpObejctName(in_pszName);
		
		return(1);
}

//..............................................................................................................................
//杀死当前线程
void CThreadBase::Kill(unsigned long lWaitTime)
{
	StopThread();

	#ifndef LINUXCODE

		register int				errorcode;
		
		if ( m_hRecordData != NULL )
		{
			errorcode = ::WaitForSingleObject(m_hRecordData, lWaitTime);
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
			int	i, iRet, smt = 1;

			for(i = 0; i < lWaitTime; i++)
			{
				iRet = pthread_kill(m_hRecordData, 0);	//	发送0信号,这通常是用来检查线程是否存在
				if(iRet == 0)
				{
					//	线程还存在
					this->CpuYield();
				}
				else if(iRet == ESRCH)
				{
					//	线程已经自我了断了.
					break;
				}
				else
				{
					this->CpuYield();
					continue;
				}
			}
			if(i == lWaitTime)
			{
				//	超时了
				pthread_cancel(m_hRecordData);			//	KILL
			}

			m_hRecordData = 0;
		}
	#endif
		m_blRunningFlag = false;

}



CManageThreadBase::CManageThreadBase()
{
	int errorcode;
	memset(&m_stHeadNode, 0, sizeof(m_stHeadNode));
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));	
	//add by liuqy 20130613 for 线程私有数据句柄
#ifndef  LINUXCODE
	m_ThrdPrvDataHandle = TLS_OUT_OF_INDEXES;	
#else
	m_blCrtThrdPrvDataHndl = false;
#endif
	//add by liuqy 20130613 for 线程私有数据句柄
#ifndef  LINUXCODE
	m_ThrdPrvDataHandle = TlsAlloc();
	errorcode = GetLastError();
	if(TLS_OUT_OF_INDEXES == m_ThrdPrvDataHandle )
		
#else
	errorcode = pthread_key_create(&m_ThrdPrvDataHandle, NULL);
	if(0 == errorcode)
	{
		m_blCrtThrdPrvDataHndl = true;
	}
	else
#endif
	{
		sprintf( m_szErrMsg,"创建线程私有数据错误句柄错误:%d %s", errorcode, MErrorCode::GetErrString(errorcode).c_str());
	}

}
CManageThreadBase::~CManageThreadBase()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	
	pstThreadNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		if(NULL != pstThreadNode)
#ifdef	LINUXCODE
			delete pstThreadNode;
#else
		{
			HeapFree(GetProcessHeap(), 0, pstThreadNode);
		}
#endif
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);

	}
	if(NULL != pstThreadNode)
#ifdef	LINUXCODE
		delete pstThreadNode;
#else
		HeapFree(GetProcessHeap(), 0, pstThreadNode);
#endif
	
	memset(&m_stHeadNode, 0, sizeof(m_stHeadNode));
	//add by liuqy 20130613 for 线程私有数据句柄
#ifndef  LINUXCODE
	if(TLS_OUT_OF_INDEXES != m_ThrdPrvDataHandle )
	{
		DWORD dwHandle = m_ThrdPrvDataHandle;
		m_ThrdPrvDataHandle = TLS_OUT_OF_INDEXES;
		CThreadBase::Sleep(10);
		TlsFree(dwHandle);
	}
#else
	if(	m_blCrtThrdPrvDataHndl)
	{
		m_blCrtThrdPrvDataHndl = false;
		CThreadBase::Sleep(10);
		pthread_key_delete(m_ThrdPrvDataHandle);
		
	}
#endif
	m_ushThreadCount = 0;

	oLocalSession.UnAttch();
}
//停止线程利用率统计
void	CManageThreadBase::StopStatisticsRate()
{
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			pstThreadNode->poThread->StopStatisticsRate();

		}
	}

	oLocalSession.UnAttch();
	
}

//开始线程利用率统计
void	CManageThreadBase::StartStatisticsRate()
{
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			pstThreadNode->poThread->m_oBeginStatisCounter.SetCurTickCount();
			pstThreadNode->poThread->m_uiLoopCount = 0;
			pstThreadNode->poThread->m_uiWorkCount = 0;
			pstThreadNode->poThread->m_uiRealWorkUSECCount = 0;
			pstThreadNode->poThread->m_blEntryWorkStatis = false;
			pstThreadNode->poThread->m_blStatisticsRate = true;

		}
	}

	oLocalSession.UnAttch();
	
}

//检查是否存在私有数据句柄
bool	CManageThreadBase::rbl_IsExistThrdPrvDataHandle()
{
#ifndef  LINUXCODE
	return	TLS_OUT_OF_INDEXES != m_ThrdPrvDataHandle ;
#else
	return m_blCrtThrdPrvDataHndl	;
#endif
}


//线程私有数据句柄
#ifndef  LINUXCODE
DWORD		CManageThreadBase::GetThrdPrvDataHandle() 
#else
pthread_key_t	CManageThreadBase::GetThrdPrvDataHandle()
#endif
{
	return	m_ThrdPrvDataHandle;	
};

//设置当前线程文件行号
void	CManageThreadBase::SetCurThrdSrcFileLine(char * in_pszSrcFileName, unsigned short	in_ushSrcFileLine)
{
	CThreadBase * poThread = FindCurrentThread();
	if(NULL != poThread)
		poThread->SetSrcFileLineName(in_pszSrcFileName, in_ushSrcFileLine );
}


//查找当前线程对象
CThreadBase * CManageThreadBase::FindCurrentThread()
{
	CThreadBase * 	poRet = NULL;
	TLinkNode	*		pstThreadNode;

	unsigned long ulCurThreadID = 	GetCurrentThreadId();
#ifndef  LINUXCODE
	if(TLS_OUT_OF_INDEXES != m_ThrdPrvDataHandle )
	{
		poRet = (CThreadBase	*)TlsGetValue(m_ThrdPrvDataHandle);
	}
#else
	if(m_blCrtThrdPrvDataHndl)
	{
		poRet =(CThreadBase	*)pthread_getspecific(m_ThrdPrvDataHandle);
	}
#endif
	if(NULL != poRet && ulCurThreadID == poRet->m_ulThreadID)
		return poRet;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(ulCurThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				poRet = pstThreadNode->poThread;
				break;
			}
		}
	}

	oLocalSession.UnAttch();

	return poRet;

}

//将指定的线程进行挂起
bool	CManageThreadBase::SetSuspFrmTheadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				pstThreadNode->poThread->SetSuspend();
				blRet = true;
				break;
			}
		}
	}

	oLocalSession.UnAttch();
	return blRet;
}
//取当前线程是否在运行状态（线程挂起也是运行状态）
CThreadBase::eRunState	CManageThreadBase::GetRunStateFrmThreadID(unsigned	long in_ulThreadID)
{
	CThreadBase::eRunState	eRet = CThreadBase::RS_None;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				eRet = pstThreadNode->poThread->GetRunState();
				break;
			}
		}
	}
	oLocalSession.UnAttch();
	if(NULL == pNode )
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的线程[%u]", in_ulThreadID);


	return eRet;

}	
//指定的线程是否在运行中
bool		CManageThreadBase::IsRunFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				blRet = (pstThreadNode->poThread->GetRunState() == CThreadBase::RS_Run);
				break;
			}
		}
	}
	oLocalSession.UnAttch();
	if(NULL == pNode )
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的线程[%u]", in_ulThreadID);


	return blRet;

}	
//指定的线程是否被挂起
bool		CManageThreadBase::IsSuspFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				blRet = (pstThreadNode->poThread->GetRunState() != CThreadBase::RS_Run);
				break;
			}
		}
	}
	oLocalSession.UnAttch();
	if(NULL == pNode )
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的线程[%u]", in_ulThreadID);


	return blRet;

}
//将指定的线程进行激活
bool	CManageThreadBase::ActiveFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				pstThreadNode->poThread->ActivateFromSuspend();
				blRet = true;
				break;
			}
		}
	}
	oLocalSession.UnAttch();
	if(!blRet)
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的激活线程[%u]", in_ulThreadID);


	return blRet;
	
}
//将指定的线程名称进行挂起,管理名称可以不填
bool	CManageThreadBase::SetSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入挂起线程名称");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
		{
			if(0 < lFindManagerNameLen && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerNameLen))
			{
				continue;
			}
			pstThreadNode->poThread->SetSuspend();
			blRet = true;
		}
		
	}
	oLocalSession.UnAttch();

	if(!blRet)
	{
		if(0 < lFindManagerNameLen)
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到[%s]下指定的挂起线程[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的挂起线程[%s]", in_pszThreadName);
	}

	return blRet;
}
//将指定的线程名称进行激活,管理名称可以不填
bool	CManageThreadBase::ActiveFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程名称");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
		{
			if(0 < lFindManagerNameLen && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerNameLen))
			{
				continue;
			}
			pstThreadNode->poThread->ActivateFromSuspend();
			blRet = true;

		}
		
	}
	oLocalSession.UnAttch();

	if(!blRet)
	{
		if(0 < lFindManagerNameLen)
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到[%s]下指定的激活线程[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到指定的激活线程[%s]", in_pszThreadName);
	}

	return blRet;
	
}

//指定的线程名称是否都在运行中
bool		CManageThreadBase::IsRunFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程名称");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
		{
			if(0 < lFindManagerNameLen && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerNameLen))
			{
				continue;
			}
			blRet = (pstThreadNode->poThread->GetRunState() == CThreadBase::RS_Run);
			if(!blRet)
				break;

		}
		
	}
	oLocalSession.UnAttch();
	return blRet;
}
	//指定的线程名称是否都被挂起
bool		CManageThreadBase::IsSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程名称");
		return true;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
		{
			if(0 < lFindManagerNameLen && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerNameLen))
			{
				continue;
			}
			blRet = (pstThreadNode->poThread->GetRunState() != CThreadBase::RS_Run);
			if(!blRet)
				break;

		}
		
	}
	oLocalSession.UnAttch();
	return blRet;

}
///////////////////
//将指定管理名称的线程进行挂起
bool	CManageThreadBase::SetSuspFrmManagerName( const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen =0;
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入挂起线程应用名称");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszManagerName, pstThreadNode->szName, lFindNameLen))
		{
			pstThreadNode->poThread->SetSuspend();
			blRet = true;			
		}
		
	}
	oLocalSession.UnAttch();
	if(!blRet)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到挂起线程应用名称[%s]", in_pszManagerName);
	}

	return blRet;
}
//将指定管理名称的线程进行激活
bool	CManageThreadBase::ActiveFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//用于查询某些线程名
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程应用名称");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszManagerName, pstThreadNode->szName, lFindNameLen))
		{
			pstThreadNode->poThread->ActivateFromSuspend();
			blRet = true;
			
		}		
	}
	oLocalSession.UnAttch();
	if(!blRet)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有找到激活线程应用名称[%s]", in_pszManagerName);
	}

	return blRet;
	
}
//指定的线程名称是否都在运行中
bool		CManageThreadBase::IsRunFrmManagerName(const char * in_pszManagerName )
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//用于查询某些线程名
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程应用名称");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszManagerName, pstThreadNode->szName, lFindNameLen))
		{
			blRet = (pstThreadNode->poThread->GetRunState() == CThreadBase::RS_Run);
			if(!blRet)
				break;
			
		}		
	}
	oLocalSession.UnAttch();

	return blRet;

}
//指定的线程名称是否都被挂起
bool		CManageThreadBase::IsSuspFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//用于查询某些线程名
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "没有输入激活线程应用名称");
		return blRet;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszManagerName, pstThreadNode->szName, lFindNameLen))
		{
			blRet = (pstThreadNode->poThread->GetRunState() != CThreadBase::RS_Run);
			if(!blRet)
				break;
			
		}		
	}
	oLocalSession.UnAttch();

	return blRet;

}

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
long		CManageThreadBase::rl_GetThreadCount(const char * in_strName, long * out_plActivateCnt, long * out_plProcessingCount, 
										   long * out_plSuspendCnt, long * out_plOuttimeCnt)
{
	TLinkNode	*		pstThreadNode;
	long lTotalActivate = 0, lTotalProcessing = 0, lTotalSuspend = 0, lTotalTimeout = 0, lRet = 0;
	long	lFindNameLen = 0;
	
	//用于查询某些线程名
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL == pstThreadNode->poThread)
		{
			continue;
			
		}
		//按服务名查询时，只查询前几个相同名称的打印
		if(0 < lFindNameLen)
		{
			if(0 != memcmp(in_strName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
			{
				continue;
				
			}
		}
		lRet ++;
		if(! pstThreadNode->poThread->m_blRunningFlag || 0 == pstThreadNode->poThread->m_ulThreadID)
		{

		}
		else
		{
			lTotalActivate ++;
			if(pstThreadNode->poThread->m_blEntrySuspend)
			{
				lTotalSuspend++;
			}
			else
			{
				if(15000 <= pstThreadNode->poThread->m_mRefreshTime.GetDuration())
				{
					lTotalTimeout++;
				}
				else
				{
					if(pstThreadNode->poThread->m_blProcessingFlag)	
						lTotalProcessing++;
				}
				
			}
		}
		
	}
	oLocalSession.UnAttch();
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
//打印所有线程信息
void CManageThreadBase::PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction)
{
	PrintfInfoFrmName(in_oCustomSelfPRT, fpFunction, NULL, NULL);
}


//打印指定应用线程信息
void CManageThreadBase::PrintfInfoFrmManageName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszManagerName)
{
	PrintfInfoFrmName(in_oCustomSelfPRT, fpFunction, NULL, in_pszManagerName);
	
}
//打印指定线程信息
void CManageThreadBase::PrintfInfoFrmName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction,
										  const char * in_pszThreadName, const char * in_pszManagerName)
{
	TLinkNode	*		pstThreadNode;
	char						tempbuf[256];
	char				szName[66];
	char						szDateTime[20];
	char						szWorkState[20];
	MDateTime	*			poDateTime;
	double		dCountRate, dTimeRate;
	__int64	i64BeginTime;

	long	lFindNameLen = 0, lFindManagerName = 0;
	//add by liuqy 20110225 打印超时时间
	long lTime;
	long lTotalUnuse = 0, lTotalStop = 0, lTotalSuspend = 0, lTotalTimeout = 0, lTotalRun=0;
	
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);

	if(NULL != in_pszManagerName)
		lFindManagerName = strlen(in_pszManagerName);

	memset(tempbuf, 0, sizeof(tempbuf));
	
	try{
//		my_snprintf(tempbuf, 256, "|  线程ID        线程名称         开始时间     状态  循环次数  工作次数 利用率    统计时长     工作时长  利用率");
		my_snprintf(tempbuf, sizeof(tempbuf)-1, "| 线程ID       线程名称         开始时间       状态   次数率 时间率 附加");

		fpFunction(in_oCustomSelfPRT, tempbuf);
		CThreadBase::CpuYield();
		my_snprintf(tempbuf, sizeof(tempbuf)-1, "|-------- ------------------ --------------- -------- ------ ------ -----");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		CThreadBase::CpuYield();
	}
	catch(...)
	{
	}

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL == pstThreadNode->poThread
			|| (0 < lFindNameLen && 0 != memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
			|| (0 < lFindManagerName && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerName)))
		{
			continue;
			
		}
//		if(!poThreadBase->m_blBeginWorkFlag || 0 == poThreadBase->m_ulThreadID)
		if(0 == pstThreadNode->poThread->m_ulThreadID)
		{
			strcpy(szWorkState, "未用");
			lTotalUnuse++;
		}
		else
		{
			if(CThreadBase::RS_Stop >= pstThreadNode->poThread->GetRunState())
			{
				memset(szWorkState, 0, sizeof(szWorkState));
				lTime = pstThreadNode->poThread->m_mRefreshTime.GetDuration();
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
				if(pstThreadNode->poThread->m_blEntrySuspend)
				{
					strcpy(szWorkState, "挂起");
					lTotalSuspend++;
				}
				else
				{
					//modify by liuqy 20101026 将超时时间修改了超过15秒就算超时
					if(15000 <= pstThreadNode->poThread->m_mRefreshTime.GetDuration())
					{
						memset(szWorkState, 0, sizeof(szWorkState));
						//add by liuqy 20110225 打印超时时间
						lTime = pstThreadNode->poThread->m_mRefreshTime.GetDuration();
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

		}
#ifndef	_LINUXTRYOFF
		try{
#endif
			poDateTime = new MDateTime(pstThreadNode->poThread->m_stBeginWorkTime);
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
		strcpy(szName, pstThreadNode->szName);
		strcat(szName, "-");
		strcat(szName, pstThreadNode->poThread->m_szObjectName);
		if(0 != pstThreadNode->poThread->m_uiLoopCount)
		{
			i64BeginTime = pstThreadNode->poThread->m_oBeginStatisCounter.GetI64Duration();
			if(0 != pstThreadNode->poThread->m_uiLoopCount)
			{
				dCountRate = (__int64)pstThreadNode->poThread->m_uiWorkCount;
				dCountRate = dCountRate * 100 / (__int64)pstThreadNode->poThread->m_uiLoopCount;
			}
			else
				dCountRate = 0;
			if(0 < i64BeginTime)
			{
				dTimeRate = (__int64)pstThreadNode->poThread->m_uiRealWorkUSECCount ;
				dTimeRate = dTimeRate * 100 / i64BeginTime;
			}
			else
			{
				dTimeRate = 0;
				i64BeginTime = 0;
			}
			my_snprintf(tempbuf, sizeof(tempbuf)-1,"|%8u %-18s %-15s %-8s %6.2f %6.2f %0d %s", 
				pstThreadNode->poThread->m_ulThreadID, szName, szDateTime, szWorkState,
				 dCountRate,
				 dTimeRate, pstThreadNode->poThread->m_lAddtionData,
				 pstThreadNode->poThread->m_szAddtionData);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10I64d %8I64d %6d %14I64d %10I64d %6d", 
				poThreadBase->m_ulThreadID, poThreadBase->m_szObjectName, szDateTime, szWorkState,
				poThreadBase->m_uiLoopCount, poThreadBase->m_uiWorkCount, iCountRate,
				i64BeginTime, poThreadBase->m_uiRealWorkUSECCount, iTimeRate);
				*/

		}
		else
		{
			my_snprintf(tempbuf, sizeof(tempbuf)-1, "|%8u %-18s %-15s %-8s %6s %6s %0d %s", 
				pstThreadNode->poThread->m_ulThreadID, szName, szDateTime, szWorkState,
					"未统计", 		"未统计", pstThreadNode->poThread->m_lAddtionData,
				 pstThreadNode->poThread->m_szAddtionData);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10s %8s %6s %14s %10s %6s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_szObjectName, szDateTime, szWorkState,
				"未统计", 		"未统计", 				"未统计", 
				"未统计", 		"未统计", 		"未统计");
				*/

		}

		try
		{
			fpFunction(in_oCustomSelfPRT, tempbuf);
			CThreadBase::CpuYield();
		}
		catch(...)
		{
			
					assert(0);
			
		}
	
	}
	oLocalSession.UnAttch();
		my_snprintf(tempbuf, 256, "|------------------------------------------------------------------------");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		my_snprintf(tempbuf, 256, "|  总线程数       运行     超时数    挂起数     停用      未用");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		my_snprintf(tempbuf, 256, "|  %8d       %4d      %4d      %4d      %4d      %4d", 
			lTotalUnuse + lTotalStop + lTotalSuspend + lTotalTimeout + lTotalRun, 
			lTotalRun, lTotalTimeout, lTotalSuspend, lTotalStop, lTotalUnuse			);
		fpFunction(in_oCustomSelfPRT, tempbuf);

}

//打印超时线程信息
bool CManageThreadBase::ChkTimeOut(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction,
										  long in_lTimeOutSecond)
{
	TLinkNode	*		pstThreadNode;
	char						tempbuf[256];
	char			szTime[32];
	long	lTime;
	bool	blRet = false;

	memset(tempbuf, 0, sizeof(tempbuf));
	
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL == pstThreadNode->poThread)
		{
			continue;
			
		}
		if(pstThreadNode->poThread->m_blRunFlag && pstThreadNode->poThread->m_blRunningFlag)
		{
			lTime = pstThreadNode->poThread->m_mRefreshTime.GetDuration();
			lTime /= 1000;
			if(lTime >= in_lTimeOutSecond)
			{
			
				blRet = true;
				if(NULL == fpFunction)
					break;

				if(lTime < 60)
					sprintf(szTime, "%d秒", lTime);
				else
				if(lTime < 3600)
					sprintf(szTime, "%d分%d秒", lTime/60, lTime%60);
				else
				{
					lTime /= 60;
					sprintf(szTime, "%d时%d分", lTime/60, lTime%60);
				}
				my_snprintf(tempbuf, sizeof(tempbuf)-1,"<%s>线程[%s]ID[%u]停止响应%s[附加数据%d]", 
					pstThreadNode->szName, pstThreadNode->poThread->m_szObjectName, 
					pstThreadNode->poThread->m_ulThreadID, szTime, pstThreadNode->poThread->m_lAddtionData);

				try
				{
					fpFunction(in_oCustomSelfPRT, tempbuf);
				}
				catch(...)
				{
					assert(0);
				}
			}
		}
	}
	return blRet;
}


//激活所有的线程
void		CManageThreadBase::ActivateAll()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			pstThreadNode->poThread->ActivateFromSuspend();
		}

	}
	oLocalSession.UnAttch();

}
//所有的线程否都在运行中
bool	CManageThreadBase::IsRunAll()
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			blRet = (pstThreadNode->poThread->GetRunState() == CThreadBase::RS_Run);
			if(!blRet)
				break;
		}

	}
	oLocalSession.UnAttch();
	return blRet;
}
//除开自己的所有线程是否都被挂起
bool		CManageThreadBase::IsSuspUnselfAll()
{
	bool	blRet = true;
	unsigned long		ulThreadID = GetCurrentThreadId();
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && ulThreadID != pstThreadNode->poThread->m_ulThreadID)
		{
			blRet = (pstThreadNode->poThread->GetRunState() != CThreadBase::RS_Run);
			if(!blRet)
				break;
		}

	}
	oLocalSession.UnAttch();
	return blRet;

}

//除开自己的所有挂起线程
void		CManageThreadBase::SuspendUnselfAll()
{
	TLinkNode	*		pstThreadNode;
	unsigned long		ulThreadID = GetCurrentThreadId();
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && ulThreadID != pstThreadNode->poThread->m_ulThreadID)
		{
			pstThreadNode->poThread->SetSuspend();
		}

	}
	oLocalSession.UnAttch();
}
//挂起所有的线程
void		CManageThreadBase::SuspendAll()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			pstThreadNode->poThread->SetSuspend();
		}

	}
	oLocalSession.UnAttch();
}

//取注册线程的个数
long	CManageThreadBase::GetCount()
{
	long lRet = 0;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			lRet++;
		}

	}
	oLocalSession.UnAttch();
	return lRet;
}
//取指定位置的线程ID
unsigned long CManageThreadBase::GetThreadID(unsigned long in_ulPosID)
{
	unsigned long lRet = 0;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			lPos++;
			if(lPos == in_ulPosID)
			{
				lRet = pstThreadNode->poThread->m_ulThreadID;
				break;
			}
		}

	}
	oLocalSession.UnAttch();
	return lRet;

}
//取指定位置的线程名称
const char *  CManageThreadBase::GetThreadNameFrmPos(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			lPos++;
			if(lPos == in_ulPosID)
			{
				pszRet = pstThreadNode->poThread->m_szObjectName;
				break;
			}
		}

	}
	oLocalSession.UnAttch();
	return pszRet;

}
//取指定线程的线程名称
const char *  CManageThreadBase::GetThreadNameFrmThrdID(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			
			if(in_ulPosID == pstThreadNode->poThread->m_ulThreadID)
			{
				pszRet = pstThreadNode->poThread->m_szObjectName;
				break;
			}
		}

	}
	oLocalSession.UnAttch();
	return pszRet;

}
//取指定位置的线程名称
const char *  CManageThreadBase::GetThreadAppNameFrmPos(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			lPos++;
			if(lPos == in_ulPosID)
			{
				pszRet = pstThreadNode->szName;
				break;
			}
		}

	}
	oLocalSession.UnAttch();
	return pszRet;

}
//取指定线程的线程名称
const char *  CManageThreadBase::GetThreadAppNameFrmThrdID(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			
			if(in_ulPosID == pstThreadNode->poThread->m_ulThreadID)
			{
				pszRet = pstThreadNode->szName;
				break;
			}
		}

	}
	oLocalSession.UnAttch();
	return pszRet;

}

//..............................................................................................................................
void CManageThreadBase::StopAllThread(void)
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			pstThreadNode->poThread->StopThread();
		}

	}
	oLocalSession.UnAttch();

}
//将指定管理名称的线程关闭,注意名称将使用like方式进行前匹配
bool		CManageThreadBase::StopFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen =0;
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时输入线程应用名称");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszManagerName, pstThreadNode->szName, lFindNameLen))
		{
			pstThreadNode->poThread->StopThread();
			blRet = true;			
		}
		
	}
	oLocalSession.UnAttch();
	if(!blRet)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时没有找到线程应用名称[%s]", in_pszManagerName);
	}

	return blRet;
}
//将指定线程名称的线程关闭,注意名称将使用like方式进行前匹配
bool		CManageThreadBase::StopFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName )
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//用于查询某些线程名
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//没有输入名字，则退出
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时输入线程名称");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && 0 == memcmp(in_pszThreadName, pstThreadNode->poThread->m_szObjectName, lFindNameLen))
		{
			if(0 < lFindManagerNameLen && 0 != memcmp(in_pszManagerName, pstThreadNode->szName, lFindManagerNameLen))
			{
				continue;
			}
			pstThreadNode->poThread->StopThread();
			blRet = true;

		}
		
	}
	oLocalSession.UnAttch();
	if(!blRet)
	{
		if(0 < lFindManagerNameLen)
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时没有找到[%s]下指定的线程[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时没有找到指定的线程[%s]", in_pszThreadName);
	}
	return blRet;
}
//将指定的线程进行关闭
bool		CManageThreadBase::StopFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread)
		{
			if(in_ulThreadID == pstThreadNode->poThread->m_ulThreadID)
			{
				pstThreadNode->poThread->StopThread();
				blRet = true;
				break;
			}
		}
	}
	oLocalSession.UnAttch();
	if(!blRet)
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "停止时没有找到的线程[%u]", in_ulThreadID);

	return blRet;
}
//杀死除自己之外的所有线程
void	CManageThreadBase::KillThreadUnselfAll(unsigned long lWaitSecond)
{
	time_t stStartTime;
	long lWaitTime;
	stStartTime = time(NULL);
	StopAllThread();
	unsigned long		ulThreadID = GetCurrentThreadId();
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL != pstThreadNode->poThread && ulThreadID != pstThreadNode->poThread->m_ulThreadID)
		{
			if(pstThreadNode->poThread->GetRunState() >= CThreadBase::RS_Run)
			{
				lWaitTime = time(NULL) - stStartTime;
				if(lWaitTime >= lWaitSecond)
					lWaitTime = 1;
				else
				{
					lWaitTime = lWaitSecond - lWaitTime;
					lWaitTime *= 1000;
				}
				pstThreadNode->poThread->Kill(lWaitTime);
			}
		}

	}
	oLocalSession.UnAttch();
	
}

//将当前线程注册到管理中
int		CManageThreadBase::Register(const char * in_pszManagerName, CThreadBase * in_poThread)
{
	int	lRet = -1;
	TLinkNode	*		pstThreadNode;
	long	lLen = 0;
	long lCmp;
	TLinkNode	*		pstAddThreadNode;
	char	szName[sizeof(pstAddThreadNode->szName)] ;

	//用于查询某些线程名
	if(NULL != in_pszManagerName)
		lLen = strlen(in_pszManagerName);
	//没有输入名字，则退出
	if(0 >= lLen || NULL == in_poThread)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "注册时没有输入线程应用名称或线程对象为NULL");
		return -2;
	}
	lCmp = strlen(in_poThread->m_szObjectName);
	if(0 == lCmp)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "注册时输入线程对象名称为空");
		return -3;
	}
	memset(szName, 0, sizeof(szName));
	if(lLen > sizeof(szName)-1)
		lLen = sizeof(szName)-1;
	memcpy(szName, in_pszManagerName, lLen);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	TMyListNode * pAddNodePos = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		lCmp = strcmp(szName, pstThreadNode->szName);
		if(0 > lCmp)
			break;
		pAddNodePos = pNode;
		if(0 == lCmp && in_poThread == pstThreadNode->poThread )
		{
			//重复增加，则不增加
			oLocalSession.UnAttch();
			return 0;
		}

	}
#ifndef	_LINUXTRYOFF
	try{
#endif
#ifdef	LINUXCODE
		pstAddThreadNode = new TLinkNode;
#else	//windows 内存分配
		pstAddThreadNode = new (HeapAlloc(GetProcessHeap(), 0, sizeof(TLinkNode))) TLinkNode;
#endif
#ifndef	_LINUXTRYOFF
	}catch(...)
	{
		pstAddThreadNode = NULL;
	}
#endif
	if(NULL == pstAddThreadNode)
	{
		oLocalSession.UnAttch();
		lRet = MErrorCode::GetSysErr();
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "注册时分配线程管理节点失败[%d]", lRet);
		return lRet;
	}
	memset(pstAddThreadNode, 0, sizeof(*pstAddThreadNode));
	lLen = strlen(szName);
	if(lLen > sizeof(pstAddThreadNode->szName)-1)
		lLen = sizeof(pstAddThreadNode->szName)-1;

	memcpy(pstAddThreadNode->szName, szName, lLen);
	pstAddThreadNode->poThread = in_poThread;
	if(NULL == pAddNodePos)
		rv_AddListNode(&m_stHeadNode, &(pstAddThreadNode->stNode));
	else
	{
		rv_AddListNode(pAddNodePos, &(pstAddThreadNode->stNode));
	}
	pstAddThreadNode->poThread->m_poManageThread = this;
	m_ushThreadCount++;
	oLocalSession.UnAttch();
	
	return 1;
}
//从管理类中注销指定线程
bool		CManageThreadBase::Unregister(CThreadBase * in_poThread)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	//当前指针已经不存在了，则直接退出
	if(NULL == this)
		return true;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//查询所有节点
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(in_poThread == pstThreadNode->poThread)
		{
			rv_DelListNode(&pstThreadNode->stNode);
			in_poThread->m_poManageThread = NULL;
			m_ushThreadCount--;
#ifdef	LINUXCODE
			delete pstThreadNode;
#else
			HeapFree(GetProcessHeap(), 0, pstThreadNode);
#endif
			blRet = true;
			break;
		}
	}
	oLocalSession.UnAttch();
	if(!blRet)
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "注销时没有找到当前的线程");

	return blRet;

}
