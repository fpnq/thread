//------------------------------------------------------------------------------------------------------------------------------
#include "MThread.h"
//------------------------------------------------------------------------------------------------------------------------------
//add by liuqy 20100817 for �̸߳��죬��ҪΪͳ�ƴ�ӡ�߳���Ϣ����һ������
bool							MThreadBase::s_bStopAllThread = false;
MThreadBase					*	MThreadBase::s_poFirstThread = NULL;
bool							MThreadBase::s_blStatisticsRate = false;	//�߳�������ͳ�Ʊ�־
#ifndef LINUXCODE
CRITICAL_SECTION					MThreadBase::s_stSection;
#else
pthread_mutex_t						MThreadBase::s_stSection;
#endif
bool				MThreadBase::s_blSectionFlag = false;

//�����ٽ���
void MThreadBase::rv_CreateSection()
{
	if(s_blSectionFlag)
		return;
	#ifndef LINUXCODE
		::InitializeCriticalSection(&s_stSection);
	#else	
		/*
		 *	GUOGUO 2009-08-11 �����ٽ�����������Ϊ�ɵݹ���,֧��ͬһ�߳���������lock
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
//ɾ���ٽ���
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
//����
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
//����
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
	//��������뵽����
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
	//�������������ɾ��
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
	m_blSuspend = false;	//�̹߳���
	m_blEntrySuspend = true;	//�߳̽������״̬
	m_blBeginWorkFlag = false;	//�߳̿�ʼ������־
	m_blEntryWorkStatis = false;	//�߳�һ������ͳ��״̬��ʼ����Ҫ����ֹͣ״̬���������ֹͣ״̬
	m_stBeginWorkTime = 0;	//�߳̿�ʼ����ʱ�䣬��1970����������
	m_ulThreadID =0;	//�߳�ID 
	m_uiLoopCount = 0 ;	//�߳�ѭ������
	m_uiWorkCount = 0;	//��������
	m_uiRealWorkUSECCount = 0;	//�߳�ʵ�ʹ���ʱ��ͳ��(΢��)
	m_blProcessingFlag = false;	//���ڴ����־ add by liuqy 20101020 
	//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	m_oWaitEvent.Active();

}
//ˢ�¼�飬��Ҫ�����߳̽������״̬,������̵߳�һ�ν��У�����Ҫ���ÿ�ʼʱ��
void MThreadBase::Refresh(void)
{

	//��һ�ν��룬���߳̿�ʼ������
	if(!m_blBeginWorkFlag)
	{
		m_blBeginWorkFlag = true;
		//ȡ�߳�ID
		//Ϊ��ȫ���ڴ˴���ȡ�߳�ID�ţ��ɽ����̴߳�ȡ�ã���Ϊ���ô˺����Ŀ��ܲ������߳��У�
		//m_ulThreadID = GetCurrentThreadId();
		time(&m_stBeginWorkTime);
	}
	//��Ҫͳ���߳�ʹ����ʱ������ѭ��������û�н���ʱ��ͳ�Ƶģ�ȥ������
	if(s_blStatisticsRate)
	{
		m_uiLoopCount++;
		rv_FinishWork();
	}
	MCheckTimeOutObjectList::Refresh();
	//�߳���Ҫ����
	while(m_blSuspend)
	{
		m_blEntrySuspend = true;
		if ( s_bStopAllThread || m_bStopCurThread )
		{
			m_blSuspend = false;
			break;
		}
		//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
		inner_stopactive();
		m_oWaitEvent.Wait();
//		m_oWaitEvent.Wait(3000);
		inner_setactive();
		//MThread::Sleep(200);

	}
	m_blEntrySuspend = false;
}
//��ʼ�߳�������ͳ��
void	MThreadBase::rv_StartStatisticsRate()
{
	MThreadBase				*	poThreadBase;

	if(s_blStatisticsRate)
		return;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//ѭ������ÿ���߳�
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
//ͨ�������н�ָ�����߳̽��й���
bool	MThreadBase::rbl_SetSuspFrmTheadID(unsigned	long in_ulThreadID)
{
	MThreadBase				*	poThreadBase;
	bool	blRet = false;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//ѭ������ÿ���߳�
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
//ͨ�������н�ָ�����߳̽��й���
bool	MThreadBase::rbl_ActSuspFrmThreadID(unsigned	long in_ulThreadID)
{
	MThreadBase				*	poThreadBase;
	bool	blRet = false;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//ѭ������ÿ���߳�
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
//��ӡ�����߳���Ϣ
void MThreadBase::rv_PrintfInfo(void * in_oCustomSelfPRT, DF_rv_PrintReportCallBack * fpFunction)
{
	rv_PrintfInfo(in_oCustomSelfPRT, NULL, fpFunction);
}
//add by liuqy 20100830 for Ϊ��֤������ݵ���ȷ�ԣ���Ҫ�ó�CPU����Ȩ
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
long		MThreadBase::rl_GetThreadCount(const char * in_strName, long * out_plActivateCnt, long * out_plProcessingCount, 
										   long * out_plSuspendCnt, long * out_plOuttimeCnt)
{
	MThreadBase				*	poThreadBase;
	long lTotalActivate = 0, lTotalProcessing = 0, lTotalSuspend = 0, lTotalTimeout = 0, lRet = 0;
	long	lFindNameLen = 0;
	
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	rv_LockSection();
	poThreadBase = s_poFirstThread;

	while (NULL != poThreadBase)
	{
		//����������ѯʱ��ֻ��ѯǰ������ͬ���ƵĴ�ӡ
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
					//modify by liuqy 20101026 ����ʱʱ���޸��˳���15����㳬ʱ
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

//��ӡָ���߳���Ϣ
//modify by liuqy 20101210 for ����ӡ�İٷֱ���߱���С����
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
	//add by liuqy 20110225 ��ӡ��ʱʱ��
	long lTime;
	long lTotalUnuse = 0, lTotalStop = 0, lTotalSuspend = 0, lTotalTimeout = 0, lTotalRun=0;
	
	//���ڲ�ѯĳЩ�߳���
	if(NULL != in_strName)
		lFindNameLen = strlen(in_strName);

	memset(tempbuf, 0, sizeof(tempbuf));
	
#ifndef	_LINUXTRYOFF
	try
	{
#endif
//		my_snprintf(tempbuf, 256, "|  �߳�ID        �߳�����         ��ʼʱ��     ״̬  ѭ������  �������� ������    ͳ��ʱ��     ����ʱ��  ������");
		//modify by liuqy 20100926 for ��ӡ�̵߳ĸ�������
		my_snprintf(tempbuf, 256, "| �߳�ID       �߳�����         ��ʼʱ��     ״̬  ����");

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
		//����������ѯʱ��ֻ��ѯǰ������ͬ���ƵĴ�ӡ
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
			strcpy(szWorkState, "δ��");
			lTotalUnuse++;
		}
		else
		{
			if(!poThreadBase->rbl_GetRunState())
			{
				//add by liuqy 20110225 ��ӡ��ʱʱ��
				memset(szWorkState, 0, sizeof(szWorkState));
				lTime = poThreadBase->m_mRefreshTime.GetDuration();
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
			{
				if(poThreadBase->m_blEntrySuspend)
				{
					strcpy(szWorkState, "����");
					lTotalSuspend++;
				}
				else
				{
					//modify by liuqy 20101026 ����ʱʱ���޸��˳���15����㳬ʱ
					if(15000 <= poThreadBase->m_mRefreshTime.GetDuration())
					{
						memset(szWorkState, 0, sizeof(szWorkState));
						//add by liuqy 20110225 ��ӡ��ʱʱ��
						lTime = poThreadBase->m_mRefreshTime.GetDuration();
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
			//add by liuqy 20100926 for ��ӡ�̵߳ĸ�������
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
			//add by liuqy 20100926 for ��ӡ�̵߳ĸ�������
			my_snprintf(tempbuf,256,"|%8u %-18s %-14s %-7s%4d %s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
			//		"δͳ��", 
			//		"δͳ��", 
					poThreadBase->m_lAddtionData, szSrcFileLine);
			/*
			my_snprintf(tempbuf,256,"| %8u  %18s %15s %4s %10s %8s %6s %14s %10s %6s", 
				poThreadBase->m_ulThreadID, poThreadBase->m_strObjectName, szDateTime, szWorkState,
				"δͳ��", 		"δͳ��", 				"δͳ��", 
				"δͳ��", 		"δͳ��", 		"δͳ��");
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
		my_snprintf(tempbuf, 256, "|  ���߳���       ����     ��ʱ��    ������     ͣ��      δ��");
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();
		my_snprintf(tempbuf, 256, "|  %8d       %4d      %4d      %4d      %4d      %4d", 
			lTotalUnuse + lTotalStop + lTotalSuspend + lTotalTimeout + lTotalRun, 
			lTotalRun, lTotalTimeout, lTotalSuspend, lTotalStop, lTotalUnuse			);
		fpFunction(in_oCustomSelfPRT, tempbuf);
		rv_CpuYield();

}
//add by liuqy 20110505 for �������е��߳�
void		MThreadBase::rv_ActivateAll()
{
	//add by liuqy 20110505 for ֹͣ���е��̣߳�����Ҫȥ�������е��߳�
	MThreadBase				*	poThreadBase;
	rv_LockSection();
	poThreadBase = s_poFirstThread;
	//ѭ������ÿ���߳�
	while (NULL != poThreadBase)
	{
		poThreadBase->rv_ActivateFromSuspend();
		poThreadBase = poThreadBase->m_mNextThread;
	}
	rv_UnlockSection();

}

//���빤����...����Ҫ����ͳ���߳�������ʱʹ�á��������ʱ����Ҫ����rv_FinishWork
void	MThreadBase::rv_EntryWorking()
{
	m_blProcessingFlag = true;	//���ڴ����־ add by liuqy 20101020 
	if(!s_blStatisticsRate)
		return;
	rv_FinishWork();
	m_uiWorkCount++;
	m_oWorkStatisUsecCounter.SetCurTickCount();
	m_blEntryWorkStatis = true;
}
//��������������빤����(rv_EntryWorking)���ʹ��
void	MThreadBase::rv_FinishWork()
{
	m_blProcessingFlag = false;	//���ڴ����־ add by liuqy 20101020 
	if(!s_blStatisticsRate || !m_blEntryWorkStatis)
		return;
	m_uiRealWorkUSECCount += m_oWorkStatisUsecCounter.GetI64Duration();
	m_blEntryWorkStatis = false;
}
//ȡ��ǰ�߳��Ƿ�������״̬���̹߳���Ҳ������״̬��
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

			iRet = pthread_kill(m_hRecordData, 0);	//	����0�ź�,��ͨ������������߳��Ƿ����
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
	//add by liuqy 20100817 for �����߳�ʱ����Ҫ��λϵͳ����
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
		//modify by liuqy 20100817 for �����߳�ID��
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
		 *	LINUX pthread_create�������̵߳Ķ�ջ��10MB
		 *	���ǵĳ���ͨ���ò�����ô��,���Ե���Ϊ2MB
		 *	�������Լ����߼���ַ
		 */
		pthread_attr_t thread_attr;
		size_t stack_size;

		if(pthread_attr_init(&thread_attr) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		//�����߳�ջ�ռ�Ϊ2M.
		stack_size = 2048*1024;
		if(pthread_attr_setstacksize(&thread_attr, stack_size) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		if ( pthread_create(&m_hRecordData,&thread_attr,fpFunction,lpParam) != 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		//add by liuqy 20100817 for �����߳�ID��
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
	//delete by liuqy 20110519 for ֻ�������ȥ����m_bLocked,���������ʱ�������
	//	inner_stopactive(); // �߳�ֹͣ��m_bLocked ��Ϊfalse, ����ͻ��ӡ�ܶ���̳߳�ʱ��modify by anzl 20110408
	m_bStopCurThread = true;
	//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
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
			 *	�ȴ��߳̽���,�����������Ƶȴ�,��ʵ���������,
			 *	������֪����2���߳��ǲ�����Ϊ������s_bStopAllThread���˳���,
			 *	һ����LINUX��ģ��IOCP�߳�----����detach����,���ⲻ��
			 *	������servicemanager��_tagCtrlTerminalStruct::threadfunc( void * Param ) ָ����߳�,ҲΪLINUX��������detach����
			 *	���е����ⶼ���,���ᵼ��ptheead_join����
			 *	
			 *	�����������,ֹͣ�����߳�,��2�ֿ���,
			 *	1.�̲߳��ж�s_bStopAllThread����m_bStopCurThread������
			 *	2.�߳����ж��˳���־д������.��Ȼ�Ҿ���������Ŀ�����̫С��.
			 *	WINDOWS ��WaitForSingleObject�����˳�ʱ�ȴ�,
			 *	�ǳ����ҵ�����LINUX����pthread_joinû�г�ʱ�ȴ��Ĵ���
			 *	����Ĳ�ȡ��,POSIX��pthread_kill����,����0�źż���̵߳Ĵ�����
			 *	���ﵽ��ʱ�Ĵ���ʽ.
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
					//	�̻߳�����
					this->rv_CpuYield();
				}
				else if(iRet == ESRCH)
				{
#ifdef _DEBUG
//	printf("______________________Thread(%08X)(%ld) already exit\n", m_hRecordData, lWaitTime);fflush(stdout);
#endif
					//	�߳��Ѿ������˶���.
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
				//	��ʱ��
				//pthread_cancel(m_hRecordData);			//	KILL
				pthread_kill(m_hRecordData, 9);			//	anzl 20170728
			}

			m_hRecordData = 0;
		}
	#endif
	//add by liuqy 20110225 ֹͣʱ�����ܴ�ӡ��ʱ��Ӧ����Ҫ����־����
	inner_stopactive();
}
//..............................................................................................................................
bool MThread::GetThreadStopFlag(void)
{
	Refresh();

	if ( s_bStopAllThread == true || m_bStopCurThread == true )
	{
		//add by liuqy 20110225 ֹͣʱ�����ܴ�ӡ��ʱ��Ӧ����Ҫ����־����
		inner_stopactive();
		return(true);
	}
	else
	{
		return(false);
	}
}
//add by liuqy 20110323 for ��ȡֹͣ��־��ֱ�ӷ��ر�־���������κβ���
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
// ֹͣ���̣߳����ñ��̱߳�־��
void MThread::StopCurThread(void)
{
	m_bStopCurThread = true;
	//add by liuqy 20110505 for ���̹߳���ʱ��ֹͣ��ת�����߳�ȥ�ȴ�һ���¼�
	m_oWaitEvent.Active();

}

//..............................................................................................................................
void MThread::StopAllThread(void)
{
	s_bStopAllThread = true;
	//add by liuqy 20110505 for ֹͣ���е��̣߳�����Ҫȥ�������е��߳�
	rv_ActivateAll();

}

//..............................................................................................................................
void MThread::Sleep(unsigned long lMSec)
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

		//				����(GUOGUO)
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
		 *	LINUX pthread_create�������̵߳Ķ�ջ��10MB
		 *	���ǵĳ���ͨ���ò�����ô��,���Ե���Ϊ2MB
		 *	�������Լ����߼���ַ
		 */
		pthread_t		handle;
		pthread_attr_t 	thread_attr;

		if(pthread_attr_init(&thread_attr) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		//�����߳�ջ�ռ�Ϊ2M.
		stack_size = 2048*1024;
		if(pthread_attr_setstacksize(&thread_attr, stack_size) != 0)
		{
			return(MErrorCode::GetSysErr());
		}

		if ( pthread_create(&handle, &thread_attr, fpFunction, lpParam ) != 0 )
		{
			return(MErrorCode::GetSysErr());
		}

		//add by liuqy 20100817 for �����߳�ID��
		
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
