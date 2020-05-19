//------------------------------------------------------------------------------------------------------------------------------
#include "MMutex.h"
#ifdef LINUXCODE
#include <pthread.h>
#endif
#include "../BaseUnit/MThread.h"


//------------------------------------------------------------------------------------------------------------------------------
MCheckTimeOutObjectList				*	MCheckTimeOutObjectList::s_mFirstObject = NULL;
//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
#ifndef LINUXCODE
CRITICAL_SECTION				MCheckTimeOutObjectList::s_oMySection;
#else

pthread_mutex_t				MCheckTimeOutObjectList::s_oMySection;

#ifndef __GCC_VER__
#define __GCC_VER__ (__GNUC__ * 10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__ )
#endif


#endif
//end add
//add by liuqy 20130609 for 将二进制显示转换
/******************************************************************************
	函 数 名：LIB_BYTE2ASC				函数编号：
	功能描述：将指定长度的无符号字符串转换为0-F之间的ASCII字符
	入口参数：
		in_lLen				--待处理的字符串长度
		in_pus				--待转换的无符号串
		out_psz				--进行转换处理后的处理结构串
	返回说明：无
	引用变量：
	开发历史：
 ******************************************************************************/
void	LIB_BYTE2ASC(long in_lLen, const char* in_pus, char* out_psz)
{
	register int	i, k = 0;
	register unsigned char	ch;

	if(!in_pus)
	{
		if(out_psz)
			out_psz[0] = 0;
		return;
	}
	if(in_lLen == 0) in_lLen = strlen(in_pus);
	for(i = 0; i < in_lLen; i++)
	{
		ch = in_pus[i] & 240;			//	240 = 0xF0
		ch = ch >> 4;
		if(ch <= 9)						//	 9 = 0x09
			out_psz[k] = 48 | ch;		//	48 = 0x30
		else
			out_psz[k] = 64 | (ch - 9);	//	64 = 0x40, 9 = 0x09
		k++;

		ch = in_pus[i] & 15;			//	15 = 0x0F
		if(ch <= 9)						//	 9 = 0x09
			out_psz[k] = 48 | ch;		//	48 = 0x30
		else
			out_psz[k] = 64 | (ch - 9);	//	64 = 0x40, 9 = 0x09
		k++;
	}
	out_psz[k] = 0;
}
/******************************************************************************
	函 数 名：LIB_DisplayByte				函数编号：
	功能描述：显示可见字符
	入口参数：
		in_lLen				--待处理的字符串长度
		in_pus				--待转换的无符号串
		out_psz				--进行转换处理后的处理结构串
	返回说明：无
	引用变量：
	开发历史：
 ******************************************************************************/
void	LIB_DisplayByte(long in_lLen, const char* in_pus, char* out_psz)
{
	register int	i, k = 0;

	if(!in_pus)
	{
		if(out_psz)
			out_psz[0] = 0;
		return;
	}
	if(in_lLen == 0) in_lLen = strlen(in_pus);
	for(i = 0; i < in_lLen; i++)
	{
		if(0 > in_pus[i])
		{
			if(0 > in_pus[i+1])
			{
				out_psz[k++] = in_pus[i++];
				out_psz[k++] = in_pus[i];
				continue;
			}
		}
		if( 0x1f > in_pus[i] || 0x7f == in_pus[i])
		{
			out_psz[k++] = '.';
			
		}
		else
			out_psz[k++] = in_pus[i];
	}
	out_psz[k] = 0;
}
//..............................................................................................................................
MCheckTimeOutObjectList::MCheckTimeOutObjectList(const char * strName)
{
	//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	if(NULL == s_mFirstObject)
	{
#if defined(WIN32) || defined(MS_WINDOWS)
		Sleep(0);
		Sleep(0);
		Sleep(0);
		Sleep(0);
		Sleep(0);
		Sleep(0);
		Sleep(0);
		Sleep(0);
#else
		sched_yield();
		sched_yield();
		sched_yield();
		sched_yield();
		sched_yield();
		sched_yield();
		sched_yield();
		sched_yield();
#endif
		#ifndef LINUXCODE
			::InitializeCriticalSection(&s_oMySection);
		#else	
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

			pthread_mutex_init(&s_oMySection,&attr);

			pthread_mutexattr_destroy(&attr);
		#endif
	}
	#ifndef LINUXCODE
		::EnterCriticalSection(&s_oMySection);
	#else
		pthread_mutex_lock(&s_oMySection);
	#endif
	//end add
	//将对象加入到链表
	if ( s_mFirstObject != NULL )
	{
		s_mFirstObject->m_mPreObject = this;
	}
	
	m_mNextObject = s_mFirstObject;
	m_mPreObject = NULL;
	s_mFirstObject = this;
	//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	#ifndef LINUXCODE
		::LeaveCriticalSection(&s_oMySection);
	#else
		pthread_mutex_unlock(&s_oMySection);
	#endif
	//end add
	
	//初始化其他状态
	m_strClassName = strName;
	m_strObjectName[0] = 0;
	m_lAddtionData = 0;
	m_bLocked = false;
	//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
	m_pstSrcFileLine = NULL;
	//add by liuqy 20130608 for 数据有效性检查定义位置，即类成员数据定义必须在开始与结束之间
	m_ushDataFlagBegin = CLSDATAFLAG_BEGIN;
	m_ushDataFlagEnd = CLSDATAFLAG_END ;

}
//..............................................................................................................................
MCheckTimeOutObjectList::~MCheckTimeOutObjectList()
{
	//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	#ifndef LINUXCODE
		::EnterCriticalSection(&s_oMySection);
	#else
		pthread_mutex_lock(&s_oMySection);
	#endif
	//将对象从链表中删除
	if ( m_mPreObject != NULL )
	{
		m_mPreObject->m_mNextObject = m_mNextObject;
	}
	
	if ( m_mNextObject != NULL )
	{
		m_mNextObject->m_mPreObject = m_mPreObject;
	}
	if ( s_mFirstObject == this )
	{
		s_mFirstObject = m_mNextObject;
	}
	//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	#ifndef LINUXCODE
		::LeaveCriticalSection(&s_oMySection);
	#else
		pthread_mutex_unlock(&s_oMySection);
	#endif
	if(NULL == s_mFirstObject )
	{
		#ifndef LINUXCODE
			::DeleteCriticalSection(&s_oMySection);
		#else
			pthread_mutex_destroy(&s_oMySection);
		#endif
	}
	//end add
	//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
	if(NULL != m_pstSrcFileLine )
	{
		//delete m_pstSrcFileLine ;
		free( m_pstSrcFileLine );
	}
	m_pstSrcFileLine = NULL;

}
//add  by liuqy 20130613 for 取设置的线程源文件名及行号
const char * MCheckTimeOutObjectList::GetSrcFileName()
{
	if(NULL != m_pstSrcFileLine)
	{
		return m_pstSrcFileLine->szFileName;
	}
	return "";
}
unsigned short MCheckTimeOutObjectList::GetSrcFileLine()
{
	if(NULL != m_pstSrcFileLine)
	{
		return m_pstSrcFileLine->ushFileLine;
	}
	return 0;

}

//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
void MCheckTimeOutObjectList::SetSrcFileLine(const char * pszFileName, unsigned short in_ushFileLine)
{
	int lLen;
#ifndef LINUXCODE

	register i;
#else
	int i;
#endif

	if(NULL == pszFileName)
		return ;
	lLen = strlen(pszFileName);
	if(3 > lLen)
		return ;
	for(i=lLen-1; 0 < i; i--)
	{
		if('/' == pszFileName[i] || '\\' == pszFileName[i])
			break;
	}
	if('/' == pszFileName[i] || '\\' == pszFileName[i])
		i++;
	if(NULL == m_pstSrcFileLine)
	{
		//m_pstSrcFileLine = new TSrcFileLine;
		//modify by liuqy 20150630 需要将new 更换为malloc 
		m_pstSrcFileLine = (TSrcFileLine*)malloc(sizeof(TSrcFileLine));
	}
	if(NULL == m_pstSrcFileLine)
		return;
	lLen = lLen - i;
	if(lLen >= sizeof(m_pstSrcFileLine->szFileName))
		lLen = sizeof(m_pstSrcFileLine->szFileName) - 1;
	memcpy(m_pstSrcFileLine->szFileName, &pszFileName[i], lLen);
	m_pstSrcFileLine->szFileName[lLen] = 0;
	m_pstSrcFileLine->ushFileLine = in_ushFileLine;
}
//end 
//..............................................................................................................................
void MCheckTimeOutObjectList::inner_setactive(void)
{
	m_mRefreshTime.SetCurTickCount();
	m_bLocked = true;
}
//..............................................................................................................................
void MCheckTimeOutObjectList::inner_stopactive(void)
{
	m_bLocked = false;
}
//..............................................................................................................................
void MCheckTimeOutObjectList::SetSpObejctName(const char * strObjectName)
{
	my_strncpy(m_strObjectName,strObjectName,32);
}
//..............................................................................................................................

const char * MCheckTimeOutObjectList::GetSpObjectName()
{
	if ( NULL != m_strObjectName )
	{
		return m_strObjectName;
	}

	return "";
}
//..............................................................................................................................

unsigned long MCheckTimeOutObjectList::GetThreadID()
{
	return m_ulThreadID;
}

//..............................................................................................................................
void MCheckTimeOutObjectList::SetObjectAddtionData(unsigned long lIn)
{
	m_lAddtionData = lIn;
}
unsigned long MCheckTimeOutObjectList::GetObjectAddtionData()
{
	return m_lAddtionData;
}
//..............................................................................................................................
void MCheckTimeOutObjectList::Refresh(void)
{
	m_mRefreshTime.SetCurTickCount();
}
//add by liuqy 20131211 for 超时处理
bool MCheckTimeOutObjectList::rbl_ChkSpObjTimeout(unsigned long lDelayTime,tagWriteReportFunction * in_fpFunct)
{
	MCheckTimeOutObjectList				*	lpobjectptr;
	char									tempbuf[256];
	register unsigned long					errorcode;
	bool	blRet = false;
	
	lpobjectptr = s_mFirstObject;
	while ( lpobjectptr != NULL )
	{
		if ( lpobjectptr->m_bLocked == true )
		{
			//add by liuqy 20130608 for 数据有效性检查定义位置，即类成员数据定义必须在开始与结束之间
			if(CLSDATAFLAG_BEGIN != lpobjectptr->m_ushDataFlagBegin ||	CLSDATAFLAG_END !=	lpobjectptr->m_ushDataFlagEnd)
			{
				if(NULL != in_fpFunct)
				{

					my_snprintf(tempbuf,	256,	"对象<%s>%s内存被破坏(0x%x 0x%x)]",
						lpobjectptr->m_strClassName, lpobjectptr->m_ushDataFlagBegin, lpobjectptr->m_ushDataFlagEnd);
					
				#ifndef	_LINUXTRYOFF
					try
					{
				#endif
						in_fpFunct(tempbuf);
				#ifndef	_LINUXTRYOFF
					}
					catch(...)
					{
						assert(0);
					}
				#endif
				}
				
			}
			
			errorcode = lpobjectptr->m_mRefreshTime.GetDuration();
			if ( errorcode >= lDelayTime )
			{
				blRet = true;
				if(NULL == in_fpFunct)
				{
					break;
				}
				else
				{
					//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
					if(NULL != lpobjectptr->m_pstSrcFileLine)
						my_snprintf(tempbuf,256,"对象<%s:线%d>%s停止响应%d秒[附加数据%d](%s-%u)",
						lpobjectptr->m_strClassName, lpobjectptr->m_ulThreadID,lpobjectptr->m_strObjectName,errorcode / 1000,lpobjectptr->m_lAddtionData,
						lpobjectptr->m_pstSrcFileLine->szFileName, lpobjectptr->m_pstSrcFileLine->ushFileLine);
					else
						my_snprintf(tempbuf,256,"对象<%s:线%d>%s停止响应%d秒[附加数据%d]",lpobjectptr->m_strClassName, lpobjectptr->m_ulThreadID,lpobjectptr->m_strObjectName,errorcode / 1000,lpobjectptr->m_lAddtionData);
					
				#ifndef	_LINUXTRYOFF
					try
					{
				#endif
						in_fpFunct(tempbuf);
				#ifndef	_LINUXTRYOFF
					}
					catch(...)
					{
						assert(0);
					}
				#endif
				}
			}
		}
		
		lpobjectptr = lpobjectptr->m_mNextObject;
	}
	return blRet;
}

//end add
//..............................................................................................................................
void MCheckTimeOutObjectList::CheckSpObjectTimeOut(unsigned long lDelayTime,tagWriteReportFunction * fpFunction)
{
	MCheckTimeOutObjectList				*	lpobjectptr;
	char									tempbuf[256];
	register unsigned long					errorcode;
	
	lpobjectptr = s_mFirstObject;
	while ( lpobjectptr != NULL )
	{
		if ( lpobjectptr->m_bLocked == true )
		{
			//add by liuqy 20130608 for 数据有效性检查定义位置，即类成员数据定义必须在开始与结束之间
			if(CLSDATAFLAG_BEGIN != lpobjectptr->m_ushDataFlagBegin ||	CLSDATAFLAG_END !=	lpobjectptr->m_ushDataFlagEnd)
			{
				my_snprintf(tempbuf,	256,	"对象<%s>%s内存被破坏(0x%x 0x%x)]",
					lpobjectptr->m_strClassName, lpobjectptr->m_ushDataFlagBegin, lpobjectptr->m_ushDataFlagEnd);
			#ifndef	_LINUXTRYOFF
				try
				{
			#endif
					fpFunction(tempbuf);
			#ifndef	_LINUXTRYOFF
				}
				catch(...)
				{
					assert(0);
				}
			#endif
			}

			errorcode = lpobjectptr->m_mRefreshTime.GetDuration();
			if ( errorcode >= lDelayTime )
			{
				//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
				if(NULL != lpobjectptr->m_pstSrcFileLine)
					my_snprintf(tempbuf,256,"对象<%s:线%d>%s停止响应%d秒[附加数据%d](%s-%u)",
						lpobjectptr->m_strClassName, lpobjectptr->m_ulThreadID,lpobjectptr->m_strObjectName,errorcode / 1000,lpobjectptr->m_lAddtionData,
						lpobjectptr->m_pstSrcFileLine->szFileName, lpobjectptr->m_pstSrcFileLine->ushFileLine);
				else
					my_snprintf(tempbuf,256,"对象<%s:线%d>%s停止响应%d秒[附加数据%d]",lpobjectptr->m_strClassName, lpobjectptr->m_ulThreadID,lpobjectptr->m_strObjectName,errorcode / 1000,lpobjectptr->m_lAddtionData);

			#ifndef	_LINUXTRYOFF
				try
				{
			#endif
					fpFunction(tempbuf);
			#ifndef	_LINUXTRYOFF
				}
				catch(...)
				{
					assert(0);
				}
			#endif
			}
		}
		
		lpobjectptr = lpobjectptr->m_mNextObject;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
MCriticalSection::MCriticalSection(void) : MCheckTimeOutObjectList("MCriticalSection")
{
	#ifndef LINUXCODE
		::InitializeCriticalSection(&m_stSection);
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

		pthread_mutex_init(&m_stSection,&attr);

		//	ADD
		pthread_mutexattr_destroy(&attr);
		//	ADD END
	#endif
		//add by liuqy 20130608 for 数据有效性检查定义位置，即类成员数据定义必须在开始与结束之间
		m_ushMcsDataFlagBegin = CLSDATAFLAG_BEGIN;
		m_ushMcsDataFlagEnd = CLSDATAFLAG_END ;

}
//..............................................................................................................................
MCriticalSection::~MCriticalSection()
{
	#ifndef LINUXCODE
		::DeleteCriticalSection(&m_stSection);
	#else
		pthread_mutex_destroy(&m_stSection);
	#endif
}
//..............................................................................................................................
bool MCriticalSection::Lock(void)
{
	char szBuf[256];
	//add by liuqy 20130608 for 数据有效性检查定义位置，即类成员数据定义必须在开始与结束之间
	if(CLSDATAFLAG_BEGIN != m_ushMcsDataFlagBegin || CLSDATAFLAG_END !=	m_ushMcsDataFlagEnd)
	{
		//表示内存已经乱了
		memset(szBuf, 0, sizeof(szBuf));
		LIB_BYTE2ASC(sizeof(m_stSection), (char*)&m_stSection, szBuf);
		FILE * p= fopen("./MemMcs.txt", "a");
		if(NULL != p)
		{
			fprintf(p, "%d %d [%s]内存已经乱了[%s]\n", MDateTime::Now().DateToLong(),  MDateTime::Now().TimeToLong(),
				m_strClassName, szBuf);
			fclose(p);	
		}
	}

	#ifndef LINUXCODE
		::EnterCriticalSection(&m_stSection);
	#else
		if(0 > pthread_mutex_lock(&m_stSection))
			return false;
	#endif
#ifndef LINUXCODE
	m_ulThreadID = GetCurrentThreadId();
#else
	//m_ulThreadID = syscall(__NR_gettid);
	m_ulThreadID = (unsigned long)pthread_self();
#endif

	inner_setactive();
	return true;
}
//..............................................................................................................................
void MCriticalSection::UnLock(void)
{
	#ifndef LINUXCODE
		::LeaveCriticalSection(&m_stSection);
	#else
		pthread_mutex_unlock(&m_stSection);
	#endif

	inner_stopactive();
}
//add by liuqy 20110510 for 使用试加锁方式，加锁成功返回true，如果其它线程占用锁时，系统将不进行等待直接返回失败false
bool MCriticalSection::TryLock(void)
{
	bool blRet = false;
	#ifndef LINUXCODE
		blRet = ::TryEnterCriticalSection(&m_stSection);
	#else
		if(0 == pthread_mutex_trylock(&m_stSection))
			blRet = true;
	#endif
	return blRet;
}
//end add
//------------------------------------------------------------------------------------------------------------------------------
MFreeCriticalSection::MFreeCriticalSection(void) : MCheckTimeOutObjectList("MFreeCriticalSection")
{
	#ifndef LINUXCODE
		::InitializeCriticalSection(&m_stSection);
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

		pthread_mutex_init(&m_stSection,&attr);

		//	ADD
		pthread_mutexattr_destroy(&attr);
		//	ADD END
	#endif
}
//..............................................................................................................................
MFreeCriticalSection::~MFreeCriticalSection()
{
#ifndef LINUXCODE
	::DeleteCriticalSection(&m_stSection);
#else
	pthread_mutex_destroy(&m_stSection);
#endif
}
//..............................................................................................................................
void MFreeCriticalSection::Lock(void)
{
#ifndef LINUXCODE
	::EnterCriticalSection(&m_stSection);
#else
	pthread_mutex_lock(&m_stSection);
#endif
#ifndef LINUXCODE
	m_ulThreadID = GetCurrentThreadId();
#else
	//m_ulThreadID = syscall(__NR_gettid);
	m_ulThreadID = (unsigned long)pthread_self();
#endif
	
	inner_setactive();
}
//..............................................................................................................................
void MFreeCriticalSection::UnLock(void)
{
#ifndef LINUXCODE
	::LeaveCriticalSection(&m_stSection);
#else
	pthread_mutex_unlock(&m_stSection);
#endif
	
	inner_stopactive();
}
//------------------------------------------------------------------------------------------------------------------------------
MLocalSection::MLocalSection(void)
{
	m_mRecordData = NULL;
}
//..............................................................................................................................
MLocalSection::MLocalSection(const MCriticalSection * mIn)
{
	m_mRecordData = (MCriticalSection *)mIn;
	if ( m_mRecordData != NULL )
	{
		if(!m_mRecordData->Lock())
		{
			m_mRecordData = NULL;
			//throw exception("<MLocalMutex>加锁失败");
		}
	}
}
//..............................................................................................................................
MLocalSection::~MLocalSection()
{
	UnAttch();
}
//..............................................................................................................................
void MLocalSection::Attch(const MCriticalSection * mIn)
{
	if ( m_mRecordData != NULL )
	{
		assert(0);
		throw exception("<MLocalSection>重复Attch一个Section对象");
		return;
	}

	m_mRecordData = (MCriticalSection *)mIn;
	if ( m_mRecordData != NULL )
	{
		if(!m_mRecordData->Lock())
		{
			m_mRecordData = NULL;
			//throw exception("<MLocalMutex>加锁失败");
		}
	}
}
//add by liuqy 20110510 for 使用试加锁方式，加锁成功返回true，如果其它线程占用锁时，系统将不进行等待直接返回失败false
bool MLocalSection::TryAttch(const MCriticalSection * mIn)
{
	MCriticalSection * p;
	bool	blRet = false;
	if ( m_mRecordData != NULL )
	{
		assert(0);
		throw exception("<MLocalSection>重复TryAttch一个Section对象");
		return blRet;
	}

	p = (MCriticalSection *)mIn;
	if (p != NULL )
	{
		blRet = p->TryLock();
		if(blRet)
			m_mRecordData =	p;
	}
	return blRet;
}
//end add
//..............................................................................................................................
void MLocalSection::UnAttch(void)
{
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->UnLock();
		m_mRecordData = NULL;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
MMrSwMutex::MMrSwMutex(void) : MCheckTimeOutObjectList("MMrSwMutex")
{
	#ifndef LINUXCODE

		if ( (m_hWaitReadSem = ::CreateSemaphore(NULL,0,MAXLONG,NULL)) == NULL )
		{
			assert(0);
			throw exception("<MMrSwMutex>创建信号量发生错误");
		}
		
		if ( (m_hWaitWriteSem = ::CreateSemaphore(NULL,0,MAXLONG,NULL)) == NULL )
		{
			assert(0);
			throw exception("<MMrSwMutex>创建信号量发生错误");
		}
		
		::InitializeCriticalSection(&m_stSection);
		m_iWaitReadCount = 0;
		m_iWaitWriteCount = 0;
		m_iCurStatus = 0;

	#else
		
		pthread_rwlock_init(&m_stSection,NULL);

	#endif
}
//..............................................................................................................................
MMrSwMutex::~MMrSwMutex()
{
	#ifndef LINUXCODE

		::DeleteCriticalSection(&m_stSection);
		::CloseHandle(m_hWaitReadSem);
		::CloseHandle(m_hWaitWriteSem);
		m_iCurStatus = 0;

	#else

		pthread_rwlock_destroy(&m_stSection);
		
	#endif
}
//..............................................................................................................................
void MMrSwMutex::LockRead(void)
{
	#ifndef LINUXCODE

		register bool					bflag;
		register unsigned long			errorcode;
		
		::EnterCriticalSection(&m_stSection);
		
		bflag = ( m_iWaitWriteCount > 0 ) || ( m_iCurStatus < 0 );
		if ( bflag )
		{
			//目前正在写入操作或有线程正在等待写入操作，则等待
			m_iWaitReadCount ++;
		}
		else
		{
			//目前在读取或未操作状态，且没有线程等待写入操作，则读取
			m_iCurStatus ++;
		}
		
		::LeaveCriticalSection(&m_stSection);
		
		if ( bflag )
		{
			//目前正在写入操作或有线程正在等待写入操作，则等待，直到允许读取为止
			errorcode = ::WaitForSingleObject(m_hWaitReadSem,INFINITE);
			if ( errorcode == WAIT_FAILED )
			{
				assert(0);
				throw exception("<MMrSwMutex>LockRead等待信号量发生错误");
			}
		}

	#else

		pthread_rwlock_rdlock(&m_stSection);
		
	#endif

	inner_setactive();
}
//..............................................................................................................................
void MMrSwMutex::LockWrite(void)
{
	#ifndef LINUXCODE

		register bool					bflag;
		register unsigned long			errorcode;
		
		::EnterCriticalSection(&m_stSection);
		
		bflag = ( m_iCurStatus != 0 );
		if ( bflag )
		{
			//当前状态不为未操作状态，则等待（读取或写入状态）
			m_iWaitWriteCount ++;
		}
		else
		{
			//当前状态为未操作状态，修改状态，可以写入
			m_iCurStatus = -1;
		}
		
		::LeaveCriticalSection(&m_stSection);
		
		if ( bflag )
		{
			//当前状态不为未操作状态，则等待（读取或写入状态），直到可以写入为止
			errorcode = ::WaitForSingleObject(m_hWaitWriteSem,INFINITE);
			if ( errorcode == WAIT_FAILED )
			{
				assert(0);
				throw exception("<MMrSwMutex>LockWrite等待信号量发生错误");
			}
		}

	#else

		pthread_rwlock_wrlock(&m_stSection);

	#endif

	inner_setactive();
}
//..............................................................................................................................
void MMrSwMutex::UnLock(void)
{
	#ifndef LINUXCODE

		register HANDLE					htemphandle = NULL;
		register int					itemp = 1;
		
		::EnterCriticalSection(&m_stSection);
		
		if ( m_iCurStatus > 0 )
		{
			//读取状态
			m_iCurStatus --;
		}
		else if ( m_iCurStatus < 0 )
		{
			//写入状态
			m_iCurStatus ++;
		}
		
		if ( m_iCurStatus == 0 )
		{
			//如果本次unlock后，恢复为未操作状态，则通知等待读取或写入的线程
			if ( m_iWaitWriteCount > 0 )
			{
				//如果有线程写入等待，首先通知
				m_iCurStatus = -1;
				m_iWaitWriteCount --;
				htemphandle = m_hWaitWriteSem;
			}
			else if ( m_iWaitReadCount > 0 )
			{
				//如果没有写入等待，有读取等待，一起进入读取状态
				itemp = m_iWaitReadCount;
				m_iCurStatus = m_iWaitReadCount;
				m_iWaitReadCount = 0;
				htemphandle = m_hWaitReadSem;
			}
		}
		
		::LeaveCriticalSection(&m_stSection);
		
		if ( htemphandle != NULL )
		{
			::ReleaseSemaphore(htemphandle,itemp,NULL);
		}

	#else

		pthread_rwlock_unlock(&m_stSection);

	#endif

	inner_stopactive();
}
//------------------------------------------------------------------------------------------------------------------------------
MLocalMrSw::MLocalMrSw(void)
{
	m_mRecordData = NULL;
}
//..............................................................................................................................
MLocalMrSw::~MLocalMrSw()
{
	UnAttch();
}
//..............................................................................................................................
void MLocalMrSw::AttchRead(const MMrSwMutex * mIn)
{
	if ( m_mRecordData != NULL )
	{
		assert(0);
		throw exception("<MLocalMrSw>重复邦定一个MMrSwMutex对象读取");
		return;
	}

	m_mRecordData = (MMrSwMutex *)mIn;
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->LockRead();
	}
}
//..............................................................................................................................
void MLocalMrSw::AttchWrite(const MMrSwMutex * mIn)
{
	if ( m_mRecordData != NULL )
	{
		assert(0);
		throw exception("<MLocalMrSw>重复邦定一个MMrSwMutex对象写入");
		return;
	}
	
	m_mRecordData = (MMrSwMutex *)mIn;
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->LockWrite();
	}
}
//..............................................................................................................................
void MLocalMrSw::UnAttch(void)
{
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->UnLock();
		m_mRecordData = NULL;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
MWaitEvent::MWaitEvent(void)
{
	m_lInWaitCount = 0;
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
MWaitEvent::~MWaitEvent()
{
	m_blRunFlag = false;
	if(0 < m_lInWaitCount)
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
void MWaitEvent::Active(void)
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
void MWaitEvent::Wait(unsigned long lMSec)
{
	if(!m_blRunFlag	)
		return;
	#ifndef LINUXCODE
		
		register int				errorcode;
		InterlockedIncrement((PLONG)&m_lInWaitCount);
		errorcode = ::WaitForSingleObject(m_hRecordData,lMSec);
		InterlockedDecrement((PLONG)&m_lInWaitCount);
		if ( errorcode == WAIT_FAILED )
		{
			assert(0);
			throw exception("<MWaitEvent>等待事件发生错误");
		}
	#else
		int							rc, future;
		struct timespec				abstime;	//GUOGUO 20090723
		struct timeb				tp;

#if  __GCC_VER__ >= 40201
		__sync_fetch_and_add (&m_lInWaitCount, 1);		
#else
		pthread_mutex_lock(&m_hMutex);
		m_lInWaitCount++;
		pthread_mutex_unlock(&m_hMutex);

#endif
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
#if  __GCC_VER__ >= 40201
#else
					m_lInWaitCount--;					
#endif

					//	超时错误
					pthread_mutex_unlock(&m_hMutex);

#if  __GCC_VER__ >= 40201
					__sync_fetch_and_sub (&m_lInWaitCount, 1);
#endif

					return;
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
			pthread_mutex_unlock(&m_hMutex);
			throw exception("<MWaitEvent>等待事件发生错误");
#endif
		}
		m_nCount--;

#if  __GCC_VER__ >= 40201
#else
		m_lInWaitCount--;					
#endif

		pthread_mutex_unlock(&m_hMutex);

#if  __GCC_VER__ >= 40201
		__sync_fetch_and_sub (&m_lInWaitCount, 1);
#endif

	#endif
}
//------------------------------------------------------------------------------------------------------------------------------
MMutex::MMutex(void)
{
	#ifndef LINUXCODE
		m_hRecordData = NULL;
	#else
		m_iRecordData = -1;
	#endif
}
//..............................................................................................................................
MMutex::~MMutex()
{
	Close();
}
//..............................................................................................................................
int  MMutex::inner_createkeyfromstring(const char * strIn)
{
	register int					i;
	register int					errorcode = (int)strIn[0];

	for ( i=1;i<strlen(strIn);i++ )
	{
		switch ( i % 3 )
		{
			case 0:					errorcode *= (int)strIn[i];
									break;
			case 1:					errorcode += (int)strIn[i];
									break;
			case 2:					errorcode -= (int)strIn[i];
									break;
		}
	}

	return((errorcode > 0) ? (errorcode) : ((-1) * errorcode));
}
//..............................................................................................................................
int  MMutex::Create(MString strName)
{
	#ifndef LINUXCODE

		if ( (m_hRecordData = ::CreateMutex(NULL,FALSE,strName.c_str()))  == NULL )
		{
			return(MErrorCode::GetSysErr());
		}

		return(1);

	#else

		register int				iKey = inner_createkeyfromstring(strName.c_str());

		if ( (m_iRecordData = semget(iKey,1,0666 | IPC_CREAT)) == -1 )
		{
			return(MErrorCode::GetSysErr());
		}

		//	MODIFY BY GUOGUO 20090804
		//	设置信号量的初始值
		if(semctl(m_iRecordData, 0, SETVAL, 1) == -1)
		{
			semctl(m_iRecordData, 0, IPC_RMID);
			m_iRecordData = -1;
			return(MErrorCode::GetSysErr());
		}

		return(1);

	#endif
}
//..............................................................................................................................
int  MMutex::Open(MString strName)
{
	#ifndef LINUXCODE

		if ( (m_hRecordData = ::OpenMutex(MUTEX_ALL_ACCESS,FALSE,strName.c_str())) == NULL )
		{
			return(MErrorCode::GetSysErr());
		}

		return(1);

	#else

		register int				iKey = inner_createkeyfromstring(strName.c_str());

		if ( (m_iRecordData = semget(iKey,1,0666)) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}

		return(1);

	#endif
}
//..............................................................................................................................
void MMutex::Close(void)
{
	#ifndef LINUXCODE

		if ( m_hRecordData != NULL )
		{
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}

	#else

		if ( m_iRecordData != -1 )
		{
			semctl(m_iRecordData,0,IPC_RMID,0);
			m_iRecordData = -1;
		}

	#endif
}
//..............................................................................................................................
void MMutex::Lock(void)
{
	#ifndef LINUXCODE

		if ( ::WaitForSingleObject(m_hRecordData,INFINITE) == WAIT_FAILED )
		{
			assert(0);
			throw exception("<MMutex>等待互斥器发生错误");
		}

	#else

		struct sembuf					stsem;
		
		stsem.sem_num = 0;
		stsem.sem_op = -1;
		//	MODIFY BY GUOGUO 20090804
		//stsem.sem_flg = ~(IPC_NOWAIT | SEM_UNDO);	//	不等待,
		stsem.sem_flg = SEM_UNDO;					//	要等待,WINDOWS都是永远等待
		semop(m_iRecordData,&stsem,1);

	#endif
}
//..............................................................................................................................
void MMutex::UnLock(void)
{
	#ifndef LINUXCODE

		::ReleaseMutex(m_hRecordData);

	#else

		struct sembuf					stsem;

		stsem.sem_num = 0;
		stsem.sem_op = 1;
		//	MODIFY BY GUOGUO 20090804
		//stsem.sem_flg = ~(IPC_NOWAIT | SEM_UNDO);	//	不等待
		stsem.sem_flg = SEM_UNDO;					//	要等待WINDOWS都是永远等待
		semop(m_iRecordData,&stsem,1);

	#endif
}
//------------------------------------------------------------------------------------------------------------------------------
MLocalMutex::MLocalMutex(void)
{
	m_mRecordData = NULL;
}
//..............................................................................................................................
MLocalMutex::MLocalMutex(const MMutex * mIn)
{
	m_mRecordData = (MMutex *)mIn;

	if ( m_mRecordData != NULL )
	{
		m_mRecordData->Lock();
	}
}
//..............................................................................................................................
MLocalMutex::~MLocalMutex()
{
	UnAttch();
}
//..............................................................................................................................
void MLocalMutex::Attch(const MMutex * mIn)
{
	if ( m_mRecordData != NULL )
	{
		assert(0);
		throw exception("<MLocalMutex>同步变量已经邦定");
		return;
	}

	m_mRecordData = (MMutex *)mIn;
	
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->Lock();
	}
}
//..............................................................................................................................
void MLocalMutex::UnAttch(void)
{
	if ( m_mRecordData != NULL )
	{
		m_mRecordData->UnLock();
		m_mRecordData = NULL;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
