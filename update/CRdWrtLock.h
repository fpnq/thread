/***********************************************************************
 * �ļ����ƣ�CRdWrtLock.h
 * ����������ʵ�ֶ�д������
 * ������ʷ��liuqy 20110111 cd
 * ��д��ʵ�����£�
 1����Դ��
	1) ��д��־����Ϊ0��ʾΪд����Ϊ1��ʾΪ��
	2) �������ü�������ʼΪ0������һ�����߼�һ���˳�һ�����߼�һ
	3) ������ͷ��ʹ���ڱ�������߳�ID��һ����
	4) д��������Դ��д����ʱ���ü�����Ҫ����
	5) ���ٽ�����Դ���޸Ķ�д��־���������ü���ʱʹ��
 2�����������̣�
	1)	������ٽ�����Դ
	2)	����д��־�Ƿ�Ϊ1����Ϊ1����һ����Ϊ1���Լ��Ѿ���д������ִ�е�5��
	3)	��ѯ�Լ��߳��Ƿ����Ѿ������ڶ������У�����ǣ����˳��ٽ���,�������˳�����д�������ˣ�
	4)	�˳��ٽ�����ѭ���ȴ���д��־�Ƿ�Ϊ1����ʱ������ʧ���˳���Ϊ1��������һ��
	5)	���������ü�����1
	6)	���Լ������������Լ����߳�ID�����������ӵ�������ͷ��
	7)	�˳��ٽ���
	8)	���ؼ����ɹ�
 3�����������̣�
	1)	������ٽ�����Դ
	2)	���Լ������Ӷ�������ȡ��
	3)	�������ü�������0�����һ
	4)	�˳��ٽ���
 4��д�������̣�
	1)	1)	�Ѿ�����ǰ�̼߳ӹ�д������ֱ�ӷ��سɹ�; д������Դ����
	2)	����д��־�Ƿ�Ϊ0������ǣ���������12����������һ��
	3)	������ٽ�����Դ
	4)	����д��־�޸�Ϊ0
	5)	�Ӷ�����������Ϊ�Լ��߳�ID�ĸ���
	6)	�˳��ٽ���
	7)	ѭ���ȴ��������ü���С�ڵ����Լ��߳�ID���������ȴ���ʱ��������7������С�ڵ����Լ��߳�ID����ʱ������һ��
	8)	д�߳�ID����Ϊ��ǰ�߳�ID�����سɹ�
	9)	������ٽ�����Դ
	10)	����д��־�޸�Ϊ1
	11)	�˳��ٽ���
	12)	д������Դ����
	13)	����ʧ��
 5��д�������̣�
	1)	����д��־�޸�Ϊ1,д�߳�ID����Ϊ0
	2)	д������Դ����
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

//�ڵ����
typedef struct __TMYLISTNODE
{
	struct	__TMYLISTNODE * m_pstPrevious;	//ǰһ�� ��Ϊͷʱ����ֵΪ��Ч
	struct	__TMYLISTNODE * m_pstNext;	//��һ��
}TMyListNode;

//��ʼ��һ���ڵ�
void rv_InitListNode(TMyListNode * in_pstNode);
//��ͷ������һ���ڵ�
void rv_AddListNode(TMyListNode * in_pstHead, TMyListNode * in_pstAddNode);
//�ӽڵ���ɾ��
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

//��д�������࣬���Զ�д����Դ���ж�д�ӽ�������
class CLocalRdWrtLockSession
{
	friend class CRdWrtLockResource;
public:
	typedef enum eLOCKSEMFLAG
	{
		None = 0,	/*�����м���*/
		Read,	/*����ʽ����ֻ����һ��*/
		Write,  /*д��ʽ*/
	}ELockFlag;
		
	/*******************************************************
       		�� �� ��: rl_WriteLock
       		��������: д������
       		��    ��: in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       		����˵��: 0�ɹ� 	 -1ʧ��
	********************************************************/
	long rl_WriteLock(long in_lTimeOut = 30);
	long rl_WriteLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo);

	/*******************************************************
       		�� �� ��: rl_ReadLock
            ��������: ��������,ע�������ǰ�й�д��û�н�������ϵͳֱ�ӷ��سɹ�
       		��    ��: in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       		����˵��: 0�ɹ�	  -1ʧ��
	********************************************************/
	long rl_ReadLock(long in_lTimeOut = 30);
	long rl_ReadLock(long in_lTimeOut, const char * in_pszFileName, unsigned short in_ushFileLineNo);
	/*******************************************************
       		�� �� ��: rl_UnlockWrtKeepRd
       		��������: ��д������������
       		��    ��: ��
       		����˵��: 0�ɹ�	 -1ʧ��
	********************************************************/
	long rl_UnlockWrtKeepRd();
	/*******************************************************
	      �� �� ��: rl_Unlock
	   		��������: �����е���
	   		��    ��: ��
	       	����˵��: 0�ɹ�	 -1ʧ��
	********************************************************/
	long rl_Unlock( );

	/*******************************************************
       		�� �� ��: re_GetLockState
       		��������: ȡ��ǰ�������
       		��    ��: ��
       		����˵��: Noneδ������Read�Ѿ��ӹ�������Write�Ѿ��ӹ�д��
	********************************************************/
	ELockFlag	re_GetLockState();
	
	CLocalRdWrtLockSession(CRdWrtLockResource * in_poRdWrtLockRsc);
	virtual ~CLocalRdWrtLockSession();

	CLocalRdWrtLockSession();
	//���ö�д����Դ
	void SetLockRsc(CRdWrtLockResource & in_oRdWrtLockRsc);
	
	//ȡ������Ϣ
	const char * rpsz_GetErrMsg() { return m_szErrMsg;	};

protected:
	typedef struct _TLOCKNODE{
		TMyListNode	m_stNode;		//���ڵ�
		unsigned long	m_ulThreadID;	//��ǰ��ID
		unsigned short	m_ushLockCount;	//��������
		unsigned short	m_ushFileLineNo;
		char			m_szFileName[32];
	}TLockNode;
	
	TLockNode	m_stReadNode;
	bool	m_blReadLockFlag;	//���������
	bool	m_blWriteLockFlag;	//д�������
	CRdWrtLockResource * m_poRdWrtLockRsc;	//��д����Դ
	char	m_szErrMsg[512];	//������Ϣ

};
class CMyWaitEvent
{
protected:
	volatile	bool				m_blRunFlag;
#ifndef LINUXCODE
	HANDLE									m_hRecordData;
#else
	int										m_IsStartWait;
	volatile	 int										m_nCount;	//	��ȫ����,����thread_signal�ջ���,�Լ�����thread_wait���źŻ��ѵĴ���
	pthread_cond_t							m_hRecordData;
	pthread_mutex_t							m_hMutex;
#endif
public:
	CMyWaitEvent(void);
	virtual ~CMyWaitEvent();

public:
	//�����¼�
	void Active(void);
	//�ȴ��¼�
	bool Wait(unsigned long lMSec = 0xFFFFFFFF);
};


//��д����Դ
class CRdWrtLockResource
{
	friend class CLocalRdWrtLockSession;
protected:
	unsigned long	m_ulWrtThreadID;	//��ǰд�߳�ID
	#ifndef LINUXCODE
		CRITICAL_SECTION			m_stReaderSection;			//����ʹ�õ��ٽ�������
		CRITICAL_SECTION			m_stWriterSection;			//дʹ�õ��ٽ�������		

	#else
		pthread_mutex_t				m_stReaderSection;
		pthread_mutex_t				m_stWriterSection;			//дʹ�õ��ٽ�������		


	#endif
		TMyListNode			m_stReaderHeadNode;		//����ͷ�ڵ�
		unsigned	long		m_ulReaderCount;	//�������ü���
		bool				m_blAllowReadFlag;	//�������־
		CMyWaitEvent		m_oRdWaitEvent;
		CMyWaitEvent		m_oWrtWaitEvent;
	
	/*******************************************************
       		�� �� ��: rl_WriteLock
       		��������: д������
       		��    ��: in_poRdWrtSession	����--��д��������
					in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       		����˵��: 0�ɹ� 	 -1ʧ��
	********************************************************/
	long rl_WriteLock(CLocalRdWrtLockSession * in_poRdWrtSession,  long in_lTimeOut = 30);
	/*******************************************************
       		�� �� ��: rl_ReadLock
            ��������: ��������
       		��    ��: in_poRdWrtSession	����--��д��������
       				in_lTimeOut		--����-��ʱʱ�䵥λΪ�룬С�ڵ���0��ʾһֱ�ȴ�
       		����˵��: 0�ɹ�	  -1ʧ��
	********************************************************/
	long rl_ReadLock(CLocalRdWrtLockSession * in_poRdWrtSession, long in_lTimeOut = 30);
	/*******************************************************
       		�� �� ��: rl_UnlockWrtKeepRd
       		��������: ��д������������
       		��    ��: in_poRdWrtSession	����--��д��������
       		����˵��: 0�ɹ�	 -1ʧ��
	********************************************************/
	long rl_UnlockWrtKeepRd(CLocalRdWrtLockSession * in_poRdWrtSession);
	/*******************************************************
	      �� �� ��: rl_UnlockRd
	   		��������: �����
	   		��    ��: in_poRdWrtSession	����--��д��������
	       	����˵��: 0�ɹ�	 -1ʧ��
	********************************************************/
	long rl_UnlockRd(CLocalRdWrtLockSession * in_poRdWrtSession );
	/*******************************************************
	      �� �� ��: rl_UnlockWrt
	   		��������: ��д��
	   		��    ��: in_poRdWrtSession	����--��д��������
	       	����˵��: 0�ɹ�	 -1ʧ��
	********************************************************/
	long rl_UnlockWrt(CLocalRdWrtLockSession * in_poRdWrtSession );
	
public:

	CRdWrtLockResource();
	virtual ~CRdWrtLockResource();

	
};

#endif // !defined(CRWSESSION_H__7DA811_B8D7_4132_93C3_164ED646B5A2__INCLUDED_)
