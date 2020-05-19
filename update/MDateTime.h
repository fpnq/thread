//------------------------------------------------------------------------------------------------------------------------------
//单元名称：时间日期单元
//单元描述：主要处理和时间日期相关的函数
//创建日期：2007.3.15
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MDateTimeH__
#define __MEngine_MDateTimeH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
#ifdef LINUXCODE
#include <sys/timeb.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
//时间日期类，精确到秒，范围从1970-1-1 8:00:00 到 2038-1-19 3:14:07
//注意：以上范围是在32位操作系统下，64位操作系统下没有这个限制
//..............................................................................................................................
class MDateTime
{
protected:
	time_t					m_stRecordData;
public:
	MDateTime(void);
	MDateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay);
	MDateTime(unsigned short sHour,unsigned short sMin,unsigned short sSec,unsigned short sMSec);
	MDateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay,unsigned short sHour,unsigned short sMin,unsigned short sSec);
	MDateTime(time_t stTime);
	MDateTime(const MDateTime & mTime);
	virtual ~MDateTime();
public:
	//获取当前的日期和时间
	void SetCurDateTime(void);
	//从字符串中获取日期和时间，字符串格式位2005-05-29 13:05:12
	int  SetDatetimeByString(const char * strIn);
	int  SetDateByString(const char * strIn);
	int  SetTimeByString(const char * strIn);
public:
	//转换字符串，格式为2005-05-29 13:05:12
	MString DateTimeToString(void);
	MString DateToString(void);
	MString TimeToString(void);
	time_t  DateTimeToTimet(void);
	unsigned long DateToLong(void);
	unsigned long TimeToLong(void);
public:
	//重载运算符
	MDateTime & operator = (const MDateTime & mIn);
	//重载运算符，减去一个时间，得到两个时间之差，单位为秒
	int operator - (MDateTime & mIn);
	//重载运算符，减去一个时间（单位为秒），得到另外一个时间
	MDateTime operator - (int iIn);
	//重载运算符，加上一个时间（单位为秒），得到另外一个时间
	MDateTime operator + (int iIn);
	//重载运算符，加上一个时间（单位为秒），得到另外一个时间
	MDateTime & operator += (int iIn);
	//重载运算符，减去一个时间（单位为秒），得到另外一个时间
	MDateTime & operator -= (int iIn);
public:
	//获取该时间和现在时间的差值，单位为秒
	long GetDuration(void);
	//获取该时间和现在时间的差值，用字符串表示，格式为HH:MM:SS
	MString GetDurationString(void);
public:
	//获取当前时间是位于一周或一年的第几天，注意：星期天为0
	int  GetDayOfWeek(void);
	int  GetDayOfYear(void);
public:
	//分解出时间和日期
	int  DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay);
	int  DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec);
	int  GetYear(void);
	int  GetMonth(void);
	int  GetDay(void);
	int  GetHour(void);
	int  GetMinute(void);
	int  GetSecond(void);
public:
	//比较判断
	bool operator == (const MDateTime & mIn);
	bool operator != (const MDateTime & mIn);
	bool operator >  (const MDateTime & mIn);
	bool operator >= (const MDateTime & mIn);
	bool operator <  (const MDateTime & mIn);
	bool operator <= (const MDateTime & mIn);
public:
	//判断是否为空，为空表示从来没有设置过具体的时间
	bool IsEmpty(void);
	void Empty(void);
public:
	//获取当前时间的MDateTime对象
	static MDateTime Now(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//计时器，精确到毫秒（实际只能够精确到30--50毫秒左右，根据操作系统而定）
class MCounter
{
protected:
#ifndef LINUXCODE
	unsigned long					m_lRecordData;
#else
	struct timeb					m_tp;
#endif
public:
	MCounter(void);
	virtual ~MCounter();
public:
	//设置定时器（开始定时）
	void SetCurTickCount(void);
	//获取定时开始到当前的时间差（毫秒）
	unsigned long GetDuration(void);
	//获取当前定时开始到当前的时间差字符串表示（SS[MS]）
	MString GetDurationString(void);
public:
	//判断是否为空（没有使用过）
	bool IsEmpty(void);
	//设置为空
	void Empty(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//精确（高精度）计数器，返回计数单位为微秒，误差为0.5微秒
class MExactitudeCount
{
protected:
	#ifndef LINUXCODE
		double						m_dCpuFreq;
		__int64						m_RecordData;
	#else
		__int64						m_RecordData;
	#endif
public:
	MExactitudeCount(void);
	virtual ~MExactitudeCount();
public:
	//设置定时器（开始定时）
	void SetCurTickCount(void);
	//add by liuqy 20100817 for 得到的是微秒,同时可以返回64位的类型
	__int64 GetI64Duration(void);
	//获取定时开始到当前的时间差（微秒）
	unsigned long GetDuration(void);
	//获取当前定时开始到当前的时间差字符串表示（SS[MS]）
	MString GetDurationString(void);
public:
	//判断是否为空（没有使用过）
	bool IsEmpty(void);
	//设置为空
	void Empty(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//累加统计计数器，用于数据的（累加）统计计数，如对请求的统计：请求总量、请求频率（个/秒）、最大请求频率、发生时间
//注意：统计对象只能累加，适用于请求数量、丢包数量等等
class MCumulateStat
{
protected:
	__int64							m_RecordData;
	__int64							m_OldRecordData;
	unsigned long					m_lPreviousCount;
	MCounter						m_mStatTime;
	unsigned long					m_lLastFreq;
	unsigned long					m_lMaxFreq;
	MDateTime						m_mMaxTime;
	unsigned long					m_lPerStatTime;
protected:
	__inline void inner_stat(void);
public:
	//注意：构造时需要指定统计频率（单位秒），如果缺省，将设置为5秒统计一次
	MCumulateStat(void);
	MCumulateStat(unsigned long lFreq);
	virtual ~MCumulateStat();
public:
	//累加操作：如请求增加等
	void operator += (unsigned long lIn);
	//设置统计频率，单位秒
	void SetPreStatTime(unsigned long lTime);
public:
	//获取最近一次统计的频率（个[次]/秒）
	unsigned long GetFreq(void);
	//获取最大频率以及发生的时间
	unsigned long GetMaxFreq(MDateTime * mOutTime = NULL);
public:
	//获取最近一次统计比前一次统计增加的量
	unsigned long GetIncrement(void);
	//获取总的统计数量
	__int64 GetTotal(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
