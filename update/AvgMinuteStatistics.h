// AvgMinuteStatistics.h: interface for the CAvgMinuteStatistics class.
//	主要用于对最近十分钟数据进行统计求出平均值，以一分钟一个数据，超过十分钟的数据丢掉，
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
	//累加操作：如请求增加等
	void operator += (unsigned long lIn);
	void operator ++ (int);
	//获取总的统计数量
	__int64 GetTotal(void);
	//取平均值
	long GetAverageValForSecond();
	//清除数据
	void Clear();
	void	DoChk();
protected:
	MCriticalSection m_oSection;	//临界区
	long			m_lstMinuteTotal[MAX_AVG_MINUTEStatistics];	//1分钟前的总计
	unsigned short	m_ushMinuteHHMM;	//当前一钟的时间
	unsigned short	m_ushMinPosition;	//当前数据位置

};

#endif // !defined(AFX_AVGMINUTESTATISTICS_H__D9ABFACF__INCLUDED_)
