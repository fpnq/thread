// AvgMinuteStatistics.h: interface for the CAvgMinuteStatistics class.
//	��Ҫ���ڶ����ʮ�������ݽ���ͳ�����ƽ��ֵ����һ����һ�����ݣ�����ʮ���ӵ����ݶ�����
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVGMINUTESTATISTICS_H__D9ABFACF__INCLUDED_)
#define AFX_AVGMINUTESTATISTICS_H__D9ABFACF__INCLUDED_

#include "MMutex.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_AVG_MINUTEStatistics	10

class CAvgMinuteStatistics  
{
public:
	CAvgMinuteStatistics();
	virtual ~CAvgMinuteStatistics();
public:
	//�ۼӲ��������������ӵ�
	void operator += (unsigned long lIn);
	void operator ++ (int);
	//��ȡ�ܵ�ͳ������
	__int64 GetTotal(void);
	//ȡƽ��ֵ
	long GetAverageValForSecond();
	//�������
	void Clear();
	void	DoChk();
protected:
	MCriticalSection m_oSection;	//�ٽ���
	long			m_lstMinuteTotal[MAX_AVG_MINUTEStatistics];	//1����ǰ���ܼ�
	unsigned short	m_ushMinuteHHMM;	//��ǰһ�ӵ�ʱ��
	unsigned short	m_ushMinPosition;	//��ǰ����λ��

};

#endif // !defined(AFX_AVGMINUTESTATISTICS_H__D9ABFACF__INCLUDED_)
