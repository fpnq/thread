//------------------------------------------------------------------------------------------------------------------------------
//单元名称：互斥（同步）单元
//单元描述：主要处理线程、进程等数据的同步问题
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
#ifndef __MEngine_MMutexH__
#define __MEngine_MMutexH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MDateTime.h"
//------------------------------------------------------------------------------------------------------------------------------
typedef void tagWriteReportFunction(const char * strIn);
//add by liuqy 20130608 for 线程基类中增加类ID及文件位置行号指针

typedef struct __TSRCFILELINE{
	char			szFileName[32];
	unsigned short	ushFileLine;
}TSrcFileLine;

#define CLSDATAFLAG_BEGIN 0x4442
#define CLSDATAFLAG_END 0x4445
//..............................................................................................................................
//下面的类主要把那些需要检查的对象串联起来，然后在固定时间检查是否激活
//如线程对象：定期监测是否停止
//如同步对象：定期检查是否超时Lock，没有UnLock
//等等
//需要检查的对象，必须从该类进行继承。
class MCheckTimeOutObjectList
{
private:
	//add by liuqy 20111122 for 安全操作指针,当多线程频繁操作时释放，会出现不安全操作
	#ifndef LINUXCODE
		static CRITICAL_SECTION					s_oMySection;
	#else
		static pthread_mutex_t					s_oMySection;
	#endif
	//end add
protected:
	//add by liuqy 20130608 for 数据有效性检查定义位置开始，即类成员数据定义必须在开始与结束之间
	unsigned short							m_ushDataFlagBegin;
	static MCheckTimeOutObjectList		*	s_mFirstObject;
	MCheckTimeOutObjectList				*	m_mNextObject;
	MCheckTimeOutObjectList				*	m_mPreObject;
	const char							*	m_strClassName;
	char									m_strObjectName[32];
	MCounter								m_mRefreshTime;
	unsigned long							m_lAddtionData;
	bool									m_bLocked;
	//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
	TSrcFileLine						*	m_pstSrcFileLine;


	//modify by liuqy 20131031 for 将线程号移到MCheckTimeOutObjectList中
	unsigned	long			m_ulThreadID;	//线程ID

	//add by liuqy 20130608 for 数据有效性检查定义位置结束，即类成员数据定义必须在开始与结束之间
	unsigned short							m_ushDataFlagEnd;
protected:
	//激活
	void inner_setactive(void);
	//非激活
	void inner_stopactive(void);
protected:
	//刷新计时器
	void Refresh(void);
public:
	MCheckTimeOutObjectList(const char * strName);
	virtual ~MCheckTimeOutObjectList();
public:
	//检测超时的特殊对象，lDelayTime为超时时间（秒），后面一个为日志输出函数
	static void CheckSpObjectTimeOut(unsigned long lDelayTime,tagWriteReportFunction * fpFunction);
	//add by liuqy 20131211 for 超时处理
	//检测超时的特殊对象，in_fpFunct为空时表示不输出，返回是否存在超时
	static bool rbl_ChkSpObjTimeout(unsigned long lDelayTime,tagWriteReportFunction * in_fpFunct=NULL);
public:
	//设置对象名称，以便于提示查找
	void SetSpObejctName(const char * strObjectName);
	const char * GetSpObjectName();
	unsigned long GetThreadID();
	//设置对象附加数据，主要便于调试
	void SetObjectAddtionData(unsigned long lIn);

	//	获取附加数据
	unsigned long GetObjectAddtionData();
	
	//add by liuqy 20130608 for 线程基类中增加文件位置行号指针
	void SetSrcFileLine(const char * pszFileName, unsigned short in_ushFileLine);
	const char * GetSrcFileName();
	unsigned short GetSrcFileLine();
	//end add


};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalSection;
//..............................................................................................................................
//临界区，只能用于同一个进程中，并且不是内核对象，特点为速度快，CPU耗用小，不用进入系统模式
//注意：使用时必须配合MLocalSection使用，以减少死锁的发生，下面的大部分同步对象相同。
class MCriticalSection : public MCheckTimeOutObjectList
{
friend class MLocalSection;
protected:
	//add by liuqy 20130608 for 数据有效性检查定义位置开始，即类成员数据定义必须在开始与结束之间
	unsigned short							m_ushMcsDataFlagBegin;

	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;
	#else
		pthread_mutex_t						m_stSection;

	#endif
	//add by liuqy 20130608 for 数据有效性检查定义位置结束，即类成员数据定义必须在开始与结束之间
	unsigned short							m_ushMcsDataFlagEnd;

protected:
	//加锁
	bool Lock(void);
	//解锁
	void UnLock(void);
	//add by liuqy 20110510 for 使用试加锁方式，加锁成功返回true，如果其它线程占用锁时，系统将不进行等待直接返回失败false
	bool TryLock(void);
	
public:
	MCriticalSection(void);
	virtual ~MCriticalSection();
};
//..............................................................................................................................
//下面的这个类主要是MCriticalSection的衍生，主要为了解决发生异常后（try....catch）、忘记UnLock时后，会造成跳出循环或函数，造成不能UnLock的现象
//基本用法为申请一个局部变量，利用他析够时UnLock，如下：
//int  function1(void)
//{
//		MLocalSection			tempsection;
//
//		//做锁定前操作
//		
//		tempsection.Attch(&m_Section);
//
//		//作锁定后的一些事情，如果该处发生异常，将直接跳出，不会对m_Section解锁，那么就可以利用析够函数来处理UnLock
//
//		tempsection.UnAttch();
//
//		//做解锁后的一些事情
//}
//
//		//也可以如下写法，但一定要确认，该函数类部没有可以引起死锁的地方
//
//{
//		MLocalSection			tempsection(&m_Section);
//
//
//		//知道函数结束才能够释放UnLock，函数内不能够有需要释放后需要做的事情
//}
class MLocalSection
{
protected:
	MCriticalSection					*	m_mRecordData;
public:
	MLocalSection(void);
	MLocalSection(const MCriticalSection * mIn);
	virtual ~MLocalSection();
public:
	//邦定
	void Attch(const MCriticalSection * mIn);
	//add by liuqy 20110510 for 使用试加锁方式，加锁成功返回true，如果其它线程占用锁时，系统将不进行等待直接返回失败false
	bool TryAttch(const MCriticalSection * mIn);
	//解除邦定
	void UnAttch(void);
};
//------------------------------------------------------------------------------------------------------------------------------
class MFreeCriticalSection : public MCheckTimeOutObjectList
{
protected:
	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;
	#else
		pthread_mutex_t						m_stSection;
	#endif
public:
	//加锁
	void Lock(void);
	//解锁
	void UnLock(void);
public:
	MFreeCriticalSection(void);
	virtual ~MFreeCriticalSection();
};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalMrSw;
//..............................................................................................................................
//读写锁，主要用于多线程读取数据，单线程写入数据情况下使用（用MCriticalSection只能允许一个读取或一个写入），但这个将比MCriticalSection
//更加耗费资源
//读操作原则：如果当前其它线程在读或没有操作时，则读取，如果其他线程在写入，则等待
//写操作原则：如果当前没有操作则写入，如果其他线程在读或写入则等待
//解锁原则：如果有等待写入的优先激活写入操作，如果写入操作不够条件激活则激活读取操作
class MMrSwMutex : public MCheckTimeOutObjectList
{
friend class MLocalMrSw;
protected:
	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;				//内部变量的临界区保护
		HANDLE								m_hWaitReadSem;				//等待写的信号量（用于等待或激活）
		HANDLE								m_hWaitWriteSem;			//等待读的信号量（用于等待或激活）
		int									m_iCurStatus;				//当前状态（=0表示当前无任何操作，>0表示有这么多个正在读取，-1表示当前正在写入）
		int									m_iWaitReadCount;			//等待读取的线程数量
		int									m_iWaitWriteCount;			//等待写入的线程数量
	#else
		pthread_rwlock_t					m_stSection;				//Linux系统提供这个方法
	#endif
public:
	MMrSwMutex(void);
	virtual ~MMrSwMutex();
protected:
	//读取数据加锁
	void LockRead(void);
	//写入数据加锁
	void LockWrite(void);
	//解锁（读取、写入）
	void UnLock(void);
};
//..............................................................................................................................
//下面的函数同MLocalSection支持MCriticalSection一样的原理
class MLocalMrSw
{
protected:
	MMrSwMutex							*	m_mRecordData;
public:
	MLocalMrSw(void);
	virtual ~MLocalMrSw();
public:
	//邦定
	void AttchRead(const MMrSwMutex * mIn);
	void AttchWrite(const MMrSwMutex * mIn);
	//解除邦定
	void UnAttch(void);
};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalMutex;
//..............................................................................................................................
//跨进程的同步锁
class MMutex
{
friend class MLocalMutex;
protected:
	#ifndef LINUXCODE
		HANDLE								m_hRecordData;
	#else
		int									m_iRecordData;
	#endif
protected:
	__inline int  inner_createkeyfromstring(const char * strIn);
public:
	MMutex(void);
	virtual ~MMutex();
public:
	//创建、打开、关闭
	int  Create(MString strName);
	int  Open(MString strName);
	void Close(void);
protected:
	//读取、写入
	void Lock(void);
	void UnLock(void);
};
//..............................................................................................................................
//下面的同MLocalSection支持MCriticalSection一样原理
class MLocalMutex
{
protected:
	MMutex					*	m_mRecordData;
public:
	MLocalMutex(void);
	MLocalMutex(const MMutex * mIn);
	virtual ~MLocalMutex();
public:
	void Attch(const MMutex * mIn);
	void UnAttch(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//等待事件，虽然可以用在多个进程（必须加入事件名称），但为了兼容Linux，该类主要用于进程内部，进程间使用的请参照其他的类
//主要处理激活和非激活，可以用它来替代sleep函数，它能够提早响应。
class MWaitEvent
{
protected:
	volatile int			m_lInWaitCount;
	volatile	bool				m_blRunFlag;
	#ifndef LINUXCODE
		HANDLE									m_hRecordData;
	#else
		int										m_IsStartWait;
		int										m_nCount;	//	安全计数,避免thread_signal空唤醒,以及保持thread_wait被信号唤醒的处理
		pthread_cond_t							m_hRecordData;
		pthread_mutex_t							m_hMutex;
	#endif
public:
	MWaitEvent(void);
	virtual ~MWaitEvent();
public:
	//激活事件
	void Active(void);
	//等待事件
	void Wait(unsigned long lMSec = 0xFFFFFFFF);
	long GetInWaitCount() {return m_lInWaitCount;}
};
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
