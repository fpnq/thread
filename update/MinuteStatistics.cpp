// MinuteStatistics.cpp: implementation of the CMinuteStatistics class.
//	��Ҫ���ڶ����ݽ���ͳ�ƣ���һ����һ�����ݣ�Ҳ����ȡ�ܵ����ݣ�ÿ�츴λһ��
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#include "MinuteStatistics.h"
#include "MDateTime.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMinuteStatistics::CMinuteStatistics()
{
	m_llOldMinuteTotal = 0;	//1����ǰ���ܼ�
	m_llTotal = 0;		//��ǰ�ܼ�
	m_lMinuteCount = 0;	//1����ͳ��
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	m_ulCurDate = MDateTime::Now().DateToLong();
}

CMinuteStatistics::~CMinuteStatistics()
{

}
void CMinuteStatistics::rv_CalculateMinute()
{
	unsigned long ulNow = MDateTime::Now().DateToLong() ;	
	if(ulNow != m_ulCurDate )
	{
		m_llTotal = 0;
		m_llOldMinuteTotal = 0;
		m_ulCurDate = ulNow;
	}
	m_lMinuteCount = m_llTotal - m_llOldMinuteTotal;
	m_llOldMinuteTotal = m_llTotal;
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��

}

//..............................................................................................................................
void CMinuteStatistics::operator += (unsigned long lIn)
{
	MLocalSection								msection;
	unsigned short ushNowTime;

	msection.Attch(&m_oSection);
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	//ʱ�䲻��ʱ����Ҫ����ͳ��
	if(ushNowTime != m_ushMinuteHHMM)
	{
		rv_CalculateMinute();
	}
	m_llTotal += lIn;
}
void CMinuteStatistics::operator ++ (int)
{
	MLocalSection								msection;
	unsigned short ushNowTime;

	msection.Attch(&m_oSection);
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	//ʱ�䲻��ʱ����Ҫ����ͳ��
	if(ushNowTime != m_ushMinuteHHMM)
	{
		rv_CalculateMinute();
	}
	m_llTotal ++;
}
//��ȡ�ܵ�ͳ������
__int64 CMinuteStatistics::GetTotal(void)
{
	return m_llTotal;
}
//��õ�ǰ���һ���ӵ�ͳ������
long CMinuteStatistics::GetLastMinuteCount()
{
	MLocalSection								msection;
	unsigned short ushNowTime;
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	//ʱ�䲻��ʱ������Ҫ�������ٿ�����,��Ҫ����ͳ��
	if(ushNowTime != m_ushMinuteHHMM)
	{
		msection.Attch(&m_oSection);
		ushNowTime = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
		if(ushNowTime != m_ushMinuteHHMM)
			rv_CalculateMinute();
		msection.UnAttch();
	}
	return m_lMinuteCount;
}

// add by zhongjb 20111111 for ͳ��ÿ��ͨ�������ݰ������������������ʧ�ܡ����ԡ���ʧ
void CMinuteStatistics::ClearLastMinuteCount()
{
	MLocalSection section;
	section.Attch(&m_oSection);

	m_llOldMinuteTotal = 0;									// 1����ǰ���ܼ�
	m_llTotal = 0;											// ��ǰ�ܼ�
	m_lMinuteCount = 0;										// 1����ͳ��
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	// ��ǰһ�ӵ�ʱ��
	m_ulCurDate = MDateTime::Now().DateToLong();

	section.UnAttch();
}
// end add
