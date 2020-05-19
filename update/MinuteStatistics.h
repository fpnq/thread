// MinuteStatistics.h: interface for the CMinuteStatistics class.
//	主要用于对数据进行统计，以一分钟一个数据，也可以取总的数据，每天复位一次
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
	//累加操作：如请求增加等
	void operator += (unsigned long lIn);
	void operator ++ (int);
	//获取总的统计数量
	__int64 GetTotal(void);
	//获得当前最后一分钟的统计数量
	long GetLastMinuteCount();
	// add by zhongjb 20111111 for 统计每个通道的数据包发送情况，包括发包失败、重试、丢失
	void ClearLastMinuteCount();
	// end add
protected:
	MCriticalSection m_oSection;	//临界区
	__int64			m_llOldMinuteTotal;	//1分钟前的总计
	__int64			m_llTotal;		//当前总计
	long			m_lMinuteCount;	//1分钟统计
	unsigned short	m_ushMinuteHHMM;	//当前一钟的时间
	unsigned long	m_ulCurDate;

	void rv_CalculateMinute();
};

#endif // !defined(AFX_MINUTESTATISTICS_H__D9ABFACF_9340_4E09_85A3_0F46D9DB63C2__INCLUDED_)
