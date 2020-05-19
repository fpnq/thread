// MinuteStatistics.cpp: implementation of the CMinuteStatisticsEx class.
//	��Ҫ���ڶ����ݽ���ͳ�ƣ���һ����һ�����ݣ�Ҳ����ȡ�ܵ����ݣ�ÿ�츴λһ��
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#include "MinuteStatisticsEx.h"
#include "MDateTime.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMinuteStatisticsEx::CMinuteStatisticsEx()
{
	Clear();
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	m_ulCurDate = MDateTime::Now().DateToLong();
}
//������е�ͳ��
void	CMinuteStatisticsEx::Clear()
{
	MLocalSection								msection;

	msection.Attch(&m_oSection);
	m_llTotal = 0;		//�����ܼ�
	m_ulLastMinute = 0;		//ǰһ��������ͳ��
	m_ulCurMinute = 0;		//��ǰһ��������ͳ��
	m_ulTotalCnt = 0;		//�ܴ���ͳ��
	m_ulLastMinuteCnt = 0;		//ǰһ���Ӵ���ͳ��
	m_ulCurMinuteCnt = 0;		//��ǰһ���Ӵ���ͳ��
	m_ulMaxMinute = 0;		//���һ��������ͳ��
	m_ulMaxMinuteCnt = 0;	//���һ���Ӵ���ͳ��
	m_ushMaxMinuteHHMM = 0;	//���һ�ӵ�ʱ��
	m_ushTotalMinutes = 0;
	//add by liuqy 20120113 for ������������
	m_stFirstDataSecond = 0;	//���ݵ�����ʱ�� ��1970������������
	m_stLastDataSecond = 0;		//���һ�����ݵ�����ʱ�� ��1970������������
	m_ulRemoveSecond = 0;		//��ȥ��ʱ��
	m_dMaxFlowRate = 0;
	m_dFlowRate = 0;
	m_stLastMinuteRemoveSecond = 0;
	m_stLastMinuteFirstDataSecond = 0;
}

CMinuteStatisticsEx::~CMinuteStatisticsEx()
{

}
void CMinuteStatisticsEx::rv_CalculateMinute()
{
	double dData;
	long lUseTime;
	unsigned long ulNow = MDateTime::Now().DateToLong() ;	
	if(ulNow != m_ulCurDate )
	{
		Clear();
		m_ulCurDate = ulNow;
	}
	//add by liuqy 20120113 for ������������
	if(0 < m_stFirstDataSecond)
	{
		lUseTime = 0;
		if(0 < m_stLastMinuteFirstDataSecond )
		{
			lUseTime = m_stLastDataSecond- m_stLastMinuteFirstDataSecond - m_stLastMinuteRemoveSecond ;
			if(0 == lUseTime)
				lUseTime = 1;
		}

		if(0 >= lUseTime)
		{
			m_dFlowRate = 0.0;
		}
		else
		{
			dData = m_ulCurMinute;
			dData /= lUseTime;
			m_dFlowRate = dData;
		}
	}
	else
	{
		m_dFlowRate = 0.0;
	}
	m_stLastMinuteRemoveSecond = 0;
	m_stLastMinuteFirstDataSecond = 0;
	//���һ��������ͳ��
	if(m_ulMaxMinute < m_ulCurMinute)
	{
		m_ulMaxMinute = m_ulCurMinute;
		m_ulMaxMinuteCnt = m_ulCurMinuteCnt;	//���һ��������ʱ����ͳ��
		m_ushMaxMinuteHHMM = m_ushMinuteHHMM;	//���һ�ӵ�ʱ��
		m_dMaxFlowRate = m_dFlowRate;
	}
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	m_ulLastMinute = m_ulCurMinute;		//ǰһ��������ͳ��
	m_ulCurMinute = 0;		//��ǰһ��������ͳ��
	m_ulLastMinuteCnt = m_ulCurMinuteCnt;		//ǰһ���Ӵ���ͳ��
	m_ulCurMinuteCnt = 0;		//�ܴ���ͳ��
	if(0 != m_ulLastMinute)
		m_ushTotalMinutes++;

}
//add by liuqy 20120113 for ������������
double		CMinuteStatisticsEx::GetMaxFlowRate()	//�����������
{
	return m_dMaxFlowRate;
}
double		CMinuteStatisticsEx::GetFlowRate()		//����������
{
	double dRet = 0.0;
	unsigned long ulUseDataSecond;
	MLocalSection								msection;
	//add by liuqy 20120113 for ������������
	if(0 < m_stFirstDataSecond)
	{
		msection.Attch(&m_oSection);
		ulUseDataSecond = m_stLastDataSecond - m_stFirstDataSecond - m_ulRemoveSecond;
		dRet = m_llTotal;
		msection.UnAttch();
		ulUseDataSecond += 1;
	
		dRet /= ulUseDataSecond;
	}
	return dRet; 
}
double		CMinuteStatisticsEx::GetLastMinuteFlowRate()		//����ǰһ��������
{
	return m_dFlowRate; 
}

//..............................................................................................................................
void CMinuteStatisticsEx::operator += (unsigned long lIn)
{
	MLocalSection								msection;
	unsigned short ushNowTime;
	time_t stNow;

	msection.Attch(&m_oSection);
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//��ǰһ�ӵ�ʱ��
	//ʱ�䲻��ʱ����Ҫ����ͳ��
	if(ushNowTime != m_ushMinuteHHMM)
	{
		rv_CalculateMinute();
	}
	m_llTotal += lIn;
	m_ulCurMinute += lIn;
	m_ulTotalCnt++;
	m_ulCurMinuteCnt++;
	stNow = time(NULL);
	//add by liuqy 20120113 for ������������
	if(0 == m_stFirstDataSecond)
	{
		m_stFirstDataSecond = stNow;
		m_stLastDataSecond = stNow;
	}
	//����5������ݣ�����Ҫ��ȥ���������
	if(5 <= stNow - m_stLastDataSecond)
	{
		m_ulRemoveSecond += ((stNow - m_stLastDataSecond));
	}
	if(0 == m_stLastMinuteFirstDataSecond)
	{
		 m_stLastMinuteFirstDataSecond = stNow;
	}
	else
	{
		if(5 <= stNow - m_stLastDataSecond)
			m_stLastMinuteRemoveSecond	+= ((stNow - m_stLastDataSecond));

	}
	m_stLastDataSecond = stNow;

}
//��ȡ�ܵ�ͳ������
__int64 CMinuteStatisticsEx::GetTotal(void)
{
	return m_llTotal;
}
//��õ�ǰ���һ���ӵ�ͳ������
unsigned long CMinuteStatisticsEx::GetLastMinute()
{
	rv_ChkMinute();
	return m_ulLastMinute;
}
//��ȡ�ܵĴ���ͳ��
unsigned long CMinuteStatisticsEx::GetTotalCount(void)
{
	return m_ulTotalCnt;
}
//��õ�ǰ���һ���ӵĴ���ͳ��
unsigned long CMinuteStatisticsEx::GetLastMinuteCount()
{
	rv_ChkMinute();
	return m_ulLastMinuteCnt;
}

void CMinuteStatisticsEx::rv_ChkMinute()
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
}
//ȡ���һ����ʱ�����ݣ�out_pulMinuteCnt��������������ʱ�Ĵ�����out_pushMinuteHHMMΪ������ʱ��ʱ��
unsigned long CMinuteStatisticsEx::GetMaxMinute(unsigned long * out_pulMinuteCnt, unsigned short * out_pushMinuteHHMM)
{
	rv_ChkMinute();
	if(NULL != out_pulMinuteCnt) 
		*out_pulMinuteCnt  = m_ulMaxMinuteCnt;
	if(NULL != out_pushMinuteHHMM)
		*out_pushMinuteHHMM = m_ushMaxMinuteHHMM;
	return m_ulMaxMinute;
}
