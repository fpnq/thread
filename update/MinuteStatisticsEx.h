// MinuteStatistics.h: interface for the CMinuteStatisticsEx class.
//	��Ҫ���ڶ����ݽ���ͳ�ƣ���һ����һ�����ݣ�Ҳ����ȡ�ܵ����ݣ�ÿ�츴λһ��,��ͳ�����ݣ�����ʹ�ô�������ͳ��
//  add by liuqy 20101203
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINUTESTATISTICSEX_H__D9ABFACF_9340_4E09_85A3_0F46D9DB69C3__INCLUDED_)
#define AFX_MINUTESTATISTICSEX_H__D9ABFACF_9340_4E09_85A3_0F46D9DB69C3__INCLUDED_

#include "MMutex.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMinuteStatisticsEx 
{
public:
	CMinuteStatisticsEx();
	virtual ~CMinuteStatisticsEx();
public:
	//�ۼ����ݲ�����ͬʱʹ�ô�������
	void operator += (unsigned long lIn);
	//��ȡ�ܵ�ͳ������
	__int64 GetTotal(void);
	//��õ�ǰ���һ���ӵ�����ͳ��
	unsigned long GetLastMinute();
	//��ȡ�ܵĴ���ͳ��
	unsigned long GetTotalCount(void);
	//��õ�ǰ���һ���ӵĴ���ͳ��
	unsigned long GetLastMinuteCount();
	//�������з�����
	unsigned short GetTotalMinutes() {		return m_ushTotalMinutes;	}
	//������е�ͳ��
	void	Clear();
	//ȡ���һ����ʱ�����ݣ�out_pulMinuteCnt��������������ʱ�Ĵ�����out_pushMinuteHHMMΪ������ʱ��ʱ��
	unsigned long GetMaxMinute(unsigned long * out_pulMinuteCnt = NULL, unsigned short * out_pushMinuteHHMM = NULL);
	//add by liuqy 20120113 for ������������
	double		GetMaxFlowRate();	//����������
	double		GetFlowRate();		//���ص�ǰ����
	double		GetLastMinuteFlowRate();		//����ǰһ��������
protected:
	MCriticalSection m_oSection;	//�ٽ���
	__int64				m_llTotal;		//�����ܼ�
	unsigned long		m_ulLastMinute;		//ǰһ��������ͳ��
	unsigned long		m_ulCurMinute;		//��ǰһ��������ͳ��
	unsigned long		m_ulTotalCnt;		//�ܴ���ͳ��
	unsigned long		m_ulLastMinuteCnt;		//ǰһ���Ӵ���ͳ��
	unsigned long		m_ulCurMinuteCnt;		//��ǰһ���Ӵ���ͳ��
	unsigned short	m_ushMinuteHHMM;	//��ǰһ�ӵ�ʱ��
	unsigned long	m_ulCurDate;
	unsigned long		m_ulMaxMinute;		//���һ��������ͳ��
	unsigned long		m_ulMaxMinuteCnt;	//���һ��������ʱ�Ĵ���
	unsigned short	m_ushMaxMinuteHHMM;	//���һ�ӵ�ʱ��
	unsigned short	m_ushTotalMinutes;	//�����ݵķ�����
	//add by liuqy 20120113 for ������������
	time_t				m_stFirstDataSecond;	//���ݵ�����ʱ�� ��1970������������
	time_t				m_stLastDataSecond;		//���һ�����ݵ�����ʱ�� ��1970������������
	time_t				m_stLastMinuteFirstDataSecond;	//���һ���ӵ�ʱ���һ�����ݵ���ʱ��
	unsigned long		m_stLastMinuteRemoveSecond;		//��ȥ��ʱ��
	unsigned long		m_ulRemoveSecond;		//��ȥ��ʱ��
	double				m_dMaxFlowRate;				//���ʱ������
	double				m_dFlowRate;				//���ʱ������
	void rv_CalculateMinute();
	void rv_ChkMinute();
};

#endif // !defined(AFX_MINUTESTATISTICSEX_H__D9ABFACF_9340_4E09_85A3_0F46D9DB69C3__INCLUDED_)
