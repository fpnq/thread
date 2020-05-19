// MinuteStatistics.h: interface for the CMinuteStatistics class.
//	��Ҫ���ڶ����ݽ���ͳ�ƣ���һ����һ�����ݣ�Ҳ����ȡ�ܵ����ݣ�ÿ�츴λһ��
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINUTESTATISTICS_H__D9ABFACF_9340_4E09_85A3_0F46D9DB63C2__INCLUDED_)
#define AFX_MINUTESTATISTICS_H__D9ABFACF_9340_4E09_85A3_0F46D9DB63C2__INCLUDED_

#include "MMutex.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMinuteStatistics  
{
public:
	CMinuteStatistics();
	virtual ~CMinuteStatistics();
public:
	//�ۼӲ��������������ӵ�
	void operator += (unsigned long lIn);
	void operator ++ (int);
	//��ȡ�ܵ�ͳ������
	__int64 GetTotal(void);
	//��õ�ǰ���һ���ӵ�ͳ������
	long GetLastMinuteCount();
	// add by zhongjb 20111111 for ͳ��ÿ��ͨ�������ݰ������������������ʧ�ܡ����ԡ���ʧ
	void ClearLastMinuteCount();
	// end add
protected:
	MCriticalSection m_oSection;	//�ٽ���
	__int64			m_llOldMinuteTotal;	//1����ǰ���ܼ�
	__int64			m_llTotal;		//��ǰ�ܼ�
	long			m_lMinuteCount;	//1����ͳ��
	unsigned short	m_ushMinuteHHMM;	//��ǰһ�ӵ�ʱ��
	unsigned long	m_ulCurDate;

	void rv_CalculateMinute();
};

#endif // !defined(AFX_MINUTESTATISTICS_H__D9ABFACF_9340_4E09_85A3_0F46D9DB63C2__INCLUDED_)
