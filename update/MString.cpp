//------------------------------------------------------------------------------------------------------------------------------
#include "MString.h"
#include "MErrorCode.h"
//------------------------------------------------------------------------------------------------------------------------------
void MString::inner_initstring(void)
{
	m_lpszStringPtr = m_szDefaultBuf;
	m_lMaxSize = MSTRING_DEFAULT_SIZE;
	m_lpszStringPtr[0] = 0;
}
//..............................................................................................................................
int  MString::inner_allocstring(unsigned long lSize)
{
	register unsigned long			errorcode;
	char						*	lptempptr;

	assert(m_lpszStringPtr != NULL);

	//判断是否需要重新分配空间
	errorcode = lSize + 1;
	if ( m_lMaxSize >= errorcode )
	{
		return(1);
	}

	//调整分配空间大小，保留原有空间
	errorcode = ((errorcode >> 8) + 1) << 8;
	lptempptr = m_lpszStringPtr;
	
	//开始分配
	//m_lpszStringPtr = new char[errorcode];
	//modify by liuqy 20150630 需要将new 更换为malloc 
	m_lpszStringPtr =(char*) malloc(errorcode);
	if ( m_lpszStringPtr == NULL )
	{
		//没有足够的内存可以分配
		assert(0);
		m_lpszStringPtr = lptempptr;

		return(ERR_PUBLIC_NOMEMORY);
	}

	//拷贝原有空间数据
	my_strncpy(m_lpszStringPtr,lptempptr,m_lMaxSize);
	m_lMaxSize = errorcode;

	//清除原有空间
	if ( lptempptr != m_szDefaultBuf )
	{
		//modify by liuqy 20150630 需要将new 更换为malloc 
		//delete [] lptempptr;
		free(lptempptr);
	}

	return(1);
}
//..............................................................................................................................
MString::MString(void)
{
	inner_initstring();
}
//..............................................................................................................................
MString::MString(char			cIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%c",cIn);
}
//..............................................................................................................................
MString::MString(short			wIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%d",wIn);
}
//..............................................................................................................................
MString::MString(unsigned short	wIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%u",wIn);
}
//..............................................................................................................................
MString::MString(int				iIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%d",iIn);
}
//..............................................................................................................................
MString::MString(unsigned int	iIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%u",iIn);
}
//..............................................................................................................................
MString::MString(long			lIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%d",lIn);
}
//..............................................................................................................................
MString::MString(unsigned long	lIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%u",lIn);
}
//..............................................................................................................................
MString::MString(float			fIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%f",fIn);
}
//..............................................................................................................................
MString::MString(double			dIn)
{
	inner_initstring();
	my_snprintf(m_lpszStringPtr,m_lMaxSize,"%f",dIn);
}
//..............................................................................................................................
MString::MString(const char *	strIn)
{
	assert(strIn != NULL);

	inner_initstring();
	inner_allocstring(strlen(strIn));

	my_strncpy(m_lpszStringPtr,strIn,m_lMaxSize);
}
//..............................................................................................................................
MString::MString(const MString  &mIn)
{
	inner_initstring();
	inner_allocstring(strlen(mIn.m_lpszStringPtr));

	my_strncpy(m_lpszStringPtr,mIn.m_lpszStringPtr,m_lMaxSize);
}
//..............................................................................................................................
MString::MString(char cValue,unsigned long lCount)
{
	inner_initstring();
	inner_allocstring(lCount);

	memset(m_lpszStringPtr,cValue,m_lMaxSize);
	m_lpszStringPtr[m_lMaxSize - 1] = 0;
}
//..............................................................................................................................
MString::MString(bool			bIn)
{
	inner_initstring();

	if ( bIn == true )
	{
		my_snprintf(m_lpszStringPtr,m_lMaxSize,"true");
	}
	else
	{
		my_snprintf(m_lpszStringPtr,m_lMaxSize,"false");
	}
}

//..............................................................................................................................
MString::~MString()
{
	if ( m_lpszStringPtr != m_szDefaultBuf )
	{
		//modify by liuqy 20150630 需要将new 更换为malloc 
		//delete [] m_lpszStringPtr;
		free(m_lpszStringPtr);
		m_lpszStringPtr = m_szDefaultBuf;
	}
	m_lMaxSize = MSTRING_DEFAULT_SIZE;
}
//..............................................................................................................................
MString & MString::operator = (const MString & strIn)
{
	inner_allocstring(strlen(strIn.m_lpszStringPtr));
	my_strncpy(m_lpszStringPtr,strIn.m_lpszStringPtr,m_lMaxSize);

	return(* this);
}
//..............................................................................................................................
MString & MString::operator = (const char * strIn)
{
	assert(strIn != NULL);

	inner_allocstring(strlen(strIn));
	my_strncpy(m_lpszStringPtr,strIn,m_lMaxSize);

	return(* this);
}
//..............................................................................................................................
MString & MString::operator += (const MString & strIn)
{
	inner_allocstring(strlen(m_lpszStringPtr) + strlen(strIn.m_lpszStringPtr));
	my_strncat(m_lpszStringPtr,strIn.m_lpszStringPtr,m_lMaxSize);

	return(* this);
}
//..............................................................................................................................
MString & MString::operator += (const char * strIn)
{
	assert(strIn != NULL);

	inner_allocstring(strlen(m_lpszStringPtr) + strlen(strIn));
	my_strncat(m_lpszStringPtr,strIn,m_lMaxSize);

	return(* this);
}
//..............................................................................................................................
MString   MString::operator + (const MString & strIn)
{
	MString					mtempstring(m_lpszStringPtr);

	mtempstring += strIn;

	return(mtempstring);
}
//..............................................................................................................................
MString   MString::operator + (const char * strIn)
{
	MString					mtempstring(m_lpszStringPtr);
	
	mtempstring += strIn;
	
	return(mtempstring);
}
//..............................................................................................................................
MString operator + (const char * strIn1,const MString & strIn2)
{
	MString					mtempstring(strIn1);

	mtempstring += strIn2;

	return(mtempstring);
}
//..............................................................................................................................
bool operator == (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator == (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator == (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator != (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) != 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator != (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) != 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator != (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) != 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >  (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) > 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >  (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) > 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >  (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) > 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >= (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) >= 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >= (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) >= 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator >= (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) >= 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <  (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) < 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <  (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) < 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <  (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) < 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <= (const MString & strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2.m_lpszStringPtr) <= 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <= (const char * strIn1,const MString & strIn2)
{
	if ( strcmp(strIn1,strIn2.m_lpszStringPtr) <= 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
bool operator <= (const MString & strIn1,const char * strIn2)
{
	if ( strcmp(strIn1.m_lpszStringPtr,strIn2) < 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
const char * MString::c_str(void)
{
	return(m_lpszStringPtr);
}
//..............................................................................................................................
unsigned long MString::GetLength(void)
{
	return(strlen(m_lpszStringPtr));
}
//..............................................................................................................................
void MString::Upper(void)
{
	#ifndef LINUXCODE
		_strupr(m_lpszStringPtr);
	#else
		register unsigned int			i;
		
		for ( i=0;i<strlen(m_lpszStringPtr);i++ )
		{
			if ( m_lpszStringPtr[i] >= 'a' && m_lpszStringPtr[i] <= 'z' )
			{
				m_lpszStringPtr[i] -= 32;	
			}
		}
	#endif
}
//..............................................................................................................................
void MString::Lower(void)
{
	#ifndef LINUXCODE
		_strlwr(m_lpszStringPtr);
	#else
		register unsigned int			i;
		
		for ( i=0;i<strlen(m_lpszStringPtr);i++ )
		{
			if ( m_lpszStringPtr[i] >= 'A' && m_lpszStringPtr[i] <= 'Z' )
			{
				m_lpszStringPtr[i] += 32;
			}
		}
	#endif
}
//..............................................................................................................................
bool MString::IsEmpty(void)
{
	assert(m_lpszStringPtr != NULL);

	if ( m_lpszStringPtr[0] == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
void MString::Empty(void)
{
	assert(m_lpszStringPtr != NULL);

	m_lpszStringPtr[0] = 0;
}
//..............................................................................................................................
void MString::Format(const char * szIn,...)
{
	va_list						smarker;
	
	va_start(smarker,szIn);
	_vsnprintf(m_lpszStringPtr,m_lMaxSize,szIn,smarker);
	va_end( smarker );
}
//..............................................................................................................................
char MString::operator [] (unsigned int iNo)
{
	if ( iNo >= m_lMaxSize )
	{
		return(0);
	}
	else
	{
		return(m_lpszStringPtr[iNo]);
	}
}
//..............................................................................................................................
int  MString::SetAt(unsigned int iNo,char cValue)
{
	if ( iNo >= m_lMaxSize )
	{
		assert(0);
		return(ERR_PUBLIC_SLOPOVER);
	}

	m_lpszStringPtr[iNo] = cValue;

	return(1);
}
//..............................................................................................................................
MString MString::Mid(unsigned int iFirst,unsigned int iCount)
{
	register int				errorcode;
	MString						mtempstring;

	errorcode = strlen(m_lpszStringPtr);
	if ( (int)(iFirst + iCount) > errorcode )
	{
		return("");
	}

	if ( mtempstring.inner_allocstring(iCount + 1) < 0 )
	{
		return("");
	}

	memcpy(mtempstring.m_lpszStringPtr,m_lpszStringPtr + iFirst,iCount);
	mtempstring.m_lpszStringPtr[iCount] = 0;

	return(mtempstring);
}
//..............................................................................................................................
MString MString::Left(unsigned int iCount)
{
	register int				errorcode;
	MString						mtempstring;

	errorcode = strlen(m_lpszStringPtr);
	if ( (int)iCount >= errorcode )
	{
		return("");
	}

	if ( mtempstring.inner_allocstring(iCount + 1) < 0 )
	{
		return("");
	}

	memcpy(mtempstring.m_lpszStringPtr,m_lpszStringPtr,iCount);
	mtempstring.m_lpszStringPtr[iCount] = 0;

	return(mtempstring);
}
//..............................................................................................................................
MString MString::Right(unsigned int iCount)
{
	register int				errorcode;
	MString						mtempstring;
	
	errorcode = strlen(m_lpszStringPtr);
	if ( (int)iCount == errorcode )
	{
		//tangj 等于的时候就原样返回.
		mtempstring=m_lpszStringPtr;
		return mtempstring;
	}
	if ( (int)iCount > errorcode )
	{
		return("");
	}

	if ( mtempstring.inner_allocstring(iCount + 1) < 0 )
	{
		return("");
	}
	// 截掉左边iCount个字符，anzl 20090815修改
	// tangj 恢复right 正常的含义, 涉及的地方只有 节假日读取和两个期货的附加数据截取字符的地方,已调整
	memcpy(mtempstring.m_lpszStringPtr,m_lpszStringPtr + errorcode - iCount, iCount );
	mtempstring.m_lpszStringPtr[iCount] = 0;

//	memcpy(mtempstring.m_lpszStringPtr,m_lpszStringPtr + iCount, errorcode-iCount );
//	mtempstring.m_lpszStringPtr[errorcode-iCount] = 0;
	
	return(mtempstring);
}
//..............................................................................................................................
MString MString::GetMatchFrontString(MString strMatch)
{
	register char			*	lptempptr;
	MString						mtempstring;
	register unsigned long		errorcode;

	if ( (lptempptr = strstr(m_lpszStringPtr,strMatch.c_str())) == NULL )
	{
		return("");
	}

	errorcode = (unsigned int)(lptempptr - m_lpszStringPtr);
	if( errorcode == 0 )
	{
		return strMatch;
	}
	
	if ( mtempstring.inner_allocstring(errorcode + 1) < 0 )
	{
		return("");
	}

	memcpy(mtempstring.m_lpszStringPtr,m_lpszStringPtr,errorcode);
	mtempstring.m_lpszStringPtr[errorcode] = 0;
	
	return(mtempstring);
}
//..............................................................................................................................
MString MString::GetMatchBackString(MString strMatch)
{
	register char			*	lptempptr;
	MString						mtempstring;
	register unsigned long		errorcode;

	if ( (lptempptr = strstr(m_lpszStringPtr,strMatch.c_str())) == NULL )
	{
		return("");
	}

	errorcode = (unsigned int)(strlen(m_lpszStringPtr) - (lptempptr - m_lpszStringPtr) - strMatch.GetLength());
	
	if ( mtempstring.inner_allocstring(errorcode + 1) < 0 )
	{
		return("");
	}

	memcpy(mtempstring.m_lpszStringPtr,lptempptr + strMatch.GetLength(),errorcode);
	mtempstring.m_lpszStringPtr[errorcode] = 0;

	return(mtempstring);
}
//..............................................................................................................................
int  MString::GetMatchString(MString strMatch,MString * strFront,MString * strBack)
{
	register char			*	lptempptr;
	register unsigned long		errorcode;

	if ( (lptempptr = strstr(m_lpszStringPtr,strMatch.c_str())) == NULL )
	{
		return(ERR_MSTRING_NOMATCH);
	}

	errorcode = (unsigned long)(lptempptr - m_lpszStringPtr);
	if ( strFront->inner_allocstring(errorcode + 1) < 0 )
	{
		return(ERR_PUBLIC_NOMEMORY);
	}
	memcpy(strFront->m_lpszStringPtr,m_lpszStringPtr,errorcode);
	strFront->m_lpszStringPtr[errorcode] = 0;

	errorcode = (unsigned long)(strlen(m_lpszStringPtr) - (lptempptr - m_lpszStringPtr) - strMatch.GetLength());
	if ( strBack->inner_allocstring(errorcode + 1) < 0 )
	{
		return(ERR_MSTRING_NOMATCH);
	}
	memcpy(strBack->m_lpszStringPtr,lptempptr + strMatch.GetLength(),errorcode);
	strBack->m_lpszStringPtr[errorcode] = 0;

	return(1);
}
//..............................................................................................................................
int  MString::StringPosition(MString strFind)
{
	register char			*	lptempptr;

	if ( (lptempptr = strstr(m_lpszStringPtr,strFind.c_str())) == NULL )
	{
		return(ERR_MSTRING_NOMATCH);
	}

	return(lptempptr - m_lpszStringPtr);
}
//..............................................................................................................................
void MString::TrimLeft(void)
{
	register int				i;
	register int				isize;
	register int				ioffset = 0;

	isize = strlen(m_lpszStringPtr);
	ioffset = isize;

	for ( i=0;i<isize;i++ )
	{
		if (	m_lpszStringPtr[i] != ' ' && m_lpszStringPtr[i] != 0x09 && m_lpszStringPtr[i] != 0x0D
			 &&	m_lpszStringPtr[i] != 0x0A )
		{
			ioffset = i;
			break;
		}
	}

	if ( ioffset != 0 )
	{
		memmove(m_lpszStringPtr,m_lpszStringPtr + ioffset,isize - ioffset);
		m_lpszStringPtr[isize - ioffset] = 0;
	}
}
//..............................................................................................................................
void MString::TrimLeft(char * strIn)
{
	register int				i;
	register int				isize;
	register int				ioffset = 0;
	
	isize = strlen(strIn);
	ioffset = isize;

	for ( i=0;i<isize;i++ )
	{
		if (	strIn[i] != ' ' && strIn[i] != 0x09 && strIn[i] != 0x0D
			&&	strIn[i] != 0x0A )
		{
			ioffset = i;
			break;
		}
	}
	
	if ( ioffset != 0 )
	{
		memmove(strIn,strIn + ioffset,isize - ioffset);
		strIn[isize - ioffset] = 0;
	}
}
//..............................................................................................................................
void MString::TrimRight(void)
{
	register int				i;
	register int				isize;
	register int				ioffset = 0;
	
	isize = strlen(m_lpszStringPtr);
	for ( i=isize - 1;i>=0;i-- )
	{
		if (	m_lpszStringPtr[i] != ' ' && m_lpszStringPtr[i] != 0x09 && m_lpszStringPtr[i] != 0x0D
			&&	m_lpszStringPtr[i] != 0x0A )
		{
			m_lpszStringPtr[i+1] = 0;
			return;
		}
	}

	m_lpszStringPtr[0] = 0;
}
//..............................................................................................................................
void MString::TrimRight(char * strIn)
{
	register int				i;
	register int				isize;
	register int				ioffset = 0;
	
	isize = strlen(strIn);
	for ( i=isize - 1;i>=0;i-- )
	{
		if (	strIn[i] != ' ' && strIn[i] != 0x09 && strIn[i] != 0x0D
			&&	strIn[i] != 0x0A )
		{
			strIn[i+1] = 0;
			return;
		}
	}
	
	strIn[0] = 0;
}
//..............................................................................................................................
MString MString::TrimMiddleToSpace(void)
{
	register int				i;
	MString						strtemp;
	register char			*	lpcharptr;
	register int				isize;

	if ( (isize = strlen(m_lpszStringPtr)) <= 0 )
	{
		return("");
	}

	if ( strtemp.inner_allocstring(isize + 1) < 0 )
	{
		return("");
	}

	lpcharptr = strtemp.m_lpszStringPtr;
	* lpcharptr = m_lpszStringPtr[0];

	for ( i=1;i<isize;i++ )
	{
		if ( m_lpszStringPtr[i] == ' ' || m_lpszStringPtr[i] == 0x09 || m_lpszStringPtr[i] == 0x0A || m_lpszStringPtr[i] == 0x0D )
		{
			if ( m_lpszStringPtr[i-1] == ' ' || m_lpszStringPtr[i-1] == 0x09 || m_lpszStringPtr[i-1] == 0x0A || m_lpszStringPtr[i-1] == 0x0D )
			{
				//过滤掉该字符
			}
			else
			{
				* lpcharptr = ' ';
				lpcharptr ++;
			}
		}
		else
		{
			* lpcharptr = m_lpszStringPtr[i];
			lpcharptr ++;
		}
	}

	* lpcharptr = 0;

	return(strtemp);
}
//..............................................................................................................................
long MString::ToLong(void)
{
	return(strtol(m_lpszStringPtr,NULL,10));
}
//..............................................................................................................................
unsigned long MString::ToULong(void)
{
	return(strtoul(m_lpszStringPtr,NULL,10));
}
//..............................................................................................................................
double MString::ToDouble(void)
{
	return(strtod(m_lpszStringPtr,NULL));
}
//..............................................................................................................................
bool MString::ToBool(void)
{
	if ( stricmp(m_lpszStringPtr,"true") == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
MString MString::FormatString(const char * strFormat,...)
{
	char					tempbuf[MSTRING_DEFAULT_SIZE * 10];
	va_list					stmarker;

	va_start(stmarker,strFormat);
	_vsnprintf(tempbuf,MSTRING_DEFAULT_SIZE * 10,strFormat,stmarker);
	va_end( stmarker );

	return(tempbuf);
}
//..............................................................................................................................
char * MString::strncpy(char * strDest,const char * strSource,size_t iCount)
{
	return(my_strncpy(strDest,strSource,iCount));
}
//..............................................................................................................................
char * MString::strncat(char * strDest,const char * strSource,size_t iCount)
{
	return(my_strncat(strDest,strSource,iCount));
}
//..............................................................................................................................
void MString::_snprintf(char * lpszBuffer,size_t lCount,const char * lpszFormat,...)
{
	va_list					smarker;
	
	assert(lpszBuffer != NULL);
	assert(lCount > 0);
	
	if ( lpszBuffer == NULL || lCount <= 0 )
	{
		return;
	}
	
	va_start(smarker,lpszFormat);
	_vsnprintf(lpszBuffer,lCount,lpszFormat,smarker);
	va_end( smarker );
	
	lpszBuffer[lCount - 1] = 0;
}
//..............................................................................................................................
long MString::ToLong(const char * strValue)
{
	return(strtol(strValue,NULL,10));
}
//..............................................................................................................................
unsigned long MString::ToULong(const char * strValue)
{
	return(strtoul(strValue,NULL,10));
}
//..............................................................................................................................
double MString::ToDouble(const char * strValue)
{
	return(strtod(strValue,NULL));
}
//..............................................................................................................................
bool MString::ToBool(const char * strValue)
{
	if ( stricmp(strValue,"true") == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//..............................................................................................................................
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
