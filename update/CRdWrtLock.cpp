/***********************************************************************
 * 文件名称：CRdWrtLock.cpp
 * 功能描述：实现读写锁功能
 * 开发历史：liuqy 20110111 cd
 * 读写锁实现如下：请见头文件
 *********************************************************************/

#include "CRdWrtLock.h"
#include "../BaseUnit/MThread.h"

extern const char * rpsz_RemovePathFileName(const char*);

//初始化一个节点
void rv_InitListNode(TMyListNode * in_pstNode)
{
	memset(in_pstNode, 0, sizeof(TMyListNode));
}
//向头上增加一个节点
void rv_AddListNode(TMyListNode * in_pstHead, TMyListNode * in_pstAddNode)
{
	if(NULL != in_pstHead->m_pstNext)
		in_pstHead->m_pstNext->m_pstPrevious = in_pstAddNode;
	in_pstAddNode->m_pstNext = in_pstHead->m_pstNext;
	in_pstAddNode->m_pstPrevious = in_pstHead;
	in_pstHead->m_pstNext = in_pstAddNode;
}
//从节点中删除
void rv_DelListNode(TMyListNode * in_pstDelNode)
{
	if(NULL != in_pstDelNode->m_pstPrevious)
		in_pstDelNode->m_pstPrevious->m_pstNext = in_pstDelNode->m_pstNext;
	if(NULL != in_pstDelNode->m_pstNext)
		in_pstDelNode->m_pstNext->m_pstPrevious = in_pstDelNode->m_pstPrevious;
		
	in_pstDelNode->m_pstNext = NULL;
	in_pstDelNode->m_pstPrevious = NULL;
}

/*******************************************************
       	函 数 名: rl_WriteLock
       	功能描述: 写锁加锁
       	参    数: in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       	返回说明: 0成功 1加锁成功	 -1失败
********************************************************/
long CLocalRdWrtLockSession::rl_WriteLock(long in_lTimeOut)
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行读加锁");
		return -1;
	}

	return m_poRdWrtLockRsc->rl_WriteLock(this, in_lTimeOut);

}
long CLocalRdWrtLockSession::rl_WriteLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo)
{
	long lRet;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行读加锁");
		return -1;
	}
	
	lRet = m_poRdWrtLockRsc->rl_WriteLock(this, in_lTimeOut);
	if(0 <= lRet)
	{
		m_stReadNode.m_ushFileLineNo = in_ushFileLineNo;
		strncpy(m_stReadNode.m_szFileName, rpsz_RemovePathFileName(in_pszFileName), sizeof(m_stReadNode.m_szFileName)-1);

	}
	return lRet;
}

/*******************************************************
       	函 数 名: rl_ReadLock
        功能描述: 读锁加锁,注意如果以前有过写锁没有解锁，则系统直接返回成功
       	参    数: in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       	返回说明: 0成功	 1加锁成功 -1失败
********************************************************/
long CLocalRdWrtLockSession::rl_ReadLock(long in_lTimeOut)
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行读加锁");
		return -1;
	}
	
	return m_poRdWrtLockRsc->rl_ReadLock(this, in_lTimeOut);
	
}
long CLocalRdWrtLockSession::rl_ReadLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo)
{
	int lRet;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行读加锁");
		return -1;
	}

	lRet = m_poRdWrtLockRsc->rl_ReadLock(this, in_lTimeOut);
	if(0 <= lRet)
	{
		m_stReadNode.m_ushFileLineNo = in_ushFileLineNo;
		strncpy(m_stReadNode.m_szFileName, rpsz_RemovePathFileName(in_pszFileName), sizeof(m_stReadNode.m_szFileName)-1);
		
	}
	return lRet;

}

/*******************************************************
       	函 数 名: rl_UnlockWrtKeepRd
       	功能描述: 解写锁，保留读锁
       	参    数: 无
       	返回说明: 0成功	 -1失败
********************************************************/
long CLocalRdWrtLockSession::rl_UnlockWrtKeepRd()
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行解锁");
		return -1;
	}

	return m_poRdWrtLockRsc->rl_UnlockWrtKeepRd(this);
}
/*******************************************************
	  函 数 名: rl_Unlock
	   	功能描述: 解所有的锁
	   	参    数: 无
	    返回说明: 0成功	 -1失败
********************************************************/
long CLocalRdWrtLockSession::rl_Unlock( )
{
	long lRet1 = 0, lRet2 = 0;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "没有读写锁资源，不能进行解锁");
		return -1;
	}
	//有写锁，则需要去解写锁
	if(m_blWriteLockFlag)
	{
		lRet1 = m_poRdWrtLockRsc->rl_UnlockWrt(this);
	}
	//有读锁，则需要去解读写
	if(m_blReadLockFlag)
	{
		lRet2 = m_poRdWrtLockRsc->rl_UnlockRd(this);
	}
	if(0 > lRet1)
		return lRet1;
	return lRet2;
}

/*******************************************************
       	函 数 名: re_GetLockState
       	功能描述: 取当前加锁情况
       	参    数: 无
       	返回说明: None未加锁，Read已经加过读锁，Write已经加过写锁
********************************************************/
CLocalRdWrtLockSession::ELockFlag	CLocalRdWrtLockSession::re_GetLockState()
{
	if(m_blWriteLockFlag)
		return Write;
	if(m_blReadLockFlag)
		return Read;
	return None;
}
CLocalRdWrtLockSession::CLocalRdWrtLockSession()
{
	memset(&m_stReadNode, 0, sizeof(m_stReadNode));
	m_blReadLockFlag = false;	//读加锁标记
	m_blWriteLockFlag = false;	//写加锁标记
	m_poRdWrtLockRsc = NULL;	//读写锁资源
	memset(&m_szErrMsg[0], 0, sizeof(m_szErrMsg));	//错误信息
	
}
//设置读写锁资源
void CLocalRdWrtLockSession::SetLockRsc(CRdWrtLockResource & in_oRdWrtLockRsc)
{
	rl_Unlock();
	m_poRdWrtLockRsc = &in_oRdWrtLockRsc;	//读写锁资源
	
}

CLocalRdWrtLockSession::CLocalRdWrtLockSession(CRdWrtLockResource * in_poRdWrtLockRsc)
{
	memset(&m_stReadNode, 0, sizeof(m_stReadNode));
	m_blReadLockFlag = false;	//读加锁标记
	m_blWriteLockFlag = false;	//写加锁标记
	m_poRdWrtLockRsc = in_poRdWrtLockRsc;	//读写锁资源
	memset(&m_szErrMsg[0], 0, sizeof(m_szErrMsg));	//错误信息
	
}

CLocalRdWrtLockSession::~CLocalRdWrtLockSession()
{
	rl_Unlock();
}


/*******************************************************
       	函 数 名: rl_WriteLock
       	功能描述: 写锁加锁
       	参    数: in_poRdWrtSession	输入--读写锁操作类
				in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       	返回说明: 0成功 	 -1失败
********************************************************/
long CRdWrtLockResource::rl_WriteLock(CLocalRdWrtLockSession * in_poRdWrtSession,  long in_lTimeOut)
{
	long lNowTime,lStartTime;
	long lWaitTime;
	unsigned long	ulThreadID;	//当前线ID
	TMyListNode * pNode;
	CLocalRdWrtLockSession::TLockNode	* pstLockNode;
	unsigned long ulCount;
	long lRet = -1;
	
	ulThreadID = GetCurrentThreadId();
	//当前的写锁已经被自己加了，直接返回成功
	if(ulThreadID == m_ulWrtThreadID)
		return 0;

	if(in_poRdWrtSession->m_blWriteLockFlag )
	{
		sprintf(in_poRdWrtSession->m_szErrMsg, "已经加了写锁，不能再写锁");				
		return -1;
	}

	time(&lStartTime);
#ifndef LINUXCODE
	//先加写锁
	::EnterCriticalSection(&m_stWriterSection);
#else
	pthread_mutex_lock(&m_stWriterSection);
#endif

	if(!m_blAllowReadFlag)
	{
#ifndef LINUXCODE
		::LeaveCriticalSection(&m_stWriterSection);
#else
		pthread_mutex_unlock(&m_stWriterSection);
#endif
		sprintf(in_poRdWrtSession->m_szErrMsg, "当前线程有重复进入加写锁情况");				
		return -1;
	}
		//加写锁成功后，关闭所有读操作，同时搜索自己节点的个数
#ifndef LINUXCODE
	::EnterCriticalSection(&m_stReaderSection);
#else
	pthread_mutex_lock(&m_stReaderSection);
#endif

		m_blAllowReadFlag = false;
		ulCount = 0;
		//查询所有节点
		Mylist_for_each(pNode, &m_stReaderHeadNode)
		{
			pstLockNode = Mylist_entry(pNode, CLocalRdWrtLockSession::TLockNode, m_stNode);
			if(ulThreadID == pstLockNode->m_ulThreadID)
			{
				ulCount += pstLockNode->m_ushLockCount;
			}
		}
#ifndef LINUXCODE
	::LeaveCriticalSection(&m_stReaderSection);
#else
	pthread_mutex_unlock(&m_stReaderSection);
#endif

		if(0 < ulCount)
		{
			sprintf(in_poRdWrtSession->m_szErrMsg, "当前线程已加过读锁再写锁，可能会死锁");				
			lRet = -3;
			goto WriteLockErr_Label;
		}
		do {
			if(m_ulReaderCount <= ulCount)
				break;
			time(&lNowTime);
			lWaitTime = lNowTime - lStartTime;
			if(lWaitTime >= in_lTimeOut)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "等待写锁超时");				
				lRet = -1;
				goto WriteLockErr_Label;
			}
			if(! m_oWrtWaitEvent.Wait((in_lTimeOut - lWaitTime) * 1000))
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "等待写锁出现错误");				
				lRet = -2;
				goto WriteLockErr_Label;
			}
			
		} while(1);

	in_poRdWrtSession->m_blWriteLockFlag = true;
	m_ulWrtThreadID = ulThreadID;	//当前写锁线程ID号

	return 0;
WriteLockErr_Label:
#ifndef LINUXCODE
	::EnterCriticalSection(&m_stReaderSection);
#else
	pthread_mutex_lock(&m_stReaderSection);	
#endif

	m_blAllowReadFlag = true;

#ifndef LINUXCODE
	::LeaveCriticalSection(&m_stReaderSection);
	::LeaveCriticalSection(&m_stWriterSection);
#else
	pthread_mutex_unlock(&m_stReaderSection);	
	pthread_mutex_unlock(&m_stWriterSection);	
#endif

	return lRet;
	
}
/*******************************************************
       	函 数 名: rl_ReadLock
        功能描述: 读锁加锁
       	参    数: in_poRdWrtSession	输入--读写锁操作类
       			in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       	返回说明: 0成功	 -1失败
********************************************************/
long CRdWrtLockResource::rl_ReadLock(CLocalRdWrtLockSession * in_poRdWrtSession, long in_lTimeOut)
{

	long lNowTime,lStartTime;
	long lWaitTime;
	bool	blResetFlag;
	bool	blFirstFlag;
	time(&lStartTime);
	in_poRdWrtSession->m_stReadNode.m_ulThreadID = GetCurrentThreadId();
	blFirstFlag=true;

	do {
		blResetFlag = false;
#ifndef LINUXCODE
			::EnterCriticalSection(&m_stReaderSection);
#else
			pthread_mutex_lock(&m_stReaderSection);
#endif
			if(!m_blAllowReadFlag)
			{
				blResetFlag = true;
				m_oRdWaitEvent.Active();
			}
			//允许读锁或自己已经加过写锁，则表示可以加读锁
			if(m_blAllowReadFlag || in_poRdWrtSession->m_stReadNode.m_ulThreadID == m_ulWrtThreadID)
			{
				//自己清除过标志，则必须设置回去
				if( blResetFlag && m_blAllowReadFlag)
					m_oRdWaitEvent.Active();
				break;
			}
			//查询自己线程是否在已经存在于读者链中，如果是，有读锁嵌套，表示与写锁碰锁，会造成写锁超时错误
			if(blFirstFlag)
			{
				TMyListNode * pNode;
				CLocalRdWrtLockSession::TLockNode	* pstLockNode;
				//查询所有节点
				Mylist_for_each(pNode, &m_stReaderHeadNode)
				{
					pstLockNode = Mylist_entry(pNode, CLocalRdWrtLockSession::TLockNode, m_stNode);
					if(in_poRdWrtSession->m_stReadNode.m_ulThreadID == pstLockNode->m_ulThreadID)
					{
						//已经加过读锁，则加读锁直接成功
						break;						
					}
				}
			}
#ifndef LINUXCODE
			::LeaveCriticalSection(&m_stReaderSection);
#else
			pthread_mutex_unlock(&m_stReaderSection);
#endif
			blFirstFlag = false;
			time(&lNowTime);
			lWaitTime = lNowTime - lStartTime;
			if(lWaitTime >= in_lTimeOut)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "等待读锁超时");				
				return -1;
			}
			if(! m_oRdWaitEvent.Wait((in_lTimeOut - lWaitTime) * 1000))
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "等待读锁出现错误");				
				return  -2;			
			}
			
		} while(1);
		//如果没有加过读锁，则需要将自己的链加入
		if(!in_poRdWrtSession->m_blReadLockFlag)
			rv_AddListNode(&m_stReaderHeadNode, &(in_poRdWrtSession->m_stReadNode.m_stNode));
		in_poRdWrtSession->m_stReadNode.m_ushLockCount++;
		m_ulReaderCount ++;
#ifndef LINUXCODE
		::LeaveCriticalSection(&m_stReaderSection);
#else
		pthread_mutex_unlock(&m_stReaderSection);
#endif

	in_poRdWrtSession->m_blReadLockFlag = true;
	return 0;
	
}
/*******************************************************
       	函 数 名: rl_UnlockWrtKeepRd
       	功能描述: 解写锁，保留读锁
       	参    数: in_poRdWrtSession	输入--读写锁操作类
       	返回说明: 0成功	 -1失败
********************************************************/
long CRdWrtLockResource::rl_UnlockWrtKeepRd(CLocalRdWrtLockSession * in_poRdWrtSession)
{
	long lRet;
	//如果没有加过读锁，则需要将自己加入
	if(!in_poRdWrtSession->m_blReadLockFlag)
	{
		//持有写时，直接增加读即可
		if(in_poRdWrtSession->m_blWriteLockFlag)
		{
				memset(&(in_poRdWrtSession->m_stReadNode), 0, sizeof(in_poRdWrtSession->m_stReadNode));
				in_poRdWrtSession->m_stReadNode.m_ulThreadID = GetCurrentThreadId();

#ifndef LINUXCODE
				::EnterCriticalSection(&m_stReaderSection);
#else
				pthread_mutex_lock(&m_stReaderSection);
#endif

				rv_AddListNode(&m_stReaderHeadNode, &(in_poRdWrtSession->m_stReadNode.m_stNode));
				in_poRdWrtSession->m_stReadNode.m_ushLockCount++;
				m_ulReaderCount ++;
#ifndef LINUXCODE
				::LeaveCriticalSection(&m_stReaderSection);
#else
				pthread_mutex_unlock(&m_stReaderSection);
#endif
				in_poRdWrtSession->m_blReadLockFlag = true;
		}
		
	}
	lRet = rl_UnlockWrt(in_poRdWrtSession);
	if( 0 > lRet)
		return lRet;
	if(!in_poRdWrtSession->m_blReadLockFlag)
	{
		sprintf(in_poRdWrtSession->m_szErrMsg, "保持读锁错误，因为根本没有加过锁");
		return -1;

	}
	return 0;

}
/*******************************************************
	  函 数 名: rl_UnlockRd
	   	功能描述: 解读锁
	   	参    数: in_poRdWrtSession	输入--读写锁操作类
	    返回说明: 0成功	 -1失败
********************************************************/
long CRdWrtLockResource::rl_UnlockRd(CLocalRdWrtLockSession * in_poRdWrtSession )
{
	//如果没有加过读锁，则不处理解锁过程
	if(in_poRdWrtSession->m_blReadLockFlag)
	{
#ifndef LINUXCODE
			::EnterCriticalSection(&m_stReaderSection);
#else
			pthread_mutex_lock(&m_stReaderSection);

#endif
			if(in_poRdWrtSession->m_stReadNode.m_ushLockCount > m_ulReaderCount)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "当前读写操作类是加读锁%d个，但锁资源中加读锁只有%d，读锁被复位",
					in_poRdWrtSession->m_stReadNode.m_ushLockCount, m_ulReaderCount);
				rv_DelListNode(&(in_poRdWrtSession->m_stReadNode.m_stNode));
				rv_DelListNode(&m_stReaderHeadNode);

				m_ulReaderCount = 0;
#ifndef LINUXCODE
				::LeaveCriticalSection(&m_stReaderSection);
#else
				pthread_mutex_unlock(&m_stReaderSection);

#endif
				return -1;
			}
			rv_DelListNode(&(in_poRdWrtSession->m_stReadNode.m_stNode));
			m_ulReaderCount -= in_poRdWrtSession->m_stReadNode.m_ushLockCount;

#ifndef LINUXCODE
			::LeaveCriticalSection(&m_stReaderSection);
#else
			pthread_mutex_unlock(&m_stReaderSection);			
#endif

			in_poRdWrtSession->m_blReadLockFlag = false;
			//激活等待者
			m_oRdWaitEvent.Active();
			m_oWrtWaitEvent.Active();
			

	}
	return 0;

}
/*******************************************************
	  函 数 名: rl_UnlockWrt
	   	功能描述: 解写锁
	   	参    数: in_poRdWrtSession	输入--读写锁操作类
	    返回说明: 0成功	 -1失败
********************************************************/
long CRdWrtLockResource::rl_UnlockWrt(CLocalRdWrtLockSession * in_poRdWrtSession )
{
	//如果没有加过写锁，则不处理解锁过程
	if(in_poRdWrtSession->m_blWriteLockFlag)
	{
			if(m_blAllowReadFlag)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "当前读写操作类是加写锁标志，但锁资源没有加写锁");
				return -1;
			}
			m_blAllowReadFlag = true;
			m_ulWrtThreadID = 0;	//当前写锁线程ID号
#ifndef LINUXCODE
			::LeaveCriticalSection(&m_stWriterSection);
#else
			pthread_mutex_unlock(&m_stWriterSection);
#endif
			in_poRdWrtSession->m_blWriteLockFlag = false;
			//激活等待者
			m_oRdWaitEvent.Active();
			m_oWrtWaitEvent.Active();
			

		
	}
	return 0;
}


CRdWrtLockResource::CRdWrtLockResource()
{
#ifndef LINUXCODE
		::InitializeCriticalSection(&m_stReaderSection);		//读者使用的临界区保护
		::InitializeCriticalSection(&m_stWriterSection);			//写使用的临界区保护	
#else
		/*
		 *	GUOGUO 2009-08-11 设置临界区锁的属性为可递归锁,支持同一线程连续调用lock
		 */
		//	ADD
		pthread_mutexattr_t attr;

		/*
		 *	2010-11-19在x86_64机器上运行发现的一个程序的BUG
		 *		当时的情况是,无法递归锁. 怎么也没想到是因为这个构造函数里面的
		 *		pthread_mutex_init 出现95错误, 导致锁不是递归的.
		 *		而导致pthread_mutex_init失败的原因又是因为attr没有使用pthread_mutexattr_init初始化.
		 *		似乎在 32位的RedHat AS4 Update6的机器上,正常运行..
		 *		总结了下原因,是因为不同的系统,对新的加载的内存页没有 填充 zero 所以,无论怎么样,
		 *		我们不能依赖系统为我们做工作,而应该我们主动的调用pthread_mutexattr_init做完这个初始化工作
		 *		这样才是最安全的
		 *
		 *		https://bugzilla.redhat.com/show_bug.cgi?id=204863
		 *															---		GUOGUO 2010-11-10
		 */
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		//	ADD END
		pthread_mutex_init(&m_stReaderSection,&attr);
		//	ADD
		pthread_mutexattr_destroy(&attr);

		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		//	ADD END
		pthread_mutex_init(&m_stWriterSection,&attr);
		//	ADD
		pthread_mutexattr_destroy(&attr);


#endif
		memset(&m_stReaderHeadNode, 0, sizeof(m_stReaderHeadNode)); //读者头节点
		m_ulReaderCount = 0;	//读者引用计数
		m_blAllowReadFlag = true;	//允许读标志
		m_ulWrtThreadID = 0;	//当前写锁线程ID号
	
}
CRdWrtLockResource::~CRdWrtLockResource()
{
	#ifndef LINUXCODE

		::DeleteCriticalSection(&m_stReaderSection);
		::DeleteCriticalSection(&m_stWriterSection);
		
	#else
		pthread_mutex_destroy(&m_stReaderSection);
		pthread_mutex_destroy(&m_stWriterSection);

		
	#endif
		m_ulReaderCount = 0;
		memset(&m_stReaderHeadNode, 0, sizeof(m_stReaderHeadNode)); //读者头节点

}


CMyWaitEvent::CMyWaitEvent(void)
{
	m_blRunFlag = true;
	#ifndef LINUXCODE
		
		if ( (m_hRecordData = ::CreateEvent(NULL,FALSE,FALSE,NULL)) == NULL )
		{
			assert(0);
			throw exception("<MWaitEvent>创建等待事件发生错误");
		}

	#else
		m_nCount	 = 0;
		m_IsStartWait = 0;
		pthread_mutex_init(&m_hMutex, NULL);
		pthread_cond_init(&m_hRecordData,NULL);

	#endif
}
//..............................................................................................................................
CMyWaitEvent::~CMyWaitEvent()
{
	m_blRunFlag = false;
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
	#ifndef LINUXCODE
	
		if ( m_hRecordData != NULL )
		{
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}

	#else

		pthread_mutex_destroy(&m_hMutex);
		pthread_cond_destroy(&m_hRecordData);
		m_IsStartWait = 0;
		m_nCount = 0;

	#endif
}
//..............................................................................................................................
void CMyWaitEvent::Active(void)
{
	if(!m_blRunFlag	)
		return;
	#ifndef LINUXCODE

		::SetEvent(m_hRecordData);
		
	#else

		pthread_mutex_lock(&m_hMutex);
		if(m_nCount == 0 && m_IsStartWait > 0)
		{
			//	进行判断,以便安全进行唤醒,否则可能导致唤醒的错位
			pthread_cond_signal(&m_hRecordData);
		}
		m_nCount++;
		pthread_mutex_unlock(&m_hMutex);

	#endif
}
//..............................................................................................................................
bool CMyWaitEvent::Wait(unsigned long lMSec)
{
	if(!m_blRunFlag	)
		return false;
	#ifndef LINUXCODE
		register int				errorcode;
		errorcode = ::WaitForSingleObject(m_hRecordData,lMSec);
		if ( errorcode == WAIT_FAILED )
		{
			return false;
		}
	#else
		int							rc, future;
		struct timespec				abstime;	//GUOGUO 20090723
		struct timeb				tp;

		ftime(&tp);
		/*	Note:
		 *	1s=1000000000纳秒
		 *	1s=1000000	 微秒
		 *	1s=1000		 毫秒
		 */
		future = tp.millitm  + lMSec % 1000;	//	未来lMsec的毫秒值
		future = future * 1000000;				//	换算成纳秒

		if(future >= 1000000000)
		{
			abstime.tv_nsec = future % 1000000000;
			abstime.tv_sec = tp.time + lMSec / 1000 + future / 1000000000;
		}
		else
		{
			abstime.tv_nsec = future;
			abstime.tv_sec = tp.time + lMSec / 1000;
		}


		//sttemp = lMSec;
		pthread_mutex_lock(&m_hMutex);
		m_IsStartWait = 1;
		while(m_nCount == 0 && m_blRunFlag)
		{
			//	得到锁后,必须再次判断m_nCount
			//	因为 2种可能.
			//	1:timewait可能是被UNIX 的signal唤醒
			//	2:pthread_cond_signal不唤醒它
			rc = pthread_cond_timedwait(&m_hRecordData,&m_hMutex,&abstime);
			if(rc != 0)
			{
				if(rc == ETIMEDOUT)
				{

					//	超时错误
					pthread_mutex_unlock(&m_hMutex);


					return false;
				}
				else if(rc == EINTR)
				{
					//	发生了signal 中断信号
					//	其实应该重新设置sttemp的,算了,简单起见.
					continue;
				}
				else
				{
					//	其它错误
					//pthread_mutex_unlock(&m_hMutex);
					//return;
					continue;
				}
			}
			else
			{
				//	被唤醒了
				continue;	//	非常不理解pthread_cond_timewait==0的时候,
							//	似乎好象并没有被pthread_cond_signal唤醒,也不是被中断信号唤醒
							//	不太清楚还有什么情况会导致这样的事情发生
							//	为了安全起见我准备继续测试条件变量m_nCount的值.
			}
		}
		if(m_nCount <= 0)
		{
			//	绝对不可能发生的错误,前面已经尽了最大的努力去控制m_nCount的值
			//	如果还能走到这里,那么只能是去死循环了.系统的问题已经相当的严重
			//	也许可以考虑修正m_nCount = 0,但是我不建议这样做,
			//	已经是有问题了,为了避免问题进一步扩大,我认为它应该去死循环.然后的等待coder去解决这个问题
			//
			//						--	2010 GUOGUO
#ifdef _DEBUG
			while(1)
			{
				printf("rc=%d, m_nCount=%d, m_IsStartWait=%d, errno = %d\n", rc, m_nCount, m_IsStartWait, errno);
				MThread::Sleep(1);
			}
#else
			usleep(1000);
			char	szException[256];
			memset(szException, 0, sizeof(szException));
			FILE	*fp;
			fp = fopen("/tmp/MMutex.log", "a+");
			if(fp)
			{
				fprintf(fp, "time=%d, rc=%d, m_nCount=%d, m_IsStartWait=%d, errno = %d", time(NULL), rc, m_nCount, m_IsStartWait, errno);
				fclose(fp);
			}
#endif
			return false;
		}
		m_nCount--;
		pthread_mutex_unlock(&m_hMutex);


#endif
	return true;
}
