// MinuteStatistics.cpp: implementation of the CMinuteStatistics class.
//	主要用于对数据进行统计，以一分钟一个数据，也可以取总的数据，每天复位一次
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#include "MinuteStatistics.h"
#include "MDateTime.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMinuteStatistics::CMinuteStatistics()
{
	m_llOldMinuteTotal = 0;	//1分钟前的总计
	m_llTotal = 0;		//当前总计
	m_lMinuteCount = 0;	//1分钟统计
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
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
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间

}

//..............................................................................................................................
void CMinuteStatistics::operator += (unsigned long lIn)
{
	MLocalSection								msection;
	unsigned short ushNowTime;

	msection.Attch(&m_oSection);
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	//时间不等时，需要计算统计
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
	ushNowTime = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
	//时间不等时，需要计算统计
	if(ushNowTime != m_ushMinuteHHMM)
	{
		rv_CalculateMinute();
	}
	m_llTotal ++;
}
//获取总的统计数量
__int64 CMinuteStatistics::GetTotal(void)
{
	return m_llTotal;
}
//获得当前最后一分钟的统计数量
long CMinuteStatistics::GetLastMinuteCount()
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
	return m_lMinuteCount;
}

// add by zhongjb 20111111 for 统计每个通道的数据包发送情况，包括发包失败、重试、丢失
void CMinuteStatistics::ClearLastMinuteCount()
{
	MLocalSection section;
	section.Attch(&m_oSection);

	m_llOldMinuteTotal = 0;									// 1分钟前的总计
	m_llTotal = 0;											// 当前总计
	m_lMinuteCount = 0;										// 1分钟统计
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	// 当前一钟的时间
	m_ulCurDate = MDateTime::Now().DateToLong();

	section.UnAttch();
}
// end add
