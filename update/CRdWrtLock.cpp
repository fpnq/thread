/***********************************************************************
 * �ļ����ƣ�CRdWrtLock.cpp
 * ����������ʵ�ֶ�д������
 * ������ʷ��liuqy 20110111 cd
 * ��д��ʵ�����£����ͷ�ļ�
 *********************************************************************/

#include "CRdWrtLock.h"
#include "../BaseUnit/MThread.h"

extern const char * rpsz_RemovePathFileName(const char*);

//��ʼ��һ���ڵ�
void rv_InitListNode(TMyListNode * in_pstNode)
{
	memset(in_pstNode, 0, sizeof(TMyListNode));
}
//��ͷ������һ���ڵ�
void rv_AddListNode(TMyListNode * in_pstHead, TMyListNode * in_pstAddNode)
{
	if(NULL != in_pstHead->m_pstNext)
		in_pstHead->m_pstNext->m_pstPrevious = in_pstAddNode;
	in_pstAddNode->m_pstNext = in_pstHead->m_pstNext;
	in_pstAddNode->m_pstPrevious = in_pstHead;
	in_pstHead->m_pstNext = in_pstAddNode;
}
//�ӽڵ���ɾ��
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
       	�� �� ��: rl_WriteLock
       	��������: д������
       	��    ��: in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       	����˵��: 0�ɹ� 1�����ɹ�	 -1ʧ��
********************************************************/
long CLocalRdWrtLockSession::rl_WriteLock(long in_lTimeOut)
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��ж�����");
		return -1;
	}

	return m_poRdWrtLockRsc->rl_WriteLock(this, in_lTimeOut);

}
long CLocalRdWrtLockSession::rl_WriteLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo)
{
	long lRet;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��ж�����");
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
       	�� �� ��: rl_ReadLock
        ��������: ��������,ע�������ǰ�й�д��û�н�������ϵͳֱ�ӷ��سɹ�
       	��    ��: in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       	����˵��: 0�ɹ�	 1�����ɹ� -1ʧ��
********************************************************/
long CLocalRdWrtLockSession::rl_ReadLock(long in_lTimeOut)
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��ж�����");
		return -1;
	}
	
	return m_poRdWrtLockRsc->rl_ReadLock(this, in_lTimeOut);
	
}
long CLocalRdWrtLockSession::rl_ReadLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo)
{
	int lRet;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��ж�����");
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
       	�� �� ��: rl_UnlockWrtKeepRd
       	��������: ��д������������
       	��    ��: ��
       	����˵��: 0�ɹ�	 -1ʧ��
********************************************************/
long CLocalRdWrtLockSession::rl_UnlockWrtKeepRd()
{
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��н���");
		return -1;
	}

	return m_poRdWrtLockRsc->rl_UnlockWrtKeepRd(this);
}
/*******************************************************
	  �� �� ��: rl_Unlock
	   	��������: �����е���
	   	��    ��: ��
	    ����˵��: 0�ɹ�	 -1ʧ��
********************************************************/
long CLocalRdWrtLockSession::rl_Unlock( )
{
	long lRet1 = 0, lRet2 = 0;
	if(NULL == m_poRdWrtLockRsc)
	{
		sprintf(m_szErrMsg, "û�ж�д����Դ�����ܽ��н���");
		return -1;
	}
	//��д��������Ҫȥ��д��
	if(m_blWriteLockFlag)
	{
		lRet1 = m_poRdWrtLockRsc->rl_UnlockWrt(this);
	}
	//�ж���������Ҫȥ���д
	if(m_blReadLockFlag)
	{
		lRet2 = m_poRdWrtLockRsc->rl_UnlockRd(this);
	}
	if(0 > lRet1)
		return lRet1;
	return lRet2;
}

/*******************************************************
       	�� �� ��: re_GetLockState
       	��������: ȡ��ǰ�������
       	��    ��: ��
       	����˵��: Noneδ������Read�Ѿ��ӹ�������Write�Ѿ��ӹ�д��
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
	m_blReadLockFlag = false;	//���������
	m_blWriteLockFlag = false;	//д�������
	m_poRdWrtLockRsc = NULL;	//��д����Դ
	memset(&m_szErrMsg[0], 0, sizeof(m_szErrMsg));	//������Ϣ
	
}
//���ö�д����Դ
void CLocalRdWrtLockSession::SetLockRsc(CRdWrtLockResource & in_oRdWrtLockRsc)
{
	rl_Unlock();
	m_poRdWrtLockRsc = &in_oRdWrtLockRsc;	//��д����Դ
	
}

CLocalRdWrtLockSession::CLocalRdWrtLockSession(CRdWrtLockResource * in_poRdWrtLockRsc)
{
	memset(&m_stReadNode, 0, sizeof(m_stReadNode));
	m_blReadLockFlag = false;	//���������
	m_blWriteLockFlag = false;	//д�������
	m_poRdWrtLockRsc = in_poRdWrtLockRsc;	//��д����Դ
	memset(&m_szErrMsg[0], 0, sizeof(m_szErrMsg));	//������Ϣ
	
}

CLocalRdWrtLockSession::~CLocalRdWrtLockSession()
{
	rl_Unlock();
}


/*******************************************************
       	�� �� ��: rl_WriteLock
       	��������: д������
       	��    ��: in_poRdWrtSession	����--��д��������
				in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       	����˵��: 0�ɹ� 	 -1ʧ��
********************************************************/
long CRdWrtLockResource::rl_WriteLock(CLocalRdWrtLockSession * in_poRdWrtSession,  long in_lTimeOut)
{
	long lNowTime,lStartTime;
	long lWaitTime;
	unsigned long	ulThreadID;	//��ǰ��ID
	TMyListNode * pNode;
	CLocalRdWrtLockSession::TLockNode	* pstLockNode;
	unsigned long ulCount;
	long lRet = -1;
	
	ulThreadID = GetCurrentThreadId();
	//��ǰ��д���Ѿ����Լ����ˣ�ֱ�ӷ��سɹ�
	if(ulThreadID == m_ulWrtThreadID)
		return 0;

	if(in_poRdWrtSession->m_blWriteLockFlag )
	{
		sprintf(in_poRdWrtSession->m_szErrMsg, "�Ѿ�����д����������д��");				
		return -1;
	}

	time(&lStartTime);
#ifndef LINUXCODE
	//�ȼ�д��
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
		sprintf(in_poRdWrtSession->m_szErrMsg, "��ǰ�߳����ظ������д�����");				
		return -1;
	}
		//��д���ɹ��󣬹ر����ж�������ͬʱ�����Լ��ڵ�ĸ���
#ifndef LINUXCODE
	::EnterCriticalSection(&m_stReaderSection);
#else
	pthread_mutex_lock(&m_stReaderSection);
#endif

		m_blAllowReadFlag = false;
		ulCount = 0;
		//��ѯ���нڵ�
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
			sprintf(in_poRdWrtSession->m_szErrMsg, "��ǰ�߳��Ѽӹ�������д�������ܻ�����");				
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
				sprintf(in_poRdWrtSession->m_szErrMsg, "�ȴ�д����ʱ");				
				lRet = -1;
				goto WriteLockErr_Label;
			}
			if(! m_oWrtWaitEvent.Wait((in_lTimeOut - lWaitTime) * 1000))
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "�ȴ�д�����ִ���");				
				lRet = -2;
				goto WriteLockErr_Label;
			}
			
		} while(1);

	in_poRdWrtSession->m_blWriteLockFlag = true;
	m_ulWrtThreadID = ulThreadID;	//��ǰд���߳�ID��

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
       	�� �� ��: rl_ReadLock
        ��������: ��������
       	��    ��: in_poRdWrtSession	����--��д��������
       			in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       	����˵��: 0�ɹ�	 -1ʧ��
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
			//����������Լ��Ѿ��ӹ�д�������ʾ���ԼӶ���
			if(m_blAllowReadFlag || in_poRdWrtSession->m_stReadNode.m_ulThreadID == m_ulWrtThreadID)
			{
				//�Լ��������־����������û�ȥ
				if( blResetFlag && m_blAllowReadFlag)
					m_oRdWaitEvent.Active();
				break;
			}
			//��ѯ�Լ��߳��Ƿ����Ѿ������ڶ������У�����ǣ��ж���Ƕ�ף���ʾ��д�������������д����ʱ����
			if(blFirstFlag)
			{
				TMyListNode * pNode;
				CLocalRdWrtLockSession::TLockNode	* pstLockNode;
				//��ѯ���нڵ�
				Mylist_for_each(pNode, &m_stReaderHeadNode)
				{
					pstLockNode = Mylist_entry(pNode, CLocalRdWrtLockSession::TLockNode, m_stNode);
					if(in_poRdWrtSession->m_stReadNode.m_ulThreadID == pstLockNode->m_ulThreadID)
					{
						//�Ѿ��ӹ���������Ӷ���ֱ�ӳɹ�
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
				sprintf(in_poRdWrtSession->m_szErrMsg, "�ȴ�������ʱ");				
				return -1;
			}
			if(! m_oRdWaitEvent.Wait((in_lTimeOut - lWaitTime) * 1000))
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "�ȴ��������ִ���");				
				return  -2;			
			}
			
		} while(1);
		//���û�мӹ�����������Ҫ���Լ���������
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
       	�� �� ��: rl_UnlockWrtKeepRd
       	��������: ��д������������
       	��    ��: in_poRdWrtSession	����--��д��������
       	����˵��: 0�ɹ�	 -1ʧ��
********************************************************/
long CRdWrtLockResource::rl_UnlockWrtKeepRd(CLocalRdWrtLockSession * in_poRdWrtSession)
{
	long lRet;
	//���û�мӹ�����������Ҫ���Լ�����
	if(!in_poRdWrtSession->m_blReadLockFlag)
	{
		//����дʱ��ֱ�����Ӷ�����
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
		sprintf(in_poRdWrtSession->m_szErrMsg, "���ֶ���������Ϊ����û�мӹ���");
		return -1;

	}
	return 0;

}
/*******************************************************
	  �� �� ��: rl_UnlockRd
	   	��������: �����
	   	��    ��: in_poRdWrtSession	����--��д��������
	    ����˵��: 0�ɹ�	 -1ʧ��
********************************************************/
long CRdWrtLockResource::rl_UnlockRd(CLocalRdWrtLockSession * in_poRdWrtSession )
{
	//���û�мӹ��������򲻴����������
	if(in_poRdWrtSession->m_blReadLockFlag)
	{
#ifndef LINUXCODE
			::EnterCriticalSection(&m_stReaderSection);
#else
			pthread_mutex_lock(&m_stReaderSection);

#endif
			if(in_poRdWrtSession->m_stReadNode.m_ushLockCount > m_ulReaderCount)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "��ǰ��д�������ǼӶ���%d����������Դ�мӶ���ֻ��%d����������λ",
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
			//����ȴ���
			m_oRdWaitEvent.Active();
			m_oWrtWaitEvent.Active();
			

	}
	return 0;

}
/*******************************************************
	  �� �� ��: rl_UnlockWrt
	   	��������: ��д��
	   	��    ��: in_poRdWrtSession	����--��д��������
	    ����˵��: 0�ɹ�	 -1ʧ��
********************************************************/
long CRdWrtLockResource::rl_UnlockWrt(CLocalRdWrtLockSession * in_poRdWrtSession )
{
	//���û�мӹ�д�����򲻴����������
	if(in_poRdWrtSession->m_blWriteLockFlag)
	{
			if(m_blAllowReadFlag)
			{
				sprintf(in_poRdWrtSession->m_szErrMsg, "��ǰ��д�������Ǽ�д����־��������Դû�м�д��");
				return -1;
			}
			m_blAllowReadFlag = true;
			m_ulWrtThreadID = 0;	//��ǰд���߳�ID��
#ifndef LINUXCODE
			::LeaveCriticalSection(&m_stWriterSection);
#else
			pthread_mutex_unlock(&m_stWriterSection);
#endif
			in_poRdWrtSession->m_blWriteLockFlag = false;
			//����ȴ���
			m_oRdWaitEvent.Active();
			m_oWrtWaitEvent.Active();
			

		
	}
	return 0;
}


CRdWrtLockResource::CRdWrtLockResource()
{
#ifndef LINUXCODE
		::InitializeCriticalSection(&m_stReaderSection);		//����ʹ�õ��ٽ�������
		::InitializeCriticalSection(&m_stWriterSection);			//дʹ�õ��ٽ�������	
#else
		/*
		 *	GUOGUO 2009-08-11 �����ٽ�����������Ϊ�ɵݹ���,֧��ͬһ�߳���������lock
		 */
		//	ADD
		pthread_mutexattr_t attr;

		/*
		 *	2010-11-19��x86_64���������з��ֵ�һ�������BUG
		 *		��ʱ�������,�޷��ݹ���. ��ôҲû�뵽����Ϊ������캯�������
		 *		pthread_mutex_init ����95����, ���������ǵݹ��.
		 *		������pthread_mutex_initʧ�ܵ�ԭ��������Ϊattrû��ʹ��pthread_mutexattr_init��ʼ��.
		 *		�ƺ��� 32λ��RedHat AS4 Update6�Ļ�����,��������..
		 *		�ܽ�����ԭ��,����Ϊ��ͬ��ϵͳ,���µļ��ص��ڴ�ҳû�� ��� zero ����,������ô��,
		 *		���ǲ�������ϵͳΪ����������,��Ӧ�����������ĵ���pthread_mutexattr_init���������ʼ������
		 *		���������ȫ��
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
		memset(&m_stReaderHeadNode, 0, sizeof(m_stReaderHeadNode)); //����ͷ�ڵ�
		m_ulReaderCount = 0;	//�������ü���
		m_blAllowReadFlag = true;	//�������־
		m_ulWrtThreadID = 0;	//��ǰд���߳�ID��
	
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
		memset(&m_stReaderHeadNode, 0, sizeof(m_stReaderHeadNode)); //����ͷ�ڵ�

}


CMyWaitEvent::CMyWaitEvent(void)
{
	m_blRunFlag = true;
	#ifndef LINUXCODE
		
		if ( (m_hRecordData = ::CreateEvent(NULL,FALSE,FALSE,NULL)) == NULL )
		{
			assert(0);
			throw exception("<MWaitEvent>�����ȴ��¼���������");
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
			//	�����ж�,�Ա㰲ȫ���л���,������ܵ��»��ѵĴ�λ
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
		 *	1s=1000000000����
		 *	1s=1000000	 ΢��
		 *	1s=1000		 ����
		 */
		future = tp.millitm  + lMSec % 1000;	//	δ��lMsec�ĺ���ֵ
		future = future * 1000000;				//	���������

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
			//	�õ�����,�����ٴ��ж�m_nCount
			//	��Ϊ 2�ֿ���.
			//	1:timewait�����Ǳ�UNIX ��signal����
			//	2:pthread_cond_signal��������
			rc = pthread_cond_timedwait(&m_hRecordData,&m_hMutex,&abstime);
			if(rc != 0)
			{
				if(rc == ETIMEDOUT)
				{

					//	��ʱ����
					pthread_mutex_unlock(&m_hMutex);


					return false;
				}
				else if(rc == EINTR)
				{
					//	������signal �ж��ź�
					//	��ʵӦ����������sttemp��,����,�����.
					continue;
				}
				else
				{
					//	��������
					//pthread_mutex_unlock(&m_hMutex);
					//return;
					continue;
				}
			}
			else
			{
				//	��������
				continue;	//	�ǳ������pthread_cond_timewait==0��ʱ��,
							//	�ƺ�����û�б�pthread_cond_signal����,Ҳ���Ǳ��ж��źŻ���
							//	��̫�������ʲô����ᵼ�����������鷢��
							//	Ϊ�˰�ȫ�����׼������������������m_nCount��ֵ.
			}
		}
		if(m_nCount <= 0)
		{
			//	���Բ����ܷ����Ĵ���,ǰ���Ѿ���������Ŭ��ȥ����m_nCount��ֵ
			//	��������ߵ�����,��ôֻ����ȥ��ѭ����.ϵͳ�������Ѿ��൱������
			//	Ҳ����Կ�������m_nCount = 0,�����Ҳ�����������,
			//	�Ѿ�����������,Ϊ�˱��������һ������,����Ϊ��Ӧ��ȥ��ѭ��.Ȼ��ĵȴ�coderȥ����������
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
