//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ��̵߳�Ԫ
//��Ԫ��������Ҫ�����߲�����
//�������ڣ�20111213
//������Ա��liuqy
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
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
 *	���ܣ������ӡ�������ص�����
 *	������in_oCustomSelfPRT	����-�û���ӡʱ�����ָ�룬һ��������ӡ����ʱ�����this������ֻ��ԭ�ⷵ��
 *		in_pcszMsg		����-��ӡ�ı�����Ϣ
 *	���أ���
 ************************************************************************/
#ifndef DF_rv_PrintReportCallBack
typedef void DF_rv_PrintReportCallBack(void * in_oCustomSelfPRT, const char * in_pcszMsg);
#endif

class CManageThreadBase ;

class CThreadBase 
{
	friend class CManageThreadBase;

public:
	//��ǰ�߳�ʹ�÷���һ���У����Ƿ���������
	CThreadBase(bool in_blExecRun0Flag, const char * strObjectName);
	//�߳�ʹ�÷���һ����
	CThreadBase(const char * strObjectName);

	virtual ~CThreadBase();
public:
	//����Ϊ�û��߳���Ҫʵ�ֵĺ���, ���߳�Ϊ����һʱ���������ĺ���Ϊʵ����Ϊ�ռ��ɣ���ʹ�÷�����ʱ������һ�ĺ�����Ϊ�ռ��ɡ�
	
	//�����̷߳���һ���������ܼ򵥣���ϵͳȥ��ɵȴ��¼�������ϵͳ��ѭ������NeedRun0Again,�������Ϊtrue������Run0

	//��Ҫ������ֻ�з���true���Ż�ȥ����Run����������Ϊfalse���̻߳�˯��200���룬����Ҫ��������ʱ�������SetRunEvent
	virtual	bool	NeedRun0Again() = 0;
	//�߳������������У�ע��ú����ڲ�����ѭ����ȴ��¼���ֻ��һ�Σ��ͷ��أ�ϵͳ��ͨ���ٴε��ý��빤��
	virtual void	Run0() = 0;
	
	//����Ϊ�̷߳����������¼���ȴ�ʱ�����û�����,ϵͳ���ȴ���ѭ������Run1�������û�ʵ�ֵ�Run1�����200�����ڷ��أ�����ϵͳ�ᱨ���̳߳�ʱ
	//�߳������������У��ú����ڿ�����ѭ��������ѭ������ȴ��¼�������Ҫ����200���룬ע������ȴ��û�����������֮ǰ(���ȴ�����Ҫ�����¼�)�������EntryWorking
	virtual void	Run1() = 0;
public:
	//����Ϊ�߳�������Ҫ�ĺ���

	//���з���һ�У�����������Ҫ��
	virtual	void	SetRunEvent();

	//���з���������Ҫȥ������
	//���빤����...����Ҫ����ͳ���߳�������ʱʹ�á��������ʱ����Ҫ����FinishWork
	void			EntryWorking();

	void			SetSrcFileLineName(char * in_pszSrcFileName,	unsigned short	in_ushSrcFileLine );	//Դ�ļ����к�


public:
	//�߳�����״̬
	typedef enum
	{
		RS_None	= 0,	//δ����״̬
		RS_Stop,		//�߳��Ѿ�ֹͣ��
		RS_Run ,		//�߳�����״̬
		RS_Suspend,		//�߳��Ѿ�������
		RS_Starting,	//�߳�����������
	}eRunState;

	//�����ǶԲ����Ŀ��Ʋ���
	//����һ���߳�
	int  StartThread(const char * in_pszName = NULL);

	// ֹͣ��ǰ�߳�,���߳��Զ�ֹͣ
	void StopThread(void);
	//ɱ����ǰ�߳�,�߳̿����Ѿ�������
	void Kill(unsigned long lWaitTime);
	//�߳��Ƿ�����������
	bool IsRunning();

	//ȡ��ǰ�߳��Ƿ�������״̬
	bool			IsEntrySuspend();
	//�����̹߳���״̬
	void			SetSuspend();
	//���������߳�
	void			ActivateFromSuspend();

	//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
	virtual CThreadBase::eRunState	GetRunState();		


	//���ص�ǰ�߳��Ƿ��Ѿ�ֹͣ��
	bool GetThreadStopFlag(void);

	//ע��ע�����Դ
	virtual void Unregister();


	// ���ߺ���
	static void Sleep(unsigned long lMSec);

	//�ó�CPU����Ȩһ��
	static void CpuYield();

	//���ö������ƣ��Ա�����ʾ����
	void SetSpObejctName(const char * strObjectName);

	//���ö��󸽼����ݣ���Ҫ���ڵ���
	void SetObjectAddtionData(unsigned long lIn);

	//���ö��󸽼�˵������Ҫ���ڵ���,���ݳ���Ϊ16�ֽ�
	void SetObjectAddtionData(const char * in_pszNote);
	//	��ȡ��������
	unsigned long GetObjectAddtionData();
	//	��ȡ��������,ֻ���ڻ�ȡ�����ַ���ʱʹ��
	const char * GetObjectAddtionDataStr();

	void Release();
	//ȡ��ǰ�߳�ID
	unsigned	long	GetThreadID();

protected:
	//ֹͣ�߳�������ͳ��
	void			StopStatisticsRate();
	//��ʼ�߳�������ͳ��
	void			StartStatisticsRate();
	//ȡ�߳�������ͳ�����
	bool			GetStatisticsRate();

	//��������������빤����(EntryWorking)���ʹ��
	void			FinishWork();

	//���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	MWaitEvent					m_oWaitEvent;

	bool						m_blStatisticsRate;	//�߳�������ͳ�Ʊ�־

	bool						m_blSuspend;	//�̹߳���
	bool						m_blEntrySuspend;	//�߳̽������״̬

	bool						m_blEntryWorkStatis;	//�߳�һ������ͳ��״̬��ʼ����Ҫ����ֹͣ״̬���������ֹͣ״̬
	MExactitudeCount			m_oWorkStatisUsecCounter;	//�̹߳���ͳ�ƣ�ͳ�ƽ���ʱ����Ҫ��ȡ�õ�ֵ���ӵ�ʵ�ʹ���ʱ���С�
	MExactitudeCount			m_oBeginStatisCounter;	//�߳̿�ʼ����ͳ��ʱ��
	time_t						m_stBeginWorkTime;	//�߳̿�ʼ����ʱ�䣬��1970����������

	unsigned	long			m_ulThreadID;	//�߳�ID
	unsigned __int64			m_uiLoopCount;	//�߳�ѭ������
	unsigned __int64			m_uiWorkCount;	//��������
	unsigned __int64			m_uiRealWorkUSECCount;	//�߳�ʵ�ʹ���ʱ��ͳ��(΢��)

	char						m_szObjectName[32];
	char						m_szAddtionData[16];
	MCounter					m_mRefreshTime;
	unsigned long				m_lAddtionData;

	unsigned short				m_ushSrcFileLine;	//Դ�ļ����к�
	char						m_szSrcFileName[32];	//Դ�ļ���


	bool						m_blProcessingFlag;	//���ڴ����־ add by liuqy 20101020 
	bool						m_blExecRun0Flag;	//���б�־����ʹ�÷���1��falseΪ����2
	bool						m_blRunFlag;
	bool						m_blRunningFlag;	//����������

	CManageThreadBase	*		m_poManageThread;	//�̹߳���
#ifndef LINUXCODE
	HANDLE							m_hRecordData;
#else
	pthread_t						m_hRecordData;
#endif

protected:

	//ˢ�¼�飬��Ҫ�����߳̽������״̬
	void					Refresh(void);
	bool								m_bStopCurThread;
	//��λ����
	void		Reset();
	static unsigned int __stdcall MyThreadRun(void * lpIn);


};

class CManageThreadBase 
{
public:
	CManageThreadBase ();
	~CManageThreadBase ();


	//����ǰ�߳�ע�ᵽ������
	int		Register(const char * in_pszManagerName, CThreadBase * in_poThread);
	//�ӹ�������ע��ָ���߳�
	bool	Unregister(CThreadBase * in_poThread);

	//���ҵ�ǰ�̶߳���
	CThreadBase * FindCurrentThread();

	//���õ�ǰ�߳��ļ��к�
	void			SetCurThrdSrcFileLine(char * in_pszSrcFileName, unsigned short	in_ushSrcFileLine );


	//��ʼ�߳�������ͳ��
	void	StartStatisticsRate();
	//ֹͣ�߳�������ͳ��
	void	StopStatisticsRate();


	//��ӡ�����߳���Ϣ
	void PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction);
	//��ӡָ��Ӧ���߳���Ϣ,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	void PrintfInfoFrmManageName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszManagerName);
	//��ӡָ���߳������߳���Ϣ,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	void PrintfInfoFrmName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszThreadName, const char * in_pszManagerName=NULL);
	//��ӡ��ʱ�߳���Ϣ
	bool ChkTimeOut(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction,
										  long in_lTimeOutSecond);

	//��ָ�����߳̽��й���
	bool		SetSuspFrmTheadID(unsigned	long in_ulThreadID);
	//��ָ�����߳̽��м���
	bool		ActiveFrmThreadID(unsigned	long in_ulThreadID);
	//ȡָ���̵߳�����״̬���̹߳���Ҳ������״̬��
	virtual CThreadBase::eRunState	GetRunStateFrmThreadID(unsigned	long in_ulThreadID);	
	//ָ�����߳��Ƿ���������
	virtual	bool		IsRunFrmThreadID(unsigned	long in_ulThreadID);
	//ָ�����߳��Ƿ񱻹���
	virtual	bool		IsSuspFrmThreadID(unsigned	long in_ulThreadID);

	//��ָ�����߳����ƽ��й���,�������ƿ��Բ���,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		SetSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//��ָ�����߳����ƽ��м���,�������ƿ��Բ���,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		ActiveFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);

	//ָ�����߳������Ƿ���������
	virtual	bool		IsRunFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//ָ�����߳������Ƿ񶼱�����
	virtual	bool		IsSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);

	//��ָ���������Ƶ��߳̽��й���,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		SetSuspFrmManagerName(const char * in_pszManagerName);
	//��ָ���������Ƶ��߳̽��м���,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		ActiveFrmManagerName(const char * in_pszManagerName);
	//ָ�����߳������Ƿ���������
	virtual	bool		IsRunFrmManagerName(const char * in_pszManagerName );
	//ָ�����߳������Ƿ񶼱�����
	virtual	bool		IsSuspFrmManagerName(const char * in_pszManagerName);

	//�������е��߳�
	void		SuspendAll();
	//�������е��߳�
	void		ActivateAll();

	//���е��̷߳���������
	bool		IsRunAll();
	//�����Լ��������߳��Ƿ񶼱�����
	bool		IsSuspUnselfAll();
	//�����Լ������й����߳�
	void		SuspendUnselfAll();

	//ɱ�����Լ�֮��������߳�
	void		KillThreadUnselfAll(unsigned long lWaitSecond);
	//�ر����е��߳�
	void		StopAllThread(void);
	//��ָ���������Ƶ��̹߳ر�,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		StopFrmManagerName(const char * in_pszManagerName);
	//��ָ���߳����Ƶ��̹߳ر�,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
	bool		StopFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName = NULL);
	//��ָ�����߳̽��йر�
	bool		StopFrmThreadID(unsigned	long in_ulThreadID);

	//ȡע���̵߳ĸ���
	long	GetCount();
	//ȡָ��λ�õ��߳�ID
	unsigned long GetThreadID(unsigned long in_ulPosID);

	//ȡָ��λ�õ��߳�����
	const char * GetThreadNameFrmPos(unsigned long in_ulPosID);
	//ȡָ���̵߳��߳�����
	const char *  GetThreadNameFrmThrdID(unsigned long in_ulPosID);
	//ȡָ��λ�õ��߳�Ӧ������
	const char * GetThreadAppNameFrmPos(unsigned long in_ulPosID);
	//ȡָ���̵߳��߳�Ӧ������
	const char *  GetThreadAppNameFrmThrdID(unsigned long in_ulPosID);

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
	long		rl_GetThreadCount(const char * in_strName = NULL, long * out_plActivateCnt = NULL, long * out_plProcessingCount = NULL,
		long * out_plSuspendCnt = NULL, 
		long * out_plOuttimeCnt = NULL);	//ȡ���߳�����





	//����Ƿ����˽�����ݾ��
	bool	rbl_IsExistThrdPrvDataHandle();


	//�߳�˽�����ݾ��
#ifndef  LINUXCODE
	DWORD		GetThrdPrvDataHandle() ;
#else
	pthread_key_t	GetThrdPrvDataHandle() ;
#endif


protected:
	typedef struct _TLINKNODE
	{
		TMyListNode		stNode;		//���ڵ�
		CThreadBase	*	poThread;	//ָ�����߳�
		char			szName[32];
	}TLinkNode;
	TMyListNode			m_stHeadNode;
	char				m_szErrMsg[512];

	MCriticalSection		m_oMutex;
	
	unsigned short			m_ushThreadCount;

	//�߳�˽�����ݾ��
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
