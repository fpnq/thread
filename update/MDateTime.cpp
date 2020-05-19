//------------------------------------------------------------------------------------------------------------------------------
#include "MDateTime.h"
//------------------------------------------------------------------------------------------------------------------------------
MDateTime::MDateTime(void)
{
	m_stRecordData = 0;
}
//..............................................................................................................................
MDateTime::MDateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay)
{
	struct tm						sttime = {0};
	
	sttime.tm_year = sYear - 1900;
	sttime.tm_mon = sMonth - 1;
	sttime.tm_mday = sDay;
	sttime.tm_hour = 0;  
	sttime.tm_min = 0;
	sttime.tm_sec = 0;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}
//..............................................................................................................................
MDateTime::MDateTime(unsigned short sHour,unsigned short sMin,unsigned short sSec,unsigned short sMSec)
{
	struct tm						sttime = {0};
	
	sttime.tm_year = 70;
	sttime.tm_mon = 0;
	sttime.tm_mday = 1;
	sttime.tm_hour = sHour;  
	sttime.tm_min = sMin;
	sttime.tm_sec = sSec;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}
//..............................................................................................................................
MDateTime::MDateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay,unsigned short sHour,unsigned short sMin,unsigned short sSec)
{
	struct tm						sttime={0};


	
	
	sttime.tm_year = sYear - 1900;
	sttime.tm_mon = sMonth - 1;
	sttime.tm_mday = sDay;
	sttime.tm_hour = sHour;  
	sttime.tm_min = sMin;
	sttime.tm_sec = sSec;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}
//..............................................................................................................................
MDateTime::MDateTime(time_t stTime)
{
	m_stRecordData = stTime;
}
//..............................................................................................................................
MDateTime::MDateTime(const MDateTime & mTime)
{
	m_stRecordData = mTime.m_stRecordData;
}
//..............................................................................................................................
MDateTime::~MDateTime()
{

}
//..............................................................................................................................
void MDateTime::SetCurDateTime(void)
{
	m_stRecordData = time(NULL);
}
//..............................................................................................................................
int  MDateTime::SetDatetimeByString(const char * strIn)
{
	int								iyear,imon,iday,ihour,imin,isec;
	struct tm						sttime = {0};

	sscanf(strIn,"%04d-%02d-%02d %02d:%02d:%02d",&iyear,&imon,&iday,&ihour,&imin,&isec);

	sttime.tm_year = iyear - 1900;
	sttime.tm_mon = imon - 1;
	sttime.tm_mday = iday;
	sttime.tm_hour = ihour;  
	sttime.tm_min = imin;
	sttime.tm_sec = isec;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return(ERR_MDATETIME_INVALID);
	}

	return(1);
}
//..............................................................................................................................
int  MDateTime::SetDateByString(const char * strIn)
{
	int								iyear,imon,iday;
	struct tm						sttime = {0};

	
	sscanf(strIn,"%04d-%02d-%02d",&iyear,&imon,&iday);
	
	sttime.tm_year = iyear - 1900;
	sttime.tm_mon = imon - 1;
	sttime.tm_mday = iday;
	sttime.tm_hour = 0;  
	sttime.tm_min = 0;
	sttime.tm_sec = 0;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return(ERR_MDATETIME_INVALID);
	}
	
	return(1);
}
//..............................................................................................................................
int  MDateTime::SetTimeByString(const char * strIn)
{
	int								ihour,imin,isec;
	struct tm						sttime = {0};
	
	sscanf(strIn,"%02d:%02d:%02d",&ihour,&imin,&isec);
	
	sttime.tm_year = 70;
	sttime.tm_mon = 0;
	sttime.tm_mday = 1;
	sttime.tm_hour = ihour;  
	sttime.tm_min = imin;
	sttime.tm_sec = isec;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return(ERR_MDATETIME_INVALID);
	}
	
	return(1);
}
//..............................................................................................................................
MString MDateTime::DateTimeToString(void)
{
	char							tempbuf[256];

	#ifndef LINUXCODE

		struct tm				*	lptemptime;
	
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}

		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d %02d:%02d:%02d",
						lptemptime->tm_year + 1900,
						lptemptime->tm_mon + 1,
						lptemptime->tm_mday,
						lptemptime->tm_hour,
						lptemptime->tm_min,
						lptemptime->tm_sec	);
	#else

		struct tm					sttemptime = {0};

		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}

		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d %02d:%02d:%02d",
						sttemptime.tm_year + 1900,
						sttemptime.tm_mon + 1,
						sttemptime.tm_mday,
						sttemptime.tm_hour,
						sttemptime.tm_min,
						sttemptime.tm_sec	);
	#endif

	return(tempbuf);
}
//..............................................................................................................................
MString MDateTime::DateToString(void)
{
	char							tempbuf[256];
	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d",
						lptemptime->tm_year + 1900,
						lptemptime->tm_mon + 1,
						lptemptime->tm_mday	);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d %02d:%02d:%02d",
						sttemptime.tm_year + 1900,
						sttemptime.tm_mon + 1,
						sttemptime.tm_mday	);
	#endif
		
	return(tempbuf);
}
//..............................................................................................................................
MString MDateTime::TimeToString(void)
{
	char							tempbuf[256];
	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%02d:%02d:%02d",
						lptemptime->tm_hour,
						lptemptime->tm_min,
						lptemptime->tm_sec	);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%02d:%02d:%02d",
						sttemptime.tm_hour,
						sttemptime.tm_min,
						sttemptime.tm_sec	);
	#endif
	
	return(tempbuf);
}
//..............................................................................................................................
time_t  MDateTime::DateTimeToTimet(void)
{
	return(m_stRecordData);
}
//..............................................................................................................................
unsigned long MDateTime::DateToLong(void)
{	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(0);
		}

		return((lptemptime->tm_year + 1900) * 10000 + (lptemptime->tm_mon + 1) * 100 + lptemptime->tm_mday);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(0);
		}
		
		return((sttemptime.tm_year + 1900) * 10000 + (sttemptime.tm_mon + 1) * 100 + sttemptime.tm_mday);
	#endif
}
//..............................................................................................................................
unsigned long MDateTime::TimeToLong(void)
{	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(0);
		}

		return(lptemptime->tm_hour * 10000 + lptemptime->tm_min * 100 + lptemptime->tm_sec);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(0);
		}
		
		return(sttemptime.tm_hour * 10000 + sttemptime.tm_min * 100 + sttemptime.tm_sec);
	#endif
}
//..............................................................................................................................
MDateTime & MDateTime::operator = (const MDateTime & mIn)
{
	m_stRecordData = mIn.m_stRecordData;

	return(* this);
}
//..............................................................................................................................
int MDateTime::operator - (MDateTime & mIn)
{
	return((int)(m_stRecordData - mIn.m_stRecordData));
}
//..............................................................................................................................
MDateTime MDateTime::operator - (int iIn)
{
	return(MDateTime(m_stRecordData - iIn));
}
//..............................................................................................................................
MDateTime MDateTime::operator + (int iIn)
{
	return(MDateTime(m_stRecordData + iIn));
}
//..............................................................................................................................
MDateTime & MDateTime::operator += (int iIn)
{
	m_stRecordData += iIn;

	return(* this);
}
//..............................................................................................................................
MDateTime & MDateTime::operator -= (int iIn)
{
	m_stRecordData -= iIn;
	
	return(* this);
}
//..............................................................................................................................
long MDateTime::GetDuration(void)
{
	return((long)(time(NULL) - m_stRecordData));
}
//..............................................................................................................................
MString MDateTime::GetDurationString(void)
{
	register long				errorcode;
	char						tempbuf[256];

	errorcode = GetDuration();
	my_snprintf(tempbuf,256,"%02d:%02d:%02d",errorcode / 3500,(errorcode % 3600) / 60,errorcode % 60);

	return(tempbuf);
}
//..............................................................................................................................
int  MDateTime::GetDayOfWeek(void)
{	
	#ifndef LINUXCODE
	
		struct tm				*	lptemptime;

		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_wday);
	#else
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData, &sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_wday);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetDayOfYear(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_yday);
	#else
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData, &sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_yday);
	#endif
}
//..............................................................................................................................
int  MDateTime::DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay)
{
	assert(sYear != NULL);
	assert(sMonth != NULL);
	assert(sDay != NULL);

	#ifndef LINUXCODE

		struct tm				*	lptemptime;

		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}

		* sYear = lptemptime->tm_year + 1900;
		* sMonth = lptemptime->tm_mon + 1;
		* sDay = lptemptime->tm_mday;

		return(1);
	#else

		struct tm					sttemptime = {0};

		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}

		* sYear = sttemptime.tm_year + 1900;
		* sMonth = sttemptime.tm_mon + 1;
		* sDay = sttemptime.tm_mday;
		
		return(1);
	#endif
}
//..............................................................................................................................
int  MDateTime::DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec)
{
	assert(sHour != NULL);
	assert(sMin != NULL);
	assert(sSec != NULL);
	
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		* sHour = lptemptime->tm_hour;
		* sMin = lptemptime->tm_min;
		* sSec = lptemptime->tm_sec;
		
		return(1);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		* sHour = sttemptime.tm_hour;
		* sMin = sttemptime.tm_min;
		* sSec = sttemptime.tm_sec;
		
		return(1);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetYear(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_year+1900);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_year+1900);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetMonth(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_mon+1);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_mon+1);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetDay(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_mday);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_mday);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetHour(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_hour);
	#else
		
		struct tm					sttemptime= {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_hour);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetMinute(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_min);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_min);
	#endif
}
//..............................................................................................................................
int  MDateTime::GetSecond(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(lptemptime->tm_sec);
	#else
		
		struct tm					sttemptime = {0};
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_sec);
	#endif
}
//..............................................................................................................................
bool MDateTime::operator == (const MDateTime & mIn)
{
	if ( m_stRecordData == mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::operator != (const MDateTime & mIn)
{
	if ( m_stRecordData != mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::operator >  (const MDateTime & mIn)
{
	if ( m_stRecordData > mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::operator >= (const MDateTime & mIn)
{
	if ( m_stRecordData >= mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::operator <  (const MDateTime & mIn)
{
	if ( m_stRecordData < mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::operator <= (const MDateTime & mIn)
{
	if ( m_stRecordData <= mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool MDateTime::IsEmpty(void)
{
	if ( m_stRecordData == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

//..............................................................................................................................
void MDateTime::Empty(void)
{
	m_stRecordData = 0;
}
//..............................................................................................................................
MDateTime MDateTime::Now(void)
{
	return(MDateTime(time(NULL)));
}
//------------------------------------------------------------------------------------------------------------------------------
MCounter::MCounter(void)
{
#ifndef LINUXCODE
	m_lRecordData = 0;
#else
	memset(&m_tp, 0, sizeof(struct timeb));
#endif
}
//..............................................................................................................................
MCounter::~MCounter()
{

}
//..............................................................................................................................
void MCounter::SetCurTickCount(void)
{
	#ifndef LINUXCODE
		//Windows版本
		m_lRecordData = ::GetTickCount();
	#else
		/*
		 *	MODIFY BY GUOGUO 20100709
		 *	其实我建议WINDOWS 版也使用ftime来获取时间,本来这里无非就是为了统计毫秒数
		 *	为什么要用WINAPI函数 ::GetTickCount,完全可以使用ANSI的C函数ftime来进行
		 *	是为了ifdef 好看吗?
		 *					--	GUOGUO 20100709
		 */
		ftime(&m_tp);
	#endif
}
//..............................................................................................................................
unsigned long MCounter::GetDuration(void)
{
	#ifndef LINUXCODE
		//Windows版本
		return(::GetTickCount() - m_lRecordData);
	#else
		//Linux版本
		/*
		 *	MODIFY BY GUOGUO 20100709
		 *	其实我建议WINDOWS 版也使用ftime来获取时间,本来这里无非就是为了统计毫秒数
		 *	为什么要用WINAPI函数 ::GetTickCount,完全可以使用ANSI的C函数ftime来进行
		 *	是为了ifdef 好看吗?
		 *							--	GUOGUO 20100709
		 */
		struct timeb	tp;
		ftime(&tp);

		return ( (tp.time - m_tp.time) * 1000 + tp.millitm - m_tp.millitm);
	#endif
}
//..............................................................................................................................
MString MCounter::GetDurationString(void)
{
	char						tempbuf[256];
	register unsigned long		errorcode;

	errorcode = GetDuration();
	my_snprintf(tempbuf,256,"%d[%d]",errorcode / 1000,errorcode % 1000);
	
	return(tempbuf);
}
//..............................................................................................................................
bool MCounter::IsEmpty(void)
{
#ifndef LINUXCODE
	if ( m_lRecordData == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
#else
	struct timeb	tp;

	memset(&tp, 0, sizeof(struct timeb));
	if(memcmp(&m_tp, &tp, sizeof(struct timeb)) == 0)
		return true;
	else
		return false;
#endif
}
//..............................................................................................................................
void MCounter::Empty(void)
{
#ifndef LINUXCODE
	m_lRecordData = 0;
#else
	memset(&m_tp, 0, sizeof(struct timeb));
#endif
}
//------------------------------------------------------------------------------------------------------------------------------
MExactitudeCount::MExactitudeCount(void)
{
	#ifndef LINUXCODE

		LARGE_INTEGER					sttemp;
	
		QueryPerformanceFrequency(&sttemp);
		m_dCpuFreq = (double)sttemp.QuadPart;

	#else
	#endif
}
//..............................................................................................................................
MExactitudeCount::~MExactitudeCount()
{

}
//..............................................................................................................................
void MExactitudeCount::SetCurTickCount(void)
{
	#ifndef LINUXCODE
	
		LARGE_INTEGER					sttemp;
		
		QueryPerformanceCounter(&sttemp);
		m_RecordData = sttemp.QuadPart;

	#else

		struct timeval 					sttemp;
		
		gettimeofday(&sttemp,NULL);
		//modify by liuqy 20100817 for 为保证数据为溢出，得先赋给64位再运算
		//m_RecordData = sttemp.tv_sec * 1000000 + sttemp.tv_usec;
		m_RecordData = sttemp.tv_sec ;
		m_RecordData = m_RecordData * 1000000 + sttemp.tv_usec;
		//end modify
	#endif
}
//modify by liuqy 20100817 for 得到的是微秒,同时可以返回64位的类型
__int64 MExactitudeCount::GetI64Duration(void)
{
	#ifndef LINUXCODE

		LARGE_INTEGER					sttemp;
		
		QueryPerformanceCounter(&sttemp);
		//modify by liuqy 20100816 for 使用时间应该为1000000
		//return((unsigned long)((1000 * (double)(sttemp.QuadPart - m_RecordData)) / m_dCpuFreq));
		return (__int64)((1000000 * (double)(sttemp.QuadPart - m_RecordData)) / m_dCpuFreq);
	#else

		struct timeval 					sttemp;
		__int64 i64Ret;
		gettimeofday(&sttemp,NULL);
		i64Ret = sttemp.tv_sec;
		i64Ret =i64Ret * 1000000 + sttemp.tv_usec;
		return((i64Ret - m_RecordData) / 1000);

	#endif

}

//..............................................................................................................................
unsigned long MExactitudeCount::GetDuration(void)
{
	#ifndef LINUXCODE

		LARGE_INTEGER					sttemp;
		
		QueryPerformanceCounter(&sttemp);
		//modify by liuqy 20100816 for 使用时间应该为1000000
		//return((unsigned long)((1000 * (double)(sttemp.QuadPart - m_RecordData)) / m_dCpuFreq));
		return (unsigned long)((1000000 * (double)(sttemp.QuadPart - m_RecordData)) / m_dCpuFreq);
	#else

		struct timeval 					sttemp;
		//modify by liuqy 20100817 for 为保证数据为溢出，得先赋给64位再运算
		__int64 i64Ret;

		gettimeofday(&sttemp,NULL);
		//return(((sttemp.tv_sec * 1000000+ sttemp.tv_usec) - m_RecordData) / 1000);
		i64Ret = sttemp.tv_sec;
		i64Ret =i64Ret * 1000000 + sttemp.tv_usec;
		return((i64Ret - m_RecordData) / 1000);
		//end modify by liuqy 

	#endif
}
//..............................................................................................................................
MString MExactitudeCount::GetDurationString(void)
{
	char						tempbuf[256];
	register unsigned long		errorcode;
	
	errorcode = GetDuration();
	my_snprintf(tempbuf,256,"%d[%d]",errorcode / 1000,errorcode % 1000);
	
	return(tempbuf);
}
//..............................................................................................................................
bool MExactitudeCount::IsEmpty(void)
{
	if ( m_RecordData == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
void MExactitudeCount::Empty(void)
{
	m_RecordData = 0;
}
//------------------------------------------------------------------------------------------------------------------------------
MCumulateStat::MCumulateStat(void)
{
	m_RecordData = 0;
	m_OldRecordData = 0;
	m_lPreviousCount = 0;
	m_mStatTime.SetCurTickCount();
	
	m_lLastFreq = 0;
	m_lMaxFreq = 0;

	m_lPerStatTime = 1 * 1000;
}
//..............................................................................................................................
MCumulateStat::MCumulateStat(unsigned long lFreq)
{
	m_RecordData = 0;
	m_OldRecordData = 0;
	m_lPreviousCount = 0;
	m_mStatTime.SetCurTickCount();
	
	m_lLastFreq = 0;
	m_lMaxFreq = 0;
	
	if ( lFreq == 0 )
	{
		assert(0);
		lFreq = 1;
	}

	m_lPerStatTime = lFreq * 1000;
}
//..............................................................................................................................
MCumulateStat::~MCumulateStat()
{

}
//..............................................................................................................................
void MCumulateStat::inner_stat(void)
{
	register unsigned long				errorcode;

	errorcode = m_mStatTime.GetDuration();
	if ( errorcode >= m_lPerStatTime )
	{
		//开始统计操作
		m_mStatTime.SetCurTickCount();

		m_lPreviousCount = (unsigned long)(m_RecordData - m_OldRecordData);
		m_lLastFreq = (m_lLastFreq * 2 + (m_lPreviousCount * 1000 / errorcode) * 8) / 10;
		if ( m_lLastFreq > m_lMaxFreq )
		{
			m_lMaxFreq = m_lLastFreq;
			m_mMaxTime.SetCurDateTime();
		}
		m_OldRecordData = m_RecordData;
	}
}
//..............................................................................................................................
void MCumulateStat::operator += (unsigned long lIn)
{
	inner_stat();

	m_RecordData += lIn;
}
//..............................................................................................................................
void MCumulateStat::SetPreStatTime(unsigned long lTime)
{
	if ( lTime == 0 )
	{
		assert(0);
		lTime = 20;
	}
	
	m_lPerStatTime = lTime * 1000;
}
//..............................................................................................................................
unsigned long MCumulateStat::GetFreq(void)
{
	inner_stat();

	return(m_lLastFreq);
}
//..............................................................................................................................
unsigned long MCumulateStat::GetMaxFreq(MDateTime * mOutTime)
{
	inner_stat();

	return(m_lMaxFreq);
}
//..............................................................................................................................
unsigned long MCumulateStat::GetIncrement(void)
{
	inner_stat();

	return(m_lPreviousCount);
}
//..............................................................................................................................
__int64 MCumulateStat::GetTotal(void)
{
	inner_stat();

	return(m_RecordData);
}
//------------------------------------------------------------------------------------------------------------------------------
