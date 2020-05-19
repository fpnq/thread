//------------------------------------------------------------------------------------------------------------------------------
//单元名称：字符串单元
//单元描述：主要处理字符串相关操作
//创建日期：2007.3.14
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MStringH__
#define __MEngine_MStringH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MTypeDefine.h"
//------------------------------------------------------------------------------------------------------------------------------
#define MSTRING_DEFAULT_SIZE					256				//缺省字符串长度
//..............................................................................................................................
class MString
{
protected:
	char				*	m_lpszStringPtr;
	char					m_szDefaultBuf[MSTRING_DEFAULT_SIZE];
	unsigned long			m_lMaxSize;
protected:
	//初始化字符串各个成员结构
	__inline void inner_initstring(void);
	//给字符串重新分配空间，如果空间已经满足，不再分配
	__inline int  inner_allocstring(unsigned long lSize);
public:
	MString(void);
	MString(char			cIn);
	MString(short			wIn);
	MString(unsigned short	wIn);
	MString(int				iIn);
	MString(unsigned int	iIn);
	MString(long			lIn);
	MString(unsigned long	lIn);
	MString(float			fIn);
	MString(double			dIn);
	MString(const char *	strIn);
	MString(const MString  &mIn);
	MString(char cValue,unsigned long lCount);
	MString(bool			bIn);
	virtual ~MString();
public:
	//运算符重载，复制运算
	MString & operator = (const MString & strIn);
	MString & operator = (const char * strIn);
	
	//运算符重载，连接运算，并且修改自身
	MString & operator += (const MString & strIn);
	MString & operator += (const char * strIn);
	
	//运算符重载，复制运算，不影响自身
	MString   operator + (const MString & strIn);
	MString   operator + (const char * strIn);
	friend MString operator + (const char * strIn1,const MString & strIn2);
public:
	//运算符重载，比较运算
	friend bool operator == (const MString & strIn1,const MString & strIn2);
	friend bool operator == (const char * strIn1,const MString & strIn2);
	friend bool operator == (const MString & strIn1,const char * strIn2);
	friend bool operator != (const MString & strIn1,const MString & strIn2);
	friend bool operator != (const char * strIn1,const MString & strIn2);
	friend bool operator != (const MString & strIn1,const char * strIn2);
	friend bool operator >  (const MString & strIn1,const MString & strIn2);
	friend bool operator >  (const char * strIn1,const MString & strIn2);
	friend bool operator >  (const MString & strIn1,const char * strIn2);
	friend bool operator >= (const MString & strIn1,const MString & strIn2);
	friend bool operator >= (const char * strIn1,const MString & strIn2);
	friend bool operator >= (const MString & strIn1,const char * strIn2);
	friend bool operator <  (const MString & strIn1,const MString & strIn2);
	friend bool operator <  (const char * strIn1,const MString & strIn2);
	friend bool operator <  (const MString & strIn1,const char * strIn2);
	friend bool operator <= (const MString & strIn1,const MString & strIn2);
	friend bool operator <= (const char * strIn1,const MString & strIn2);
	friend bool operator <= (const MString & strIn1,const char * strIn2);
public:
	//转换char *类型
	const char * c_str(void);
	
	//获取字符串长度
	unsigned long GetLength(void);
	
	//转换字符串位大写字符
	void Upper(void);
	
	//转换字符串位小写字符
	void Lower(void);
	
	//判断字符串是否为空，即==""
	bool IsEmpty(void);
	
	//清空字符串
	void Empty(void);
	
	//格式化输入字符串，注意：由字符串长度的限制，如果是却省的（未分配的，则为MSTRING_DEFAULT_SIZE）
	void Format(const char * szIn,...);
	
	//获取指定位置的字符，重载操作符，越界返回0
	char operator [] (unsigned int iNo);
	
	//设置指定位置的字符，参数越界，返回错误
	int  SetAt(unsigned int iNo,char cValue);
	
	//获取指定中间位置的字符串，没有或越界返回""
	MString Mid(unsigned int iFirst,unsigned int iCount);

	//获取指定左边位置的字符串，没有或越界返回""
	MString Left(unsigned int iCount);

	//获取指定右边位置的字符串，没有或越界返回""
	MString Right(unsigned int iCount);

	//获取第一个指定匹配字符串位置前的字符串，如果没有匹配返回""
	MString GetMatchFrontString(MString strMatch);

	//获取第一个指定匹配字符串位置后的字符串，如果没有匹配返回""
	MString GetMatchBackString(MString strMatch);

	//获取第一个指定匹配字符串位置前后的字符串，如果没有匹配返回""
	int  GetMatchString(MString strMatch,MString * strFront,MString * strBack);

	//查找字符串，返回字符串所在位置，从0开始记数，失败返回<0
	int  StringPosition(MString strFind);

	//清除左边开始的连续空格、TAB字符 
	void TrimLeft(void);

	//清除右边开始的连续空格、TAB字符
	void TrimRight(void);

	//将连续的空格或TAB字符转换成为单一的空格，并且过滤掉左边和右边的空格或TAB字符，即如"    A   B  C D  "转换成为"A B C D"
	MString TrimMiddleToSpace(void);
public:
	//类型转换函数
	long ToLong(void);
	unsigned long ToULong(void);
	double ToDouble(void);
	bool ToBool(void);
public:
	//格式化输入字符串，注意：有MSTRING_DEFAULT_SIZE * 10字符长度的限制
	static MString FormatString(const char * strFormat,...);

	//拷贝字符串：为了修正标准strncpy函数在操作iCount时不能置0的问题
	static char * strncpy(char * strDest,const char * strSource,size_t iCount);
	
	//连接字符串：为了修正标准strncat函数在操作iCount时不能置0的问题
	static char * strncat(char * strDest,const char * strSource,size_t iCount);

	//打印字符串：为了修正标准snprintf函数在操作iCount时不能置0的问题
	static void _snprintf(char * lpszBuffer,size_t lCount,const char * lpszFormat,...);
	
	//类型转换函数
	static long ToLong(const char * strValue);
	static unsigned long ToULong(const char * strValue);
	static double ToDouble(const char * strValue);
	static bool ToBool(const char * strValue);

	//清除左边或右边开始的连续空格、TAB字符
	static void TrimLeft(char * strIn);
	static void TrimRight(char * strIn);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
