// MinuteStatistics.h: interface for the CMinuteStatisticsEx class.
//	主要用于对数据进行统计，以一分钟一个数据，也可以取总的数据，每天复位一次,和统计数据，还对使用次数进行统计
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
	//累加数据操作，同时使用次数增加
	void operator += (unsigned long lIn);
	//获取总的统计数量
	__int64 GetTotal(void);
	//获得当前最后一分钟的数据统计
	unsigned long GetLastMinute();
	//获取总的次数统计
	unsigned long GetTotalCount(void);
	//获得当前最后一分钟的次数统计
	unsigned long GetLastMinuteCount();
	//有数据有分钟数
	unsigned short GetTotalMinutes() {		return m_ushTotalMinutes;	}
	//清除所有的统计
	void	Clear();
	//取最大一分钟时的数据，out_pulMinuteCnt返回最大分钟数据时的次数，out_pushMinuteHHMM为最大分钟时的时间
	unsigned long GetMaxMinute(unsigned long * out_pulMinuteCnt = NULL, unsigned short * out_pushMinuteHHMM = NULL);
	//add by liuqy 20120113 for 计算数据流量
	double		GetMaxFlowRate();	//返回总流速
	double		GetFlowRate();		//返回当前流速
	double		GetLastMinuteFlowRate();		//返回前一分钟流速
protected:
	MCriticalSection m_oSection;	//临界区
	__int64				m_llTotal;		//数据总计
	unsigned long		m_ulLastMinute;		//前一分钟数据统计
	unsigned long		m_ulCurMinute;		//当前一分数据钟统计
	unsigned long		m_ulTotalCnt;		//总次数统计
	unsigned long		m_ulLastMinuteCnt;		//前一分钟次数统计
	unsigned long		m_ulCurMinuteCnt;		//当前一分钟次数统计
	unsigned short	m_ushMinuteHHMM;	//当前一钟的时间
	unsigned long	m_ulCurDate;
	unsigned long		m_ulMaxMinute;		//最大一分钟数据统计
	unsigned long		m_ulMaxMinuteCnt;	//最大一分钟数据时的次数
	unsigned short	m_ushMaxMinuteHHMM;	//最大一钟的时间
	unsigned short	m_ushTotalMinutes;	//有数据的分钟数
	//add by liuqy 20120113 for 计算数据流量
	time_t				m_stFirstDataSecond;	//数据到来的时间 从1970年以来的秒数
	time_t				m_stLastDataSecond;		//最后一个数据到来的时间 从1970年以来的秒数
	time_t				m_stLastMinuteFirstDataSecond;	//最后一分钟的时间第一个数据到达时间
	unsigned long		m_stLastMinuteRemoveSecond;		//除去的时间
	unsigned long		m_ulRemoveSecond;		//除去的时间
	double				m_dMaxFlowRate;				//最大时的流量
	double				m_dFlowRate;				//最大时的流量
	void rv_CalculateMinute();
	void rv_ChkMinute();
};

#endif // !defined(AFX_MINUTESTATISTICSEX_H__D9ABFACF_9340_4E09_85A3_0F46D9DB69C3__INCLUDED_)
