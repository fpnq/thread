// MinuteStatistics.cpp: implementation of the CAvgMinuteStatistics class.
//	主要用于对数据进行统计，以一分钟一个数据，也可以取总的数据，每天复位一次
//  add by liuqy 20100624
//////////////////////////////////////////////////////////////////////

#include "AvgMinuteStatistics.h"
#include "MDateTime.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAvgMinuteStatistics::CAvgMinuteStatistics()
{
	memset(&m_lstMinuteTotal, 0, sizeof(m_lstMinuteTotal));	//1分钟前的总计
	m_ushMinPosition = 0;		
	m_ushMinuteHHMM = MDateTime::Now().TimeToLong() / 100;	//当前一钟的时间
}

CAvgMinuteStatistics::~CAvgMinuteStatistics()
{

}
void CAvgMinuteStatistics::DoChk()
{
	MLocalSection								msection;
	unsigned long ulMin,Second;
	ulMin = MDateTime::Now().TimeToLong() ;	
	Second = ulMin % 100;
	ulMin  /=  100;
	if(ulMin != m_ushMinuteHHMM  || Second >= 58 || m_ushMinPosition >= MAX_AVG_MINUTEStatistics )
	{
		msection.Attch(&m_oSection);
		ulMin = MDateTime::Now().TimeToLong()/100 ;	
		if(ulMin != m_ushMinuteHHMM  || m_ushMinPosition >= MAX_AVG_MINUTEStatistics )
		{
			m_ushMinPosition ++;
			m_ushMinPosition = m_ushMinPosition % MAX_AVG_MINUTEStatistics;
			m_lstMinuteTotal[m_ushMinPosition] = 0;
			m_ushMinuteHHMM = ulMin;
		}
	}

}

//..............................................................................................................................
void CAvgMinuteStatistics::operator += (unsigned long lIn)
{
	MLocalSection								msection;
	DoChk();

	msection.Attch(&m_oSection);
	m_lstMinuteTotal[m_ushMinPosition] += lIn;
}
void CAvgMinuteStatistics::operator ++ (int)
{
	MLocalSection								msection;
	DoChk();
	
	msection.Attch(&m_oSection);
	m_lstMinuteTotal[m_ushMinPosition] ++;
}
//获取总的统计数量
__int64 CAvgMinuteStatistics::GetTotal(void)
{
	__int64 lRet;
	int i;

	for(lRet = 0, i=0; i<MAX_AVG_MINUTEStatistics; i++)
		lRet += m_lstMinuteTotal[i];
	return lRet;
}

void CAvgMinuteStatistics::Clear()
{
	MLocalSection section;
	section.Attch(&m_oSection);
	memset(&m_lstMinuteTotal, 0, sizeof(m_lstMinuteTotal));	//1分钟前的总计
	m_ushMinPosition = 0;		

	section.UnAttch();
}
//取平均值
long CAvgMinuteStatistics::GetAverageValForSecond()
{
	__int64 lRet;
	int i;
	int lSecond;
	MLocalSection								msection;
	unsigned long ulMin,lNowSecond;
	msection.Attch(&m_oSection);
	ulMin = MDateTime::Now().TimeToLong() ;	
	lNowSecond = ulMin % 100;
	ulMin  /=  100;
	for(lRet = 0, lSecond=0, i=0; i<MAX_AVG_MINUTEStatistics; i++)
	{
		lRet += m_lstMinuteTotal[i];
		if(m_ushMinPosition == i)
			lSecond += lNowSecond;
		else
			lSecond += 60;
	}
	return lRet/lSecond;

}
