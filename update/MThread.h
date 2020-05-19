//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ��̵߳�Ԫ
//��Ԫ��������Ҫ�����̴߳�����ֹͣ�Ȳ���
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
#ifndef __MEngine_MThreadH__
#define __MEngine_MThreadH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MMutex.h"

#ifndef	LINUXCODE
	typedef 	unsigned int			eng_tid_t;
#else
	typedef		pthread_t				eng_tid_t;	// pthread_t is not thread id, but the program used it as thread id
#endif

//���涨�����̺߳���
typedef void * (__stdcall * tagMTheadFunction)(void *);
/************************************************************************
 *	���ܣ������ӡ�������ص�����
 *	������in_oCustomSelfPRT	����-�û���ӡʱ�����ָ�룬һ��������ӡ����ʱ�����this������ֻ��ԭ�ⷵ��
 *		in_pcszMsg		����-��ӡ�ı�����Ϣ
 *	���أ���
 ************************************************************************/
#ifndef DF_rv_PrintReportCallBack
typedef void DF_rv_PrintReportCallBack(void * in_oCustomSelfPRT, const char * in_pcszMsg);
#endif
//..........................................................................................................................
//ע�⣺�̺߳���Ӧ�ð������·�ʽ��д
/*
void * __stdcall threadfunction(void *in)
{
	��ʼ��һЩ�ֲ����������һ������Ǵ�this����

	//���߳�û��ֹͣ��ȫ���߳�û��ֹͣ
	while ( GetThreadStopFlag() == false )
	{
		�����̹߳���1
		//ע�⣺��������Ҫ���ڵ��ԣ�SetSpObjectAddtionData(1);

		�����̹߳���2
		//ע�⣺��������Ҫ���ڵ��ԣ�SetSpObjectAddtionData(2);

		�����̹߳���3
		//ע�⣺��������Ҫ���ڵ��ԣ�SetSpObjectAddtionData(3);
	}
	  
	return(0);
}
*/
//add by liuqy 20100816 for �̸߳��죬��ҪΪͳ�ƴ�ӡ�߳���Ϣ����һ������
class MThreadBase : public MCheckTimeOutObjectList
{
public:
	MThreadBase();
	~MThreadBase();
public:
	//ȡ��ǰ�߳��Ƿ�������״̬
	bool			rbl_IsEntrySuspend(){
		//�Ǽ���״̬ʱ������ʾ�Ѿ�����
		return (m_blEntrySuspend || !m_bLocked);
	};
	//�����̹߳���״̬
	void			rv_SetSuspend(){
		m_blSuspend = true;
	};
	//���������߳�
	void			rv_ActivateFromSuspend(){
		m_blSuspend = false;
		//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
		m_oWaitEvent.Active();
		rv_CpuYield(); 
		m_blSuspend = false;
		m_oWaitEvent.Active();
	};
	//���빤����...����Ҫ����ͳ���߳�������ʱʹ�á��������ʱ����Ҫ����rv_FinishWork
	void			rv_EntryWorking();
	//��������������빤����(rv_EntryWorking)���ʹ��
	void			rv_FinishWork();
	//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
	virtual bool	rbl_GetRunState() = 0;		
	//add by liuqy 20101210 for ȡ�����е�״̬
	//ȡ��ǰ�߳��Ƿ�����˹�����
	bool			rbl_GetEntryWorkingFlag(){
		return m_blEntryWorkStatis;
	}
	//add by liuqy 20100830 for Ϊ��֤������ݵ���ȷ�ԣ���Ҫ�ó�CPU����Ȩ
	static void rv_CpuYield();

public:
	//��ӡ�����߳���Ϣ
	static void rv_PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction);
	//��ӡָ���߳���Ϣ
	static void rv_PrintfInfo(void * in_oCustomSelfPRT, const char * in_strName, DF_rv_PrintReportCallBack * fpFunction);
	//ֹͣ�߳�������ͳ��
	static void			rv_StopStatisticsRate(){
		s_blStatisticsRate = false;
	};
	//��ʼ�߳�������ͳ��
	static void			rv_StartStatisticsRate();
	//add by liuqy 20100827 for ����ȡ״̬
	//ȡ�߳�������ͳ�����
	static bool			rbl_GetStatisticsRate(){
		return s_blStatisticsRate;
	};
	//add by liuqy 20100917
	//ͨ�������н�ָ�����߳̽��й���
	static bool		rbl_SetSuspFrmTheadID(unsigned	long in_ulThreadID);
	//ͨ�������н�ָ�����߳̽��й���
	static bool		rbl_ActSuspFrmThreadID(unsigned	long in_ulThreadID);
	//add by liuqy 20110505 for �������е��߳�
	static void		rv_ActivateAll();

	//add by liuqy 20101020 for ����ͳ�Ƶ�ǰ�߳����������߳���
	/*****************************
	 * ��������rl_GetThreadCount
	 * ���ܣ�ȡ�̸߳���
	 * ������in_strName	����-��ѯ���߳����ƣ�������ǰ��������ͬ��
	 *		out_plActivateCnt	���-������߳�����(��ֹͣ��û��ʹ�õ�)
	 *		out_plProcessingCount	���-��ǰ����ִ�д�����̸߳���
	 *		out_plSuspendCnt	���-�һ��̸߳���
	 *		out_plOuttimeCnt	���-��ʱ�߳���
	 * ���أ��̸߳���
	 *****************************/
	static long		rl_GetThreadCount(const char * in_strName = NULL, long * out_plActivateCnt = NULL, long * out_plProcessingCount = NULL,
		long * out_plSuspendCnt = NULL, 
		long * out_plOuttimeCnt = NULL);	//ȡ���߳�����
	//add by liuqy 20171213 for �߳�ֹͣ�󣬱��븴λ��ز���
	void		rv_Reset();
	//end add by liuqy 20171213
	
protected:
	static MThreadBase		*	s_poFirstThread;
	static bool					s_blStatisticsRate;	//�߳�������ͳ�Ʊ�־
#ifndef LINUXCODE
	static 	CRITICAL_SECTION					s_stSection;
#else
	static 	pthread_mutex_t						s_stSection;
#endif
	static	bool				s_blSectionFlag;
	//�����ٽ���
	static void rv_CreateSection();
	//ɾ���ٽ���
	static void rv_DeleteSection();
	//����
	static void	rv_LockSection();
	//����
	static void rv_UnlockSection();
protected:
	MThreadBase				*	m_mNextThread;
	MThreadBase				*	m_mPreThread;
	//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	MWaitEvent					m_oWaitEvent;

	bool						m_blSuspend;	//�̹߳���
	bool						m_blEntrySuspend;	//�߳̽������״̬
	bool						m_blBeginWorkFlag;	//�߳̿�ʼ������־

	bool						m_blEntryWorkStatis;	//�߳�һ������ͳ��״̬��ʼ����Ҫ����ֹͣ״̬���������ֹͣ״̬
	MExactitudeCount			m_oWorkStatisUsecCounter;	//�̹߳���ͳ�ƣ�ͳ�ƽ���ʱ����Ҫ��ȡ�õ�ֵ���ӵ�ʵ�ʹ���ʱ���С�
	MExactitudeCount			m_oBeginStatisCounter;	//�߳̿�ʼ����ͳ��ʱ��
	time_t						m_stBeginWorkTime;	//�߳̿�ʼ����ʱ�䣬��1970����������
	//modify by liuqy 20131031 for ���̺߳��Ƶ�MCheckTimeOutObjectList��
	//unsigned	long			m_ulThreadID;	//�߳�ID
	unsigned __int64			m_uiLoopCount;	//�߳�ѭ������
	unsigned __int64			m_uiWorkCount;	//��������
	unsigned __int64			m_uiRealWorkUSECCount;	//�߳�ʵ�ʹ���ʱ��ͳ��(΢��)
	static bool							s_bStopAllThread;
	bool						m_blProcessingFlag;	//���ڴ����־ add by liuqy 20101020 
protected:
	//ˢ�¼�飬��Ҫ�����߳̽������״̬
	void Refresh(void);
	bool								m_bStopCurThread;
	//��λ����



};
//end add by liuqy 
//ע�⣺Ϊ�˼���Windows����ϵͳ��Linux����ϵͳ��ȥ������ͣ���ָ��̲߳���
//..........................................................................................................................
//modify by liuqy 20100816 for �̸߳��죬��ҪΪͳ�ƴ�ӡ�߳���Ϣ����һ������,�ô���ʵ���̹߳��𣨷ǲ���ϵͳ�Ĺ��𣩣���¼�̹߳������
class MThread : public MThreadBase
{
	//��ֹͣ��־�ƶ�����������
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
	// ������ֹͣ�߳�
	int  StartThread(MString strName,tagMTheadFunction fpFunction,void * lpParam,bool bCheckAlive = true);
	void StopThread(unsigned long lWaitTime = 5000);

public:
	// ��ȡ�߳��Ƿ�ֹͣ��־ ע���߳���Ҫ����һЩ��������ˢ��ʱ�䣬����ʱ�����������˳��ȴ�
	bool GetThreadStopFlag(void);
	//add by liuqy 20110323 for ��ȡֹͣ��־��ֱ�ӷ��ر�־���������κβ���
	bool TestGetThreadStopFlag();
	// ֹͣ���̣߳����ñ��̱߳�־��
	void StopCurThread(void);

	//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
	bool	rbl_GetRunState();
public:
	// ֹͣ��Modual�������߳�
	static void StopAllThread(void);

	// ���ߺ���
	static void Sleep(unsigned long lMSec);


	static void ExitThread(void);
	static void DetachThread(void);
	static int  CreateThreadExt( size_t stack_size, tagMTheadFunction
				fpFunction, void* lpParam, eng_tid_t& tid );
};
//------------------------------------------------------------------------------------------------------------------------------
//�������߳��������һ����װ���µ��߳�����ü̳и��̵߳ķ�ʽ
class MThreadClass : public MThread
{
protected:
public:
	MThreadClass(void);
	virtual ~MThreadClass();
protected:
	static void * __stdcall threadfunction(void * lpIn);
public:
	//�����̣߳�ֹͣ�߳�ʹ�û����
	int  StartThread(MString strName);
public:
	//����������Ǽ̳�ʱ����Ҫд���࣬���̵߳�ѭ���壬д��ͬ���ࣨ�����棩
	virtual int  Execute(void) = 0;
};
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
