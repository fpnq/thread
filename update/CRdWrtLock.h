/***********************************************************************
 * 文件名称：CRdWrtLock.h
 * 功能描述：实现读写锁功能
 * 开发历史：liuqy 20110111 cd
 * 读写锁实现如下：
 1、资源：
	1) 读写标志：当为0表示为写，当为1表示为读
	2) 读者引用计数：初始为0，进入一个读者加一，退出一个读者减一
	3) 读者链头：使用于保存读者线程ID的一个链
	4) 写互斥锁资源：写操作时引用计算需要加锁
	5) 读临界区资源：修改读写标志及读者引用计数时使用
 2、读加锁过程：
	1)	进入读临界区资源
	2)	检查读写标志是否为1，不为1则下一步，为1或自己已经加写锁，则执行第5步
	3)	查询自己线程是否在已经存在于读者链中，如果是，则退出临界区,报错误退出（与写锁死锁了）
	4)	退出临界区，循环等待读写标志是否为1，超时报错返回失败退出；为1则跳到第一步
	5)	将读者引用计数加1
	6)	将自己的链中增加自己自线程ID，并将链增加到读者链头上
	7)	退出临界区
	8)	返回加锁成功
 3、读解锁过程：
	1)	进入读临界区资源
	2)	将自己的链从读者链上取下
	3)	读者引用计数大于0，则减一
	4)	退出临界区
 4、写加锁过程：
	1)	1)	已经被当前线程加过写锁，则直接返回成功; 写互锁资源加锁
	2)	检查读写标志是否为0，如果是，则跳到第12步，否则下一步
	3)	进入读临界区资源
	4)	将读写标志修改为0
	5)	从读者链中搜索为自己线程ID的个数
	6)	退出临界区
	7)	循环等待读者引用计数小于等于自己线程ID个数，当等待超时则跳到第7步；当小于等于自己线程ID个数时进入下一步
	8)	写线程ID设置为当前线程ID，返回成功
	9)	进入读临界区资源
	10)	将读写标志修改为1
	11)	退出临界区
	12)	写互锁资源解锁
	13)	返回失败
 5、写解锁过程：
	1)	将读写标志修改为1,写线程ID设置为0
	2)	写互锁资源解锁
 *********************************************************************/
 
#if !defined(CRWSESSION_H__7DA811_B8D7_4132_93C3_164ED646B5A2__INCLUDED_)
#define CRWSESSION_H__7DA811_B8D7_4132_93C3_164ED646B5A2__INCLUDED_

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "MDateTime.h"


#define My_ReadLock(oLocalOperLock, TimeOut) \
(oLocalOperLock.rl_ReadLock( TimeOut))

#define My_WriteLock(oLocalOperLock, TimeOut) \
(oLocalOperLock.rl_WriteLock(TimeOut))

//节点操作
typedef struct __TMYLISTNODE
{
	struct	__TMYLISTNODE * m_pstPrevious;	//前一个 当为头时，该值为无效
	struct	__TMYLISTNODE * m_pstNext;	//下一个
}TMyListNode;

//初始化一个节点
void rv_InitListNode(TMyListNode * in_pstNode);
//向头上增加一个节点
void rv_AddListNode(TMyListNode * in_pstHead, TMyListNode * in_pstAddNode);
//从节点中删除
void rv_DelListNode(TMyListNode * in_pstDelNode);
/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define Mylist_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define Mylist_for_each(posPtr, head) \
	for (posPtr = (head)->m_pstNext; NULL != posPtr; \
        	posPtr = posPtr->m_pstNext)

class CRdWrtLockResource;

//读写锁操作类，即对读写锁资源进行读写加解锁操作
class CLocalRdWrtLockSession
{
	friend class CRdWrtLockResource;
public:
	typedef enum eLOCKSEMFLAG
	{
		None = 0,	/*不进行加锁*/
		Read,	/*读方式，则只加锁一个*/
		Write,  /*写方式*/
	}ELockFlag;
		
	/*******************************************************
       		函 数 名: rl_WriteLock
       		功能描述: 写锁加锁
       		参    数: in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       		返回说明: 0成功 	 -1失败
	********************************************************/
	long rl_WriteLock(long in_lTimeOut = 30);
	long rl_WriteLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo);

	/*******************************************************
       		函 数 名: rl_ReadLock
            功能描述: 读锁加锁,注意如果以前有过写锁没有解锁，则系统直接返回成功
       		参    数: in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       		返回说明: 0成功	  -1失败
	********************************************************/
	long rl_ReadLock(long in_lTimeOut = 30);
	long rl_ReadLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo);
	/*******************************************************
       		函 数 名: rl_UnlockWrtKeepRd
       		功能描述: 解写锁，保留读锁
       		参    数: 无
       		返回说明: 0成功	 -1失败
	********************************************************/
	long rl_UnlockWrtKeepRd();
	/*******************************************************
	      函 数 名: rl_Unlock
	   		功能描述: 解所有的锁
	   		参    数: 无
	       	返回说明: 0成功	 -1失败
	********************************************************/
	long rl_Unlock( );

	/*******************************************************
       		函 数 名: re_GetLockState
       		功能描述: 取当前加锁情况
       		参    数: 无
       		返回说明: None未加锁，Read已经加过读锁，Write已经加过写锁
	********************************************************/
	ELockFlag	re_GetLockState();
	
	CLocalRdWrtLockSession(CRdWrtLockResource * in_poRdWrtLockRsc);
	virtual ~CLocalRdWrtLockSession();

	CLocalRdWrtLockSession();
	//设置读写锁资源
	void SetLockRsc(CRdWrtLockResource & in_oRdWrtLockRsc);
	
	//取错误信息
	const char * rpsz_GetErrMsg() { return m_szErrMsg;	};

protected:
	typedef struct _TLOCKNODE{
		TMyListNode	m_stNode;		//链节点
		unsigned long	m_ulThreadID;	//当前线ID
		unsigned short	m_ushLockCount;	//加锁计数
		unsigned short	m_ushFileLineNo;
		char			m_szFileName[32];
	}TLockNode;
	
	TLockNode	m_stReadNode;
	bool	m_blReadLockFlag;	//读加锁标记
	bool	m_blWriteLockFlag;	//写加锁标记
	CRdWrtLockResource * m_poRdWrtLockRsc;	//读写锁资源
	char	m_szErrMsg[512];	//错误信息

};
class CMyWaitEvent
{
protected:
	volatile	bool				m_blRunFlag;
#ifndef LINUXCODE
	HANDLE									m_hRecordData;
#else
	int										m_IsStartWait;
	volatile	 int										m_nCount;	//	安全计数,避免thread_signal空唤醒,以及保持thread_wait被信号唤醒的处理
	pthread_cond_t							m_hRecordData;
	pthread_mutex_t							m_hMutex;
#endif
public:
	CMyWaitEvent(void);
	virtual ~CMyWaitEvent();

public:
	//激活事件
	void Active(void);
	//等待事件
	bool Wait(unsigned long lMSec = 0xFFFFFFFF);
};


//读写锁资源
class CRdWrtLockResource
{
	friend class CLocalRdWrtLockSession;
protected:
	unsigned long	m_ulWrtThreadID;	//当前写线程ID
	#ifndef LINUXCODE
		CRITICAL_SECTION			m_stReaderSection;			//读者使用的临界区保护
		CRITICAL_SECTION			m_stWriterSection;			//写使用的临界区保护		

	#else
		pthread_mutex_t				m_stReaderSection;
		pthread_mutex_t				m_stWriterSection;			//写使用的临界区保护		


	#endif
		TMyListNode			m_stReaderHeadNode;		//读者头节点
		unsigned	long		m_ulReaderCount;	//读者引用计数
		bool				m_blAllowReadFlag;	//允许读标志
		CMyWaitEvent		m_oRdWaitEvent;
		CMyWaitEvent		m_oWrtWaitEvent;
	
	/*******************************************************
       		函 数 名: rl_WriteLock
       		功能描述: 写锁加锁
       		参    数: in_poRdWrtSession	输入--读写锁操作类
					in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       		返回说明: 0成功 	 -1失败
	********************************************************/
	long rl_WriteLock(CLocalRdWrtLockSession * in_poRdWrtSession,  long in_lTimeOut = 30);
	/*******************************************************
       		函 数 名: rl_ReadLock
            功能描述: 读锁加锁
       		参    数: in_poRdWrtSession	输入--读写锁操作类
       				in_lTimeOut		--输入-超时时间单位为秒，小于等于0表示一直等待
       		返回说明: 0成功	  -1失败
	********************************************************/
	long rl_ReadLock(CLocalRdWrtLockSession * in_poRdWrtSession, long in_lTimeOut = 30);
	/*******************************************************
       		函 数 名: rl_UnlockWrtKeepRd
       		功能描述: 解写锁，保留读锁
       		参    数: in_poRdWrtSession	输入--读写锁操作类
       		返回说明: 0成功	 -1失败
	********************************************************/
	long rl_UnlockWrtKeepRd(CLocalRdWrtLockSession * in_poRdWrtSession);
	/*******************************************************
	      函 数 名: rl_UnlockRd
	   		功能描述: 解读锁
	   		参    数: in_poRdWrtSession	输入--读写锁操作类
	       	返回说明: 0成功	 -1失败
	********************************************************/
	long rl_UnlockRd(CLocalRdWrtLockSession * in_poRdWrtSession );
	/*******************************************************
	      函 数 名: rl_UnlockWrt
	   		功能描述: 解写锁
	   		参    数: in_poRdWrtSession	输入--读写锁操作类
	       	返回说明: 0成功	 -1失败
	********************************************************/
	long rl_UnlockWrt(CLocalRdWrtLockSession * in_poRdWrtSession );
	
public:

	CRdWrtLockResource();
	virtual ~CRdWrtLockResource();

	
};

#endif // !defined(CRWSESSION_H__7DA811_B8D7_4132_93C3_164ED646B5A2__INCLUDED_)
