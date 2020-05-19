// MinuteStatistics.cpp: implementation of the CMinuteStatisticsEx class.
//	主要用于对数据进行统计，以一分钟一个数据，也可以取总的数据，每天复位一次
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
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	m_ulCurDate = MDateTime::Now().DateToLong();
}
//清除所有的统计
void	CMinuteStatisticsEx::Clear()
{
	MLocalSection								msection;

	msection.Attch(&m_oSection);
	m_llTotal = 0;		//数据总计
	m_ulLastMinute = 0;		//前一分钟数据统计
	m_ulCurMinute = 0;		//当前一分数据钟统计
	m_ulTotalCnt = 0;		//总次数统计
	m_ulLastMinuteCnt = 0;		//前一分钟次数统计
	m_ulCurMinuteCnt = 0;		//当前一分钟次数统计
	m_ulMaxMinute = 0;		//最大一分钟数据统计
	m_ulMaxMinuteCnt = 0;	//最大一分钟次数统计
	m_ushMaxMinuteHHMM = 0;	//最大一钟的时间
	m_ushTotalMinutes = 0;
	//add by liuqy 20120113 for 计算数据流量
	m_stFirstDataSecond = 0;	//数据到来的时间 从1970年以来的秒数
	m_stLastDataSecond = 0;		//最后一个数据到来的时间 从1970年以来的秒数
	m_ulRemoveSecond = 0;		//除去的时间
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
	//add by liuqy 20120113 for 计算数据流量
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
	//最大一分钟数据统计
	if(m_ulMaxMinute < m_ulCurMinute)
	{
		m_ulMaxMinute = m_ulCurMinute;
		m_ulMaxMinuteCnt = m_ulCurMinuteCnt;	//最大一分钟数据时次数统计
		m_ushMaxMinuteHHMM = m_ushMinuteHHMM;	//最大一钟的时间
		m_dMaxFlowRate = m_dFlowRate;
	}
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	m_ulLastMinute = m_ulCurMinute;		//前一分钟数据统计
	m_ulCurMinute = 0;		//当前一分数据钟统计
	m_ulLastMinuteCnt = m_ulCurMinuteCnt;		//前一分钟次数统计
	m_ulCurMinuteCnt = 0;		//总次数统计
	if(0 != m_ulLastMinute)
		m_ushTotalMinutes++;

}
//add by liuqy 20120113 for 计算数据流量
double		CMinuteStatisticsEx::GetMaxFlowRate()	//返回最大流速
{
	return m_dMaxFlowRate;
}
double		CMinuteStatisticsEx::GetFlowRate()		//返回总流速
{
	double dRet = 0.0;
	unsigned long ulUseDataSecond;
	MLocalSection								msection;
	//add by liuqy 20120113 for 计算数据流量
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
double		CMinuteStatisticsEx::GetLastMinuteFlowRate()		//返回前一分钟流速
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
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	//时间不等时，需要计算统计
	if(ushNowTime != m_ushMinuteHHMM)
	{
		rv_CalculateMinute();
	}
	m_llTotal += lIn;
	m_ulCurMinute += lIn;
	m_ulTotalCnt++;
	m_ulCurMinuteCnt++;
	stNow = time(NULL);
	//add by liuqy 20120113 for 计算数据流量
	if(0 == m_stFirstDataSecond)
	{
		m_stFirstDataSecond = stNow;
		m_stLastDataSecond = stNow;
	}
	//超过5秒的数据，则需要除去多余的秒数
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
//获取总的统计数量
__int64 CMinuteStatisticsEx::GetTotal(void)
{
	return m_llTotal;
}
//获得当前最后一分钟的统计数量
unsigned long CMinuteStatisticsEx::GetLastMinute()
{
	rv_ChkMinute();
	return m_ulLastMinute;
}
//获取总的次数统计
unsigned long CMinuteStatisticsEx::GetTotalCount(void)
{
	return m_ulTotalCnt;
}
//获得当前最后一分钟的次数统计
unsigned long CMinuteStatisticsEx::GetLastMinuteCount()
{
	rv_ChkMinute();
	return m_ulLastMinuteCnt;
}

void CMinuteStatisticsEx::rv_ChkMinute()
{
	MLocalSection								msection;
	unsigned short ushNowTime;
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	//时间不等时，则需要加锁后再看不等,需要计算统计
	if(ushNowTime != m_ushMinuteHHMM)
	{
		msection.Attch(&m_oSection);
		ushNowTime = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
		if(ushNowTime != m_ushMinuteHHMM)
			rv_CalculateMinute();
		msection.UnAttch();
	}
}
//取最大一分钟时的数据，out_pulMinuteCnt返回最大分钟数据时的次数，out_pushMinuteHHMM为最大分钟时的时间
unsigned long CMinuteStatisticsEx::GetMaxMinute(unsigned long * out_pulMinuteCnt, unsigned short * out_pushMinuteHHMM)
{
	rv_ChkMinute();
	if(NULL != out_pulMinuteCnt) 
		*out_pulMinuteCnt  = m_ulMaxMinuteCnt;
	if(NULL != out_pushMinuteHHMM)
		*out_pushMinuteHHMM = m_ushMaxMinuteHHMM;
	return m_ulMaxMinute;
}
