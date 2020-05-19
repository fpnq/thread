//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ������ڴ浥Ԫ
//��Ԫ��������Ҫ������̴�����ֹͣ�Ȳ���
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
#ifndef __MEngine_MShareMemH__
#define __MEngine_MShareMemH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
#include "MMutex.h"
//------------------------------------------------------------------------------------------------------------------------------
class MShareMem
{
protected:
	#ifndef LINUXCODE
		HANDLE							m_hRecordData;
		void						*	m_lpData;
	#else
		int								m_hRecordData;
		void						*	m_lpData;
	#endif
protected:
	int									m_iSize;
	bool								m_bCreate;
	MMutex								m_mMutex;
protected:
	__inline int  inner_createkeyfromstring(const char * strIn);
public:
	MShareMem(void);
	virtual ~MShareMem();
public:
	//���������ڴ�
	int  Create(MString strName,unsigned long lSize,bool bAnyAccess = false);
	//�򿪹����ڴ�
	int  Open(MString strName);
	//�رչ����ڴ�
	void Close(void);
public:
	//��ȡ��д�����ݵ������ڴ�
	int  Read(unsigned long lOffset,char * lpOut,unsigned long lSize);
	int  Write(unsigned long lOffset,const char * lpIn,unsigned long lSize);
public:
	//��ȡ�����ڴ泤��
	int  GetSize(void);
};
//------------------------------------------------------------------------------------------------------------------------------
class MShareMemMt
{
protected:
	
#ifndef LINUXCODE
		HANDLE							m_hRecordData;
		void						*	m_lpData;
#else
		int								m_hRecordData;
		void						*	m_lpData;
#endif

protected:
	int									m_iSize;
	bool								m_bCreate;
	MMutex								m_mMutex;
protected:
	__inline int  inner_createkeyfromstring(const char * strIn);
public:
	MShareMemMt(void);
	virtual ~MShareMemMt();
public:
	//���������ڴ�
	int  Create(MString strName,unsigned long lSize);
	//�򿪹����ڴ�
	int  Open(MString strName);
	//�رչ����ڴ�
	void Close(void);
public:
	//��ȡ��д�����ݵ������ڴ�
	int  Read(unsigned long lOffset,char * lpOut,unsigned long lSize);
	int  Write(unsigned long lOffset,const char * lpIn,unsigned long lSize);
public:
	//��ȡ�����ڴ泤��
	int  GetSize(void);
};

#endif
//------------------------------------------------------------------------------------------------------------------------------
