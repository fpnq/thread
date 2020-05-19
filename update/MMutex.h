//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ����⣨ͬ������Ԫ
//��Ԫ��������Ҫ�����̡߳����̵����ݵ�ͬ������
//�������ڣ�2007.3.15
//������Ա��¬��Զ
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
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
//add by liuqy 20130608 for �̻߳�����������ID���ļ�λ���к�ָ��

typedef struct __TSRCFILELINE{
	char			szFileName[32];
	unsigned short	ushFileLine;
}TSrcFileLine;

#define CLSDATAFLAG_BEGIN 0x4442
#define CLSDATAFLAG_END 0x4445
//..............................................................................................................................
//���������Ҫ����Щ��Ҫ���Ķ�����������Ȼ���ڹ̶�ʱ�����Ƿ񼤻�
//���̶߳��󣺶��ڼ���Ƿ�ֹͣ
//��ͬ�����󣺶��ڼ���Ƿ�ʱLock��û��UnLock
//�ȵ�
//��Ҫ���Ķ��󣬱���Ӹ�����м̳С�
class MCheckTimeOutObjectList
{
private:
	//add by liuqy 20111122 for ��ȫ����ָ��,�����߳�Ƶ������ʱ�ͷţ�����ֲ���ȫ����
	#ifndef LINUXCODE
		static CRITICAL_SECTION					s_oMySection;
	#else
		static pthread_mutex_t					s_oMySection;
	#endif
	//end add
protected:
	//add by liuqy 20130608 for ������Ч�Լ�鶨��λ�ÿ�ʼ�������Ա���ݶ�������ڿ�ʼ�����֮��
	unsigned short							m_ushDataFlagBegin;
	static MCheckTimeOutObjectList		*	s_mFirstObject;
	MCheckTimeOutObjectList				*	m_mNextObject;
	MCheckTimeOutObjectList				*	m_mPreObject;
	const char							*	m_strClassName;
	char									m_strObjectName[32];
	MCounter								m_mRefreshTime;
	unsigned long							m_lAddtionData;
	bool									m_bLocked;
	//add by liuqy 20130608 for �̻߳����������ļ�λ���к�ָ��
	TSrcFileLine						*	m_pstSrcFileLine;


	//modify by liuqy 20131031 for ���̺߳��Ƶ�MCheckTimeOutObjectList��
	unsigned	long			m_ulThreadID;	//�߳�ID

	//add by liuqy 20130608 for ������Ч�Լ�鶨��λ�ý����������Ա���ݶ�������ڿ�ʼ�����֮��
	unsigned short							m_ushDataFlagEnd;
protected:
	//����
	void inner_setactive(void);
	//�Ǽ���
	void inner_stopactive(void);
protected:
	//ˢ�¼�ʱ��
	void Refresh(void);
public:
	MCheckTimeOutObjectList(const char * strName);
	virtual ~MCheckTimeOutObjectList();
public:
	//��ⳬʱ���������lDelayTimeΪ��ʱʱ�䣨�룩������һ��Ϊ��־�������
	static void CheckSpObjectTimeOut(unsigned long lDelayTime,tagWriteReportFunction * fpFunction);
	//add by liuqy 20131211 for ��ʱ����
	//��ⳬʱ���������in_fpFunctΪ��ʱ��ʾ������������Ƿ���ڳ�ʱ
	static bool rbl_ChkSpObjTimeout(unsigned long lDelayTime,tagWriteReportFunction * in_fpFunct=NULL);
public:
	//���ö������ƣ��Ա�����ʾ����
	void SetSpObejctName(const char * strObjectName);
	const char * GetSpObjectName();
	unsigned long GetThreadID();
	//���ö��󸽼����ݣ���Ҫ���ڵ���
	void SetObjectAddtionData(unsigned long lIn);

	//	��ȡ��������
	unsigned long GetObjectAddtionData();
	
	//add by liuqy 20130608 for �̻߳����������ļ�λ���к�ָ��
	void SetSrcFileLine(const char * pszFileName, unsigned short in_ushFileLine);
	const char * GetSrcFileName();
	unsigned short GetSrcFileLine();
	//end add


};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalSection;
//..............................................................................................................................
//�ٽ�����ֻ������ͬһ�������У����Ҳ����ں˶����ص�Ϊ�ٶȿ죬CPU����С�����ý���ϵͳģʽ
//ע�⣺ʹ��ʱ�������MLocalSectionʹ�ã��Լ��������ķ���������Ĵ󲿷�ͬ��������ͬ��
class MCriticalSection : public MCheckTimeOutObjectList
{
friend class MLocalSection;
protected:
	//add by liuqy 20130608 for ������Ч�Լ�鶨��λ�ÿ�ʼ�������Ա���ݶ�������ڿ�ʼ�����֮��
	unsigned short							m_ushMcsDataFlagBegin;

	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;
	#else
		pthread_mutex_t						m_stSection;

	#endif
	//add by liuqy 20130608 for ������Ч�Լ�鶨��λ�ý����������Ա���ݶ�������ڿ�ʼ�����֮��
	unsigned short							m_ushMcsDataFlagEnd;

protected:
	//����
	bool Lock(void);
	//����
	void UnLock(void);
	//add by liuqy 20110510 for ʹ���Լ�����ʽ�������ɹ�����true����������߳�ռ����ʱ��ϵͳ�������еȴ�ֱ�ӷ���ʧ��false
	bool TryLock(void);
	
public:
	MCriticalSection(void);
	virtual ~MCriticalSection();
};
//..............................................................................................................................
//������������Ҫ��MCriticalSection����������ҪΪ�˽�������쳣��try....catch��������UnLockʱ�󣬻��������ѭ����������ɲ���UnLock������
//�����÷�Ϊ����һ���ֲ�����������������ʱUnLock�����£�
//int  function1(void)
//{
//		MLocalSection			tempsection;
//
//		//������ǰ����
//		
//		tempsection.Attch(&m_Section);
//
//		//���������һЩ���飬����ô������쳣����ֱ�������������m_Section��������ô�Ϳ���������������������UnLock
//
//		tempsection.UnAttch();
//
//		//���������һЩ����
//}
//
//		//Ҳ��������д������һ��Ҫȷ�ϣ��ú����ಿû�п������������ĵط�
//
//{
//		MLocalSection			tempsection(&m_Section);
//
//
//		//֪�������������ܹ��ͷ�UnLock�������ڲ��ܹ�����Ҫ�ͷź���Ҫ��������
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
	//�
	void Attch(const MCriticalSection * mIn);
	//add by liuqy 20110510 for ʹ���Լ�����ʽ�������ɹ�����true����������߳�ռ����ʱ��ϵͳ�������еȴ�ֱ�ӷ���ʧ��false
	bool TryAttch(const MCriticalSection * mIn);
	//����
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
	//����
	void Lock(void);
	//����
	void UnLock(void);
public:
	MFreeCriticalSection(void);
	virtual ~MFreeCriticalSection();
};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalMrSw;
//..............................................................................................................................
//��д������Ҫ���ڶ��̶߳�ȡ���ݣ����߳�д�����������ʹ�ã���MCriticalSectionֻ������һ����ȡ��һ��д�룩�����������MCriticalSection
//���Ӻķ���Դ
//������ԭ�������ǰ�����߳��ڶ���û�в���ʱ�����ȡ����������߳���д�룬��ȴ�
//д����ԭ�������ǰû�в�����д�룬��������߳��ڶ���д����ȴ�
//����ԭ������еȴ�д������ȼ���д����������д������������������򼤻��ȡ����
class MMrSwMutex : public MCheckTimeOutObjectList
{
friend class MLocalMrSw;
protected:
	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;				//�ڲ��������ٽ�������
		HANDLE								m_hWaitReadSem;				//�ȴ�д���ź��������ڵȴ��򼤻
		HANDLE								m_hWaitWriteSem;			//�ȴ������ź��������ڵȴ��򼤻
		int									m_iCurStatus;				//��ǰ״̬��=0��ʾ��ǰ���κβ�����>0��ʾ����ô������ڶ�ȡ��-1��ʾ��ǰ����д�룩
		int									m_iWaitReadCount;			//�ȴ���ȡ���߳�����
		int									m_iWaitWriteCount;			//�ȴ�д����߳�����
	#else
		pthread_rwlock_t					m_stSection;				//Linuxϵͳ�ṩ�������
	#endif
public:
	MMrSwMutex(void);
	virtual ~MMrSwMutex();
protected:
	//��ȡ���ݼ���
	void LockRead(void);
	//д�����ݼ���
	void LockWrite(void);
	//��������ȡ��д�룩
	void UnLock(void);
};
//..............................................................................................................................
//����ĺ���ͬMLocalSection֧��MCriticalSectionһ����ԭ��
class MLocalMrSw
{
protected:
	MMrSwMutex							*	m_mRecordData;
public:
	MLocalMrSw(void);
	virtual ~MLocalMrSw();
public:
	//�
	void AttchRead(const MMrSwMutex * mIn);
	void AttchWrite(const MMrSwMutex * mIn);
	//����
	void UnAttch(void);
};
//------------------------------------------------------------------------------------------------------------------------------
class MLocalMutex;
//..............................................................................................................................
//����̵�ͬ����
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
	//�������򿪡��ر�
	int  Create(MString strName);
	int  Open(MString strName);
	void Close(void);
protected:
	//��ȡ��д��
	void Lock(void);
	void UnLock(void);
};
//..............................................................................................................................
//�����ͬMLocalSection֧��MCriticalSectionһ��ԭ��
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
//�ȴ��¼�����Ȼ�������ڶ�����̣���������¼����ƣ�����Ϊ�˼���Linux��������Ҫ���ڽ����ڲ������̼�ʹ�õ��������������
//��Ҫ������ͷǼ���������������sleep���������ܹ�������Ӧ��
class MWaitEvent
{
protected:
	volatile int			m_lInWaitCount;
	volatile	bool				m_blRunFlag;
	#ifndef LINUXCODE
		HANDLE									m_hRecordData;
	#else
		int										m_IsStartWait;
		int										m_nCount;	//	��ȫ����,����thread_signal�ջ���,�Լ�����thread_wait���źŻ��ѵĴ���
		pthread_cond_t							m_hRecordData;
		pthread_mutex_t							m_hMutex;
	#endif
public:
	MWaitEvent(void);
	virtual ~MWaitEvent();
public:
	//�����¼�
	void Active(void);
	//�ȴ��¼�
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
