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
//��ǰ�߳�ʹ�÷���һ���У����Ƿ���������
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
//ȡ��ǰ�߳�ID
unsigned	long	CThreadBase::GetThreadID()
{
	return m_ulThreadID;
}

//ע��ע�����Դ
void CThreadBase::Unregister()
{
	if(NULL != m_poManageThread)
		m_poManageThread->Unregister(this);
}

bool CThreadBase::IsEntrySuspend()
{
	//������״̬ʱ������ʾ�Ѿ�����
	return (m_blEntrySuspend || !m_blRunningFlag);
}
//�����̹߳���״̬
void			CThreadBase::SetSuspend()
{
	m_blSuspend = true;
}
//���������߳�
void		CThreadBase::ActivateFromSuspend()
{
	m_blSuspend = false;
	//���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	m_oWaitEvent.Active();
	CpuYield(); 
	CpuYield(); 
	m_blSuspend = false;
	m_oWaitEvent.Active();
}
//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
CThreadBase::eRunState	CThreadBase::GetRunState()
{
	//û������
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

			iRet = pthread_kill(m_hRecordData, 0);	//	����0�ź�,��ͨ������������߳��Ƿ����
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
//Դ�ļ����к�
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
	m_blStatisticsRate = false;	//�߳�������ͳ�Ʊ�־
	m_blSuspend = false;	//�̹߳���
	m_blEntrySuspend = true;	//�߳̽������״̬
	m_blEntryWorkStatis = false;	//�߳�һ������ͳ��״̬��ʼ����Ҫ����ֹͣ״̬���������ֹͣ״̬
	m_stBeginWorkTime = 0;	//�߳̿�ʼ����ʱ�䣬��1970����������
	m_ulThreadID =0;	//�߳�ID 
	m_uiLoopCount = 0 ;	//�߳�ѭ������
	m_uiWorkCount = 0;	//��������
	m_uiRealWorkUSECCount = 0;	//�߳�ʵ�ʹ���ʱ��ͳ��(΢��)
	m_blProcessingFlag = false;	//���ڴ����־ add by liuqy 20101020 
	//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	m_oWaitEvent.Active();
	memset(m_szAddtionData, 0, sizeof(m_szAddtionData));
	m_lAddtionData = 0;

}
//ֹͣ�߳�������ͳ��
void	CThreadBase::StopStatisticsRate()
{
	m_blStatisticsRate = false;
};
//ȡ�߳�������ͳ�����
bool	CThreadBase::GetStatisticsRate()
{
	return m_blStatisticsRate;
};
//���빤����...����Ҫ����ͳ���߳�������ʱʹ�á��������ʱ����Ҫ����FinishWork
void	CThreadBase::EntryWorking()
{
	if(!m_blRunFlag)
		return ;
	m_blProcessingFlag = true;	//���ڴ����־ add by liuqy 20101020 
	if(!m_blStatisticsRate || m_blEntryWorkStatis)
		return;
	FinishWork();
	m_uiWorkCount++;
	m_oWorkStatisUsecCounter.SetCurTickCount();
	m_blEntryWorkStatis = true;
}

//��������������빤����(EntryWorking)���ʹ��
void		CThreadBase::FinishWork()
{
	m_blProcessingFlag = false;	//���ڴ����־ add by liuqy 20101020 
	if(!m_blStatisticsRate || !m_blEntryWorkStatis)
		return;
	m_uiRealWorkUSECCount += m_oWorkStatisUsecCounter.GetI64Duration();
	m_blEntryWorkStatis = false;

}
//ˢ�¼�飬��Ҫ�����߳̽������״̬,������̵߳�һ�ν��У�����Ҫ���ÿ�ʼʱ��
void CThreadBase::Refresh(void)
{
	if(!m_blRunFlag)
		return ;
	//��һ�ν��룬���߳̿�ʼ������
	if(0 == m_stBeginWorkTime)
	{
		time(&m_stBeginWorkTime);
	}
	FinishWork();
	//��Ҫͳ���߳�ʹ����ʱ������ѭ��������û�н���ʱ��ͳ�Ƶģ�ȥ������
	if(m_blStatisticsRate)
	{
		m_uiLoopCount++;
	}
	m_mRefreshTime.SetCurTickCount();
	//�߳���Ҫ����
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

// ���ߺ���
void CThreadBase::Sleep(unsigned long lMSec)
{
	#ifndef LINUXCODE
		::Sleep(lMSec);
	#else
		//usleep(lMSec*1000);
		//	LINUXϵͳ�Ƽ�ʹ��select����ʱ,usleep�൱�Ĳ���ȷ.��������man usleep���Կ���
		//	�������и�����ë��
		//	������ʱ�����ֶ���Ҫ����ʱ���жϵ�,��һ΢���ʱ���ж�ô��û�С�
		//	�ж϶��Ǻ��뼶��,����˵Ŀǰ�����,
		//	usleep(1)�϶�����ֻ�ӳ�1΢�룬����һ��ʱ�����
		//	���Ҵӵȴ����л������ж���ҲҪ�ܵ���,
		//	�������ж���ѡ����̵�ʱ��,�����������ߵġ�
		//	�Դ�����cpu�������������,���ȿ��Ǳ�cpu����,
		//	usleep�Ժ��п��������ӡ������ν��̵�����Ҫ�չˣ�
		//	ͬһ���̵������̼߳���л��мӷ֣�usleep�Ժ�϶�Ҳ���ܲ�����
		//	����������usleepʵ�ֶ���ʱ�Ƿǳ����ɿ��ġ�
		//	usleep()���кܴ������

		//	��һЩLinux 
		//	usleep()��Ӱ���ź� 
		//	��ĳЩLinux�£���������ֵ����С��1 * 1000 * 1000Ҳ����1�룬����ú����ᱨ�������������ء� 
		//	�󲿷�ƽ̨�İ����ĵ��Ѿ���ȷ˵�ˣ��ú������Ѿ��������ĺ����� 

		struct timeval stdelay;
		lMSec = lMSec * 1000;
		stdelay.tv_sec = lMSec / 1000000;
		stdelay.tv_usec = lMSec % 1000000;
		select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &stdelay);
	#endif

}

//�ó�CPU����Ȩһ��
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

//���ö������ƣ��Ա�����ʾ����
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

//���ö��󸽼����ݣ���Ҫ���ڵ���
void CThreadBase::SetObjectAddtionData(unsigned long lIn)
{
	m_lAddtionData = lIn;

}
//���ö��󸽼�˵������Ҫ���ڵ���,���ݳ���Ϊ16�ֽ�
void CThreadBase::SetObjectAddtionData(const char * in_pszNote)
{
	memcpy(m_szAddtionData, in_pszNote, sizeof(m_szAddtionData));
	m_szAddtionData[sizeof(m_szAddtionData)-1] = 0;
}
//	��ȡ��������,ֻ���ڴ����ַ���ʱʹ��
const char * CThreadBase::GetObjectAddtionDataStr()
{
	return m_szAddtionData;
}
//	��ȡ��������
unsigned long CThreadBase::GetObjectAddtionData()
{
	return m_lAddtionData;

}


//..............................................................................................................................
bool CThreadBase::GetThreadStopFlag(void)
{
	return !m_blRunFlag;
}
//�߳��Ƿ�����������
bool CThreadBase::IsRunning()
{
	return m_blRunningFlag ;
}
//..............................................................................................................................
// ֹͣ���̣߳����ñ��̱߳�־��
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
//���з���һ�У�����������Ҫ��
void	CThreadBase::SetRunEvent()
{
	m_oWaitEvent.Active();
}
int  CThreadBase::StartThread(const char * in_pszName)
{

	StopThread();
	//ǰһ���߳����û��ֹͣ�����20�ξ�ȥǿɱ
	CpuYield();
	
	if(RS_Stop < GetRunState())
	{
		Kill(1000);
	}
	Reset();

	//�߳�û��ע�ᣬ��������
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
		//�����߳�ID��
		m_ulThreadID = errorcode;

	#else

		/*
		 *	LINUX pthread_create�������̵߳Ķ�ջ��10MB
		 *	���ǵĳ���ͨ���ò�����ô��,���Ե���Ϊ2MB
		 *	�������Լ����߼���ַ
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

		//�����߳�ջ�ռ�Ϊ2M.
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
		//�����߳�ID��
		m_ulThreadID = m_hRecordData;
		

	#endif
		if(NULL != in_pszName && 0 < strlen(in_pszName))
			SetSpObejctName(in_pszName);
		
		return(1);
}

//..............................................................................................................................
//ɱ����ǰ�߳�
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
				iRet = pthread_kill(m_hRecordData, 0);	//	����0�ź�,��ͨ������������߳��Ƿ����
				if(iRet == 0)
				{
					//	�̻߳�����
					this->CpuYield();
				}
				else if(iRet == ESRCH)
				{
					//	�߳��Ѿ������˶���.
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
				//	��ʱ��
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
	//add by liuqy 20130613 for �߳�˽�����ݾ��
#ifndef  LINUXCODE
	m_ThrdPrvDataHandle = TLS_OUT_OF_INDEXES;	
#else
	m_blCrtThrdPrvDataHndl = false;
#endif
	//add by liuqy 20130613 for �߳�˽�����ݾ��
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
		sprintf( m_szErrMsg,"�����߳�˽�����ݴ���������:%d %s", errorcode, MErrorCode::GetErrString(errorcode).c_str());
	}

}
CManageThreadBase::~CManageThreadBase()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	
	pstThreadNode = NULL;
	//��ѯ���нڵ�
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
	//add by liuqy 20130613 for �߳�˽�����ݾ��
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
//ֹͣ�߳�������ͳ��
void	CManageThreadBase::StopStatisticsRate()
{
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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

//��ʼ�߳�������ͳ��
void	CManageThreadBase::StartStatisticsRate()
{
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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

//����Ƿ����˽�����ݾ��
bool	CManageThreadBase::rbl_IsExistThrdPrvDataHandle()
{
#ifndef  LINUXCODE
	return	TLS_OUT_OF_INDEXES != m_ThrdPrvDataHandle ;
#else
	return m_blCrtThrdPrvDataHndl	;
#endif
}


//�߳�˽�����ݾ��
#ifndef  LINUXCODE
DWORD		CManageThreadBase::GetThrdPrvDataHandle() 
#else
pthread_key_t	CManageThreadBase::GetThrdPrvDataHandle()
#endif
{
	return	m_ThrdPrvDataHandle;	
};

//���õ�ǰ�߳��ļ��к�
void	CManageThreadBase::SetCurThrdSrcFileLine(char * in_pszSrcFileName, unsigned short	in_ushSrcFileLine)
{
	CThreadBase * poThread = FindCurrentThread();
	if(NULL != poThread)
		poThread->SetSrcFileLineName(in_pszSrcFileName, in_ushSrcFileLine );
}


//���ҵ�ǰ�̶߳���
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
	//��ѯ���нڵ�
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

//��ָ�����߳̽��й���
bool	CManageThreadBase::SetSuspFrmTheadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
CThreadBase::eRunState	CManageThreadBase::GetRunStateFrmThreadID(unsigned	long in_ulThreadID)
{
	CThreadBase::eRunState	eRet = CThreadBase::RS_None;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ�����߳�[%u]", in_ulThreadID);


	return eRet;

}	
//ָ�����߳��Ƿ���������
bool		CManageThreadBase::IsRunFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ�����߳�[%u]", in_ulThreadID);


	return blRet;

}	
//ָ�����߳��Ƿ񱻹���
bool		CManageThreadBase::IsSuspFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ�����߳�[%u]", in_ulThreadID);


	return blRet;

}
//��ָ�����߳̽��м���
bool	CManageThreadBase::ActiveFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ���ļ����߳�[%u]", in_ulThreadID);


	return blRet;
	
}
//��ָ�����߳����ƽ��й���,�������ƿ��Բ���
bool	CManageThreadBase::SetSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û����������߳�����");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�[%s]��ָ���Ĺ����߳�[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ���Ĺ����߳�[%s]", in_pszThreadName);
	}

	return blRet;
}
//��ָ�����߳����ƽ��м���,�������ƿ��Բ���
bool	CManageThreadBase::ActiveFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�����");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�[%s]��ָ���ļ����߳�[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ�ָ���ļ����߳�[%s]", in_pszThreadName);
	}

	return blRet;
	
}

//ָ�����߳������Ƿ���������
bool		CManageThreadBase::IsRunFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�����");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
	//ָ�����߳������Ƿ񶼱�����
bool		CManageThreadBase::IsSuspFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�����");
		return true;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//��ָ���������Ƶ��߳̽��й���
bool	CManageThreadBase::SetSuspFrmManagerName( const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen =0;
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û����������߳�Ӧ������");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ������߳�Ӧ������[%s]", in_pszManagerName);
	}

	return blRet;
}
//��ָ���������Ƶ��߳̽��м���
bool	CManageThreadBase::ActiveFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�Ӧ������");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û���ҵ������߳�Ӧ������[%s]", in_pszManagerName);
	}

	return blRet;
	
}
//ָ�����߳������Ƿ���������
bool		CManageThreadBase::IsRunFrmManagerName(const char * in_pszManagerName )
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�Ӧ������");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//ָ�����߳������Ƿ񶼱�����
bool		CManageThreadBase::IsSuspFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = true;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "û�����뼤���߳�Ӧ������");
		return blRet;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
 * ��������rl_GetThreadCount
 * ���ܣ�ȡ�̸߳���
 * ������in_strName	����-��ѯ���߳����ƣ�������ǰ��������ͬ��
 *		out_plActivateCnt	���-������߳�����(��ֹͣ��û��ʹ�õ�)
 *		out_plProcessingCount	���-��ǰ����ִ�д�����̸߳���
 *		out_plSuspendCnt	���-�һ��̸߳���
 *		out_plOuttimeCnt	���-��ʱ�߳���
 * ���أ��̸߳���
 *****************************/
long		CManageThreadBase::rl_GetThreadCount(const char * in_strName, long * out_plActivateCnt, long * out_plProcessingCount, 
										   long * out_plSuspendCnt, long * out_plOuttimeCnt)
{
	TLinkNode	*		pstThreadNode;
	long lTotalActivate = 0, lTotalProcessing = 0, lTotalSuspend = 0, lTotalTimeout = 0, lRet = 0;
	long	lFindNameLen = 0;
	
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		if(NULL == pstThreadNode->poThread)
		{
			continue;
			
		}
		//����������ѯʱ��ֻ��ѯǰ������ͬ���ƵĴ�ӡ
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
//��ӡ�����߳���Ϣ
void CManageThreadBase::PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction)
{
	PrintfInfoFrmName(in_oCustomSelfPRT, fpFunction, NULL, NULL);
}


//��ӡָ��Ӧ���߳���Ϣ
void CManageThreadBase::PrintfInfoFrmManageName(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction, const char * in_pszManagerName)
{
	PrintfInfoFrmName(in_oCustomSelfPRT, fpFunction, NULL, in_pszManagerName);
	
}
//��ӡָ���߳���Ϣ
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
	//add by liuqy 20110225 ��ӡ��ʱʱ��
	long lTime;
	long lTotalUnuse = 0, lTotalStop = 0, lTotalSuspend = 0, lTotalTimeout = 0, lTotalRun=0;
	
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);

	if(NULL != in_pszManagerName)
		lFindManagerName = strlen(in_pszManagerName);

	memset(tempbuf, 0, sizeof(tempbuf));
	
	try{
//		my_snprintf(tempbuf, 256, "|  �߳�ID        �߳�����         ��ʼʱ��     ״̬  ѭ������  �������� ������    ͳ��ʱ��     ����ʱ��  ������");
		my_snprintf(tempbuf, sizeof(tempbuf)-1, "| �߳�ID       �߳�����         ��ʼʱ��       ״̬   ������ ʱ���� ����");

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
	//��ѯ���нڵ�
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
			strcpy(szWorkState, "δ��");
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
					sprintf(szWorkState, "ͣ%d��", lTime);
				else
				if(lTime < 3600)
					sprintf(szWorkState, "ͣ%d��%d", lTime/60, lTime%60);
				else
				{
					lTime /= 60;
					sprintf(szWorkState, "ͣ%dʱ%d", lTime/60, lTime%60);
				}
				lTotalStop ++;
			}
			else
				if(pstThreadNode->poThread->m_blEntrySuspend)
				{
					strcpy(szWorkState, "����");
					lTotalSuspend++;
				}
				else
				{
					//modify by liuqy 20101026 ����ʱʱ���޸��˳���15����㳬ʱ
					if(15000 <= pstThreadNode->poThread->m_mRefreshTime.GetDuration())
					{
						memset(szWorkState, 0, sizeof(szWorkState));
						//add by liuqy 20110225 ��ӡ��ʱʱ��
						lTime = pstThreadNode->poThread->m_mRefreshTime.GetDuration();
						lTime /= 1000;
						if(lTime < 60)
							sprintf(szWorkState, "��%d��", lTime);
						else
						if(lTime < 3600)
							sprintf(szWorkState, "��%d��%d", lTime/60, lTime%60);
						else
						{
							lTime /= 60;
							sprintf(szWorkState, "��%dʱ%d��", lTime/60, lTime%60);
						}


						lTotalTimeout++;
					}
					else
					{
						strcpy(szWorkState, "����");
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
					"δͳ��", 		"δͳ��", pstThreadNode->poThread->m_lAddtionData,
				 pstThreadNode->poThread->m_szAddtionData);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10s %8s %6s %14s %10s %6s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_szObjectName, szDateTime, szWorkState,
				"δͳ��", 		"δͳ��", 				"δͳ��", 
				"δͳ��", 		"δͳ��", 		"δͳ��");
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
		my_snprintf(tempbuf, 256, "|  ���߳���       ����     ��ʱ��    ������     ͣ��      δ��");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		my_snprintf(tempbuf, 256, "|  %8d       %4d      %4d      %4d      %4d      %4d", 
			lTotalUnuse + lTotalStop + lTotalSuspend + lTotalTimeout + lTotalRun, 
			lTotalRun, lTotalTimeout, lTotalSuspend, lTotalStop, lTotalUnuse			);
		fpFunction(in_oCustomSelfPRT, tempbuf);

}

//��ӡ��ʱ�߳���Ϣ
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
	//��ѯ���нڵ�
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
					sprintf(szTime, "%d��", lTime);
				else
				if(lTime < 3600)
					sprintf(szTime, "%d��%d��", lTime/60, lTime%60);
				else
				{
					lTime /= 60;
					sprintf(szTime, "%dʱ%d��", lTime/60, lTime%60);
				}
				my_snprintf(tempbuf, sizeof(tempbuf)-1,"<%s>�߳�[%s]ID[%u]ֹͣ��Ӧ%s[��������%d]", 
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


//�������е��߳�
void		CManageThreadBase::ActivateAll()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//���е��̷߳���������
bool	CManageThreadBase::IsRunAll()
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//�����Լ��������߳��Ƿ񶼱�����
bool		CManageThreadBase::IsSuspUnselfAll()
{
	bool	blRet = true;
	unsigned long		ulThreadID = GetCurrentThreadId();
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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

//�����Լ������й����߳�
void		CManageThreadBase::SuspendUnselfAll()
{
	TLinkNode	*		pstThreadNode;
	unsigned long		ulThreadID = GetCurrentThreadId();
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//�������е��߳�
void		CManageThreadBase::SuspendAll()
{
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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

//ȡע���̵߳ĸ���
long	CManageThreadBase::GetCount()
{
	long lRet = 0;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//ȡָ��λ�õ��߳�ID
unsigned long CManageThreadBase::GetThreadID(unsigned long in_ulPosID)
{
	unsigned long lRet = 0;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//��ѯ���нڵ�
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
//ȡָ��λ�õ��߳�����
const char *  CManageThreadBase::GetThreadNameFrmPos(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//��ѯ���нڵ�
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
//ȡָ���̵߳��߳�����
const char *  CManageThreadBase::GetThreadNameFrmThrdID(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
//ȡָ��λ�õ��߳�����
const char *  CManageThreadBase::GetThreadAppNameFrmPos(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	long lPos;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	lPos = 0;
	//��ѯ���нڵ�
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
//ȡָ���̵߳��߳�����
const char *  CManageThreadBase::GetThreadAppNameFrmThrdID(unsigned long in_ulPosID)
{
	const char * pszRet = NULL;
	TLinkNode	*		pstThreadNode;
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
	//��ѯ���нڵ�
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
//��ָ���������Ƶ��̹߳ر�,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
bool		CManageThreadBase::StopFrmManagerName(const char * in_pszManagerName)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen =0;
	if(NULL != in_pszManagerName)
		lFindNameLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱ�����߳�Ӧ������");
		return false;
	}
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱû���ҵ��߳�Ӧ������[%s]", in_pszManagerName);
	}

	return blRet;
}
//��ָ���߳����Ƶ��̹߳ر�,ע�����ƽ�ʹ��like��ʽ����ǰƥ��
bool		CManageThreadBase::StopFrmThreadName(const char * in_pszThreadName, const char * in_pszManagerName )
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	long	lFindNameLen = 0, lFindManagerNameLen =0;
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszThreadName)
		lFindNameLen = strlen(in_pszThreadName);
	//û���������֣����˳�
	if(0 >= lFindNameLen )
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱ�����߳�����");
		return false;
	}
	if(NULL != in_pszManagerName)
		lFindManagerNameLen = strlen(in_pszManagerName);
	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱû���ҵ�[%s]��ָ�����߳�[%s]", in_pszManagerName, in_pszThreadName);
		else
			my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱû���ҵ�ָ�����߳�[%s]", in_pszThreadName);
	}
	return blRet;
}
//��ָ�����߳̽��йر�
bool		CManageThreadBase::StopFrmThreadID(unsigned	long in_ulThreadID)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ֹͣʱû���ҵ����߳�[%u]", in_ulThreadID);

	return blRet;
}
//ɱ�����Լ�֮��������߳�
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
	//��ѯ���нڵ�
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

//����ǰ�߳�ע�ᵽ������
int		CManageThreadBase::Register(const char * in_pszManagerName, CThreadBase * in_poThread)
{
	int	lRet = -1;
	TLinkNode	*		pstThreadNode;
	long	lLen = 0;
	long lCmp;
	TLinkNode	*		pstAddThreadNode;
	char	szName[sizeof(pstAddThreadNode->szName)] ;

	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_pszManagerName)
		lLen = strlen(in_pszManagerName);
	//û���������֣����˳�
	if(0 >= lLen || NULL == in_poThread)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ע��ʱû�������߳�Ӧ�����ƻ��̶߳���ΪNULL");
		return -2;
	}
	lCmp = strlen(in_poThread->m_szObjectName);
	if(0 == lCmp)
	{
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ע��ʱ�����̶߳�������Ϊ��");
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
	//��ѯ���нڵ�
	Mylist_for_each(pNode, &m_stHeadNode)
	{
		pstThreadNode = Mylist_entry(pNode, TLinkNode, stNode);
		lCmp = strcmp(szName, pstThreadNode->szName);
		if(0 > lCmp)
			break;
		pAddNodePos = pNode;
		if(0 == lCmp && in_poThread == pstThreadNode->poThread )
		{
			//�ظ����ӣ�������
			oLocalSession.UnAttch();
			return 0;
		}

	}
#ifndef	_LINUXTRYOFF
	try{
#endif
#ifdef	LINUXCODE
		pstAddThreadNode = new TLinkNode;
#else	//windows �ڴ����
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ע��ʱ�����̹߳���ڵ�ʧ��[%d]", lRet);
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
//�ӹ�������ע��ָ���߳�
bool		CManageThreadBase::Unregister(CThreadBase * in_poThread)
{
	bool	blRet = false;
	TLinkNode	*		pstThreadNode;
	//��ǰָ���Ѿ��������ˣ���ֱ���˳�
	if(NULL == this)
		return true;

	MLocalSection oLocalSession;
	oLocalSession.Attch(&m_oMutex);
	TMyListNode * pNode = NULL;
	//��ѯ���нڵ�
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
		my_snprintf(m_szErrMsg, sizeof(m_szErrMsg)-1, "ע��ʱû���ҵ���ǰ���߳�");

	return blRet;

}
