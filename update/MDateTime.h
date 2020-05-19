//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ�ʱ�����ڵ�Ԫ
//��Ԫ��������Ҫ�����ʱ��������صĺ���
//�������ڣ�2007.3.15
//������Ա��¬��Զ
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
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
//ʱ�������࣬��ȷ���룬��Χ��1970-1-1 8:00:00 �� 2038-1-19 3:14:07
//ע�⣺���Ϸ�Χ����32λ����ϵͳ�£�64λ����ϵͳ��û���������
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
	//��ȡ��ǰ�����ں�ʱ��
	void SetCurDateTime(void);
	//���ַ����л�ȡ���ں�ʱ�䣬�ַ�����ʽλ2005-05-29 13:05:12
	int  SetDatetimeByString(const char * strIn);
	int  SetDateByString(const char * strIn);
	int  SetTimeByString(const char * strIn);
public:
	//ת���ַ�������ʽΪ2005-05-29 13:05:12
	MString DateTimeToString(void);
	MString DateToString(void);
	MString TimeToString(void);
	time_t  DateTimeToTimet(void);
	unsigned long DateToLong(void);
	unsigned long TimeToLong(void);
public:
	//���������
	MDateTime & operator = (const MDateTime & mIn);
	//�������������ȥһ��ʱ�䣬�õ�����ʱ��֮���λΪ��
	int operator - (MDateTime & mIn);
	//�������������ȥһ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	MDateTime operator - (int iIn);
	//���������������һ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	MDateTime operator + (int iIn);
	//���������������һ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	MDateTime & operator += (int iIn);
	//�������������ȥһ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	MDateTime & operator -= (int iIn);
public:
	//��ȡ��ʱ�������ʱ��Ĳ�ֵ����λΪ��
	long GetDuration(void);
	//��ȡ��ʱ�������ʱ��Ĳ�ֵ�����ַ�����ʾ����ʽΪHH:MM:SS
	MString GetDurationString(void);
public:
	//��ȡ��ǰʱ����λ��һ�ܻ�һ��ĵڼ��죬ע�⣺������Ϊ0
	int  GetDayOfWeek(void);
	int  GetDayOfYear(void);
public:
	//�ֽ��ʱ�������
	int  DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay);
	int  DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec);
	int  GetYear(void);
	int  GetMonth(void);
	int  GetDay(void);
	int  GetHour(void);
	int  GetMinute(void);
	int  GetSecond(void);
public:
	//�Ƚ��ж�
	bool operator == (const MDateTime & mIn);
	bool operator != (const MDateTime & mIn);
	bool operator >  (const MDateTime & mIn);
	bool operator >= (const MDateTime & mIn);
	bool operator <  (const MDateTime & mIn);
	bool operator <= (const MDateTime & mIn);
public:
	//�ж��Ƿ�Ϊ�գ�Ϊ�ձ�ʾ����û�����ù������ʱ��
	bool IsEmpty(void);
	void Empty(void);
public:
	//��ȡ��ǰʱ���MDateTime����
	static MDateTime Now(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//��ʱ������ȷ�����루ʵ��ֻ�ܹ���ȷ��30--50�������ң����ݲ���ϵͳ������
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
	//���ö�ʱ������ʼ��ʱ��
	void SetCurTickCount(void);
	//��ȡ��ʱ��ʼ����ǰ��ʱ�����룩
	unsigned long GetDuration(void);
	//��ȡ��ǰ��ʱ��ʼ����ǰ��ʱ����ַ�����ʾ��SS[MS]��
	MString GetDurationString(void);
public:
	//�ж��Ƿ�Ϊ�գ�û��ʹ�ù���
	bool IsEmpty(void);
	//����Ϊ��
	void Empty(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//��ȷ���߾��ȣ������������ؼ�����λΪ΢�룬���Ϊ0.5΢��
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
	//���ö�ʱ������ʼ��ʱ��
	void SetCurTickCount(void);
	//add by liuqy 20100817 for �õ�����΢��,ͬʱ���Է���64λ������
	__int64 GetI64Duration(void);
	//��ȡ��ʱ��ʼ����ǰ��ʱ��΢�룩
	unsigned long GetDuration(void);
	//��ȡ��ǰ��ʱ��ʼ����ǰ��ʱ����ַ�����ʾ��SS[MS]��
	MString GetDurationString(void);
public:
	//�ж��Ƿ�Ϊ�գ�û��ʹ�ù���
	bool IsEmpty(void);
	//����Ϊ��
	void Empty(void);
};
//------------------------------------------------------------------------------------------------------------------------------
//�ۼ�ͳ�Ƽ��������������ݵģ��ۼӣ�ͳ�Ƽ�������������ͳ�ƣ���������������Ƶ�ʣ���/�룩���������Ƶ�ʡ�����ʱ��
//ע�⣺ͳ�ƶ���ֻ���ۼӣ��������������������������ȵ�
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
	//ע�⣺����ʱ��Ҫָ��ͳ��Ƶ�ʣ���λ�룩�����ȱʡ��������Ϊ5��ͳ��һ��
	MCumulateStat(void);
	MCumulateStat(unsigned long lFreq);
	virtual ~MCumulateStat();
public:
	//�ۼӲ��������������ӵ�
	void operator += (unsigned long lIn);
	//����ͳ��Ƶ�ʣ���λ��
	void SetPreStatTime(unsigned long lTime);
public:
	//��ȡ���һ��ͳ�Ƶ�Ƶ�ʣ���[��]/�룩
	unsigned long GetFreq(void);
	//��ȡ���Ƶ���Լ�������ʱ��
	unsigned long GetMaxFreq(MDateTime * mOutTime = NULL);
public:
	//��ȡ���һ��ͳ�Ʊ�ǰһ��ͳ�����ӵ���
	unsigned long GetIncrement(void);
	//��ȡ�ܵ�ͳ������
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
