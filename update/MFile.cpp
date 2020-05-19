//------------------------------------------------------------------------------------------------------------------------------
#include <stdlib.h>
#include "MFile.h"
#include "../Security/MCheckCode.h"

//add by liuqy for test 20140623 
#ifndef LINUXCODE
#else
#include <execinfo.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
MFile::MFile(void)
{
	#ifndef LINUXCODE
		m_hRecordData = INVALID_HANDLE_VALUE;
	#else
		m_lRecordData = -1;
	#endif

	m_strFileName = "";
}
//..............................................................................................................................
MFile::~MFile()
{
	Close();
}
//..............................................................................................................................
int  MFile::Open(MString strFileName,unsigned long lMode)
{
	#ifndef LINUXCODE

	register unsigned long				lopenmode = 0;
	register unsigned long				lsharemode = 0;
	
		Close();
		
		if ( (lMode & MFILE_OPENMODE_READ) != 0 )
		{
			lopenmode |= GENERIC_READ;
		}
		
		if ( (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= GENERIC_WRITE;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREREAD) != 0 )
		{
			lsharemode |= FILE_SHARE_READ;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREWRITE) != 0 )
		{
			lsharemode |= FILE_SHARE_WRITE;
		}
		
		m_hRecordData = ::CreateFile(	strFileName.c_str(),
										lopenmode,
										lsharemode,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL	);
		if ( m_hRecordData == INVALID_HANDLE_VALUE )
		{
			return(MErrorCode::GetSysErr());
		}
		
		m_strFileName = strFileName;
		
		return(1);

	#else

		register unsigned long				lopenmode = 0;
		register unsigned long				lsharemode = 0;
		
		Close();
		
		if ( (lMode & MFILE_OPENMODE_READ) != 0 && (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= O_RDWR;
		}
		else if ( (lMode & MFILE_OPENMODE_READ) != 0 )
		{
			lopenmode |= O_RDONLY;
		}
		else if ( (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= O_WRONLY;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREREAD) != 0 )
		{
			lsharemode |= S_IROTH;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREWRITE) != 0 )
		{
			lsharemode |= S_IWOTH;
		}
		
// 		lsharemode = 511;//huq modify 20140220
		lsharemode = 0;
		lsharemode = S_IRUSR |S_IWUSR |S_IXUSR |S_IRGRP |S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH;
		umask(0);

		m_lRecordData = open(strFileName.c_str(),lopenmode,lsharemode);
		if ( m_lRecordData < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		m_strFileName = strFileName;
		//add by liuqy for test 20140623 
#ifdef _MYDEBUG_
		struct tm					sttemptime;
		time_t	stNow;
		time(&stNow);
		localtime_r(&stNow, &sttemptime);
		struct timeb	tp;
		ftime(&tp);
		
		
		if(9 == sttemptime.tm_hour  && 30 > sttemptime.tm_min )
		{
			char szbuf[64];
			sprintf(szbuf, "./FileHdlO%0d.txt", sttemptime.tm_mday);
			FILE * p= fopen(szbuf, "a");
			if(NULL != p)
			{
				int size = 16;
				int i;
				void * array[16];
				int stack_num = backtrace(array, size);
				char ** stacktrace = backtrace_symbols(array, stack_num);
				
				fprintf(p, "%d-%d %d:%d:%d.%d [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
					tp.millitm, m_lRecordData);
				if(NULL != stacktrace)
				{
					for (i=0; i<stack_num; i++)
						fprintf(p, "%s\n", stacktrace[i]);
					free(stacktrace);
				}
				fclose(p);	
			}
		}
		
#endif
		//end add
// 		chmod(m_strFileName.c_str(), 511);//huq add 20140220
		
		return(1);
	
	#endif
}
//..............................................................................................................................
int  MFile::OpenRead(MString strFileName,unsigned long lMode)
{
	return(Open(strFileName,lMode));
}
//..............................................................................................................................
int  MFile::OpenWrite(MString strFileName,unsigned long lMode)
{
	return(Open(strFileName,lMode));
}
//..............................................................................................................................
int  MFile::Create(MString strFileName,unsigned long lMode)
{
	#ifndef LINUXCODE
		
		register unsigned long				lopenmode = 0;
		register unsigned long				lsharemode = 0;
		
		Close();
		
		if ( (lMode & MFILE_OPENMODE_READ) != 0 )
		{
			lopenmode |= GENERIC_READ;
		}
		
		if ( (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= GENERIC_WRITE;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREREAD) != 0 )
		{
			lsharemode |= FILE_SHARE_READ;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREWRITE) != 0 )
		{
			lsharemode |= FILE_SHARE_WRITE;
		}
		
		m_hRecordData = ::CreateFile(	strFileName.c_str(),
										lopenmode,
										lsharemode,
										NULL,
										CREATE_NEW,
										FILE_ATTRIBUTE_NORMAL,
										NULL	);
		if ( m_hRecordData == INVALID_HANDLE_VALUE )
		{
			return(MErrorCode::GetSysErr());
		}
		
		m_strFileName = strFileName;
		
		return(1);
		
	#else
		
		register unsigned long				lopenmode = 0;
		register unsigned long				lsharemode = 0;
		
		Close();
		
		if ( (lMode & MFILE_OPENMODE_READ) != 0 && (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= O_CREAT | O_RDWR;
		}
		else if ( (lMode & MFILE_OPENMODE_READ) != 0 )
		{
			lopenmode |= O_CREAT | O_RDONLY;
		}
		else if ( (lMode & MFILE_OPENMODE_WRITE) != 0 )
		{
			lopenmode |= O_CREAT | O_WRONLY;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREREAD) != 0 )
		{
			lsharemode |= S_IROTH;
		}
		
		if ( (lMode & MFILE_OPENMODE_SHAREWRITE) != 0 )
		{
			lsharemode |= S_IWOTH;
		}
		
		/*
		 *	lsharemode = 0755 权限
		 */
		lsharemode = 0;
		lsharemode = S_IRUSR |S_IWUSR |S_IXUSR |S_IRGRP |S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH;
		umask(0);
		m_lRecordData = open(strFileName.c_str(),lopenmode,lsharemode);
		if ( m_lRecordData < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		m_strFileName = strFileName;
#ifdef _MYDEBUG_
		//add by liuqy for test 20140623 
		struct tm					sttemptime;
		time_t	stNow;
		time(&stNow);
		localtime_r(&stNow, &sttemptime);
		struct timeb	tp;
		ftime(&tp);
		
		
		if(9 == sttemptime.tm_hour  && 30 > sttemptime.tm_min )
		{
			char szbuf[64];
			sprintf(szbuf, "./FileHdlcrt%0d.txt", sttemptime.tm_mday);
			FILE * p= fopen(szbuf, "a");
			if(NULL != p)
			{
				int size = 16;
				int i;
				void * array[16];
				int stack_num = backtrace(array, size);
				char ** stacktrace = backtrace_symbols(array, stack_num);
				
				fprintf(p, "%d-%d %d:%d:%d.%d [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
					tp.millitm, m_lRecordData);
				if(NULL != stacktrace)
				{
					for (i=0; i<stack_num; i++)
						fprintf(p, "%s\n", stacktrace[i]);
					free(stacktrace);
				}
				fclose(p);	
			}
		}
#endif
		
		//end add

// 		chmod(m_strFileName.c_str(), 511);
	
		return(1);
		
	#endif
}
//..............................................................................................................................
void MFile::Close(void)
{
	#ifndef LINUXCODE
		if ( m_hRecordData != INVALID_HANDLE_VALUE )
		{
			::CloseHandle(m_hRecordData);
			m_hRecordData = INVALID_HANDLE_VALUE;
		}
		m_strFileName = "";

	#else

//add by liuqy for test 20140623 
//#ifdef _MYDEBUG_
// 	struct tm					sttemptime;
// 	time_t	stNow;
// 	time(&stNow);
// 	localtime_r(&stNow, &sttemptime);
// 	struct timeb	tp;
// 	ftime(&tp);
// 	
// 	
// //	if(9 == sttemptime.tm_hour  && 30 > sttemptime.tm_min )
// 	{
// 		char szbuf[64];
// 		sprintf(szbuf, "./closefile.txt");
// 		FILE * p= fopen(szbuf, "a");
// 		if(NULL != p)
// 		{
// 			int size = 16;
// 			int i;
// 			void * array[16];
// 			int stack_num = backtrace(array, size);
// 			char ** stacktrace = backtrace_symbols(array, stack_num);
// 
// 			fprintf(p, "%d-%d %d:%d:%d.%d [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
// 				tp.millitm, m_lRecordData);
// 			if(NULL != stacktrace)
// 			{
// 				for (i=0; i<stack_num; i++)
// 					fprintf(p, "%s\n", stacktrace[i]);
// 				free(stacktrace);
// 			}
// 			fclose(p);	
// 		}
// 	}
//#endif
//end add
		if ( m_lRecordData >= 0 )
		{
			close(m_lRecordData);
			m_lRecordData = -1;
		}
		m_strFileName = "";

	#endif
}
//..............................................................................................................................
bool MFile::IsOpen(void)
{
	#ifndef LINUXCODE

		if ( m_hRecordData != INVALID_HANDLE_VALUE )
		{
			return(true);
		}
		else
		{
			return(false);
		}

	#else

		if ( m_lRecordData >= 0 )
		{
			return(true);
		}
		else
		{
			return(false);
		}

	#endif
}
//..............................................................................................................................
MString MFile::GetFileName(void)
{
	return(m_strFileName);
}

//..............................................................................................................................
int  MFile::Seek(unsigned long lOffset,unsigned long lType)
{
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
	if(m_lRecordData == -1)
#endif
	{
		return ERR_MFILE_WRITEALL;
	}

#ifndef LINUXCODE
	
	register int						errorcode;
	
	/*//modify by liuqy 20110706 for 支持大文件操作
	LONG p = 0;
	errorcode = ::SetFilePointer(m_hRecordData,lOffset,&p,lType);
	*/
	//20110712 by tangj 因为使用了负数,所以回退代码,另外新增seek64
	errorcode = ::SetFilePointer(m_hRecordData,lOffset,0,lType);
	if ( errorcode == 0xFFFFFFFF )
	{
		return(MErrorCode::GetSysErr());
	}
	else
	{
		return(errorcode);
	}
	
#else
	
	register int						errorcode;
	
	errorcode = lseek(m_lRecordData,lOffset,lType);
	if ( errorcode < 0 )
	{
		return(MErrorCode::GetSysErr());
	}
	else
	{
		return(errorcode);	
	}
	
#endif
}
//#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
//20110712 by tangj 因为使用了负数,所以回退代码,另外新增seek64
__int64  MFile::Seek64(__int64 i64Offset,unsigned long lType)
{
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
		if(m_lRecordData == -1)
#endif
		{
			return ERR_MFILE_WRITEALL;
		}

#ifndef LINUXCODE
	
	register int						errorcode;
	
	//20110712 by tangj 因为使用了负数,所以回退代码,另外新增seek64
	LONG	lHigh = i64Offset>>32;//高双字 
	errorcode = ::SetFilePointer(m_hRecordData, i64Offset&0xFFFFFFFF, &lHigh, lType);
	if ( errorcode == 0xFFFFFFFF )
	{
		return(MErrorCode::GetSysErr());
	}
	else
	{
		return errorcode | ((__int64)lHigh << 32);
	}
	
#else
	
	register __int64						errorcode;
	
	errorcode = lseek64(m_lRecordData,i64Offset,lType);
	if ( errorcode < 0 )
	{
		return(MErrorCode::GetSysErr());
	}
	else
	{
		return(errorcode);	
	}
	
#endif
}


//..............................................................................................................................
int  MFile::Read(char * lpOut,unsigned long lSize)
{
	assert(lpOut != NULL);
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
		if(m_lRecordData == -1)
#endif
		{
			return ERR_MFILE_READALL;
		}

	#ifndef LINUXCODE

		register unsigned long				errorcode = 0;
		
		if ( ::ReadFile(m_hRecordData,lpOut,lSize,&errorcode,NULL) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(errorcode);
		}

	#else

		register int						errorcode;
		
		if ( (errorcode = read(m_lRecordData,lpOut,lSize)) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(errorcode);	
		}

	#endif
}
//..............................................................................................................................
int  MFile::Write(const char * lpIn,unsigned long lSize)
{
	assert(lpIn != NULL);
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
		if(m_lRecordData == -1)
#endif
		{
			return ERR_MFILE_WRITEALL;
		}

	#ifndef LINUXCODE

		register unsigned long				errorcode = 0;
		
		if ( ::WriteFile(m_hRecordData,lpIn,lSize,&errorcode,NULL) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(errorcode);
		}

	#else

		register int						errorcode;
		
		if ( (errorcode = write(m_lRecordData,lpIn,lSize)) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(errorcode);
		}

	#endif
}
//..............................................................................................................................
int  MFile::ReadAll(char * lpOut,unsigned long lSize)
{
	register int							errorcode;

	if ( (errorcode = Read(lpOut,lSize)) != lSize )
	{
		return(ERR_MFILE_READALL);
	}

	return(errorcode);
}
//..............................................................................................................................
int  MFile::WriteAll(const char * lpIn,unsigned long lSize)
{
	register int							errorcode;

	if ( (errorcode = Write(lpIn,lSize)) != lSize )
	{
		return(ERR_MFILE_WRITEALL);
	}

	return(errorcode);
}
//..............................................................................................................................
void MFile::Flush(void)
{
	#ifndef LINUXCODE
		::FlushFileBuffers(m_hRecordData);
	#else
		fsync(m_lRecordData);
	#endif
}
//..............................................................................................................................
MString MFile::ReadString(unsigned long lMaxSize)
{
	MString									strtemp;

	if ( ReadString(&strtemp,lMaxSize) <= 0 )
	{
		return("");
	}

	if ( strtemp.GetLength() > 2 )
	{
		strtemp.TrimRight();
	}

	return(strtemp);
}
//..............................................................................................................................
int  MFile::ReadString(char * strOut,unsigned long lMaxSize)
{
	int										icurposition;
	int										ireadsize;
	register int							i;

	//读取数据
	icurposition = Seek(0,1);
	if ( (ireadsize = Read(strOut,lMaxSize)) <= 0 )
	{
		Seek(icurposition,0);
		
		return(ireadsize);
	}

	//分析数据
	for ( i=0;i<ireadsize;i++ )
	{
		if (	(strOut[i] == 0x0D || strOut[i] == 0x0A)
			&& (strOut[i+1] != 0x0D && strOut[i+1] != 0x0A)	)
		{
			strOut[i+1] = 0;
			
			Seek(icurposition + i + 1,0);
			
			return i + 1;
		}
	}
	
	strOut[ireadsize] = 0;
	
	Seek(icurposition + ireadsize,0);
	
	return ireadsize;
}
//..............................................................................................................................
int  MFile::ReadString(MString * strOut,unsigned long lMaxSize)
{
	char									defaultbuf[512];
	char								*	lpstringptr;
	register int							errorcode;

	//分配存储空间
	if ( lMaxSize > 512 )
	{
		//lpstringptr = new char[lMaxSize];
		//modify by liuqy 20150630 需要将new 更换为malloc 
		lpstringptr = (char*)malloc(lMaxSize);
		if ( lpstringptr == NULL )
		{
			assert(0);
			return(ERR_PUBLIC_NOMEMORY);
		}
	}
	else
	{
		lpstringptr = defaultbuf;
	}

	errorcode =  ReadString(lpstringptr,lMaxSize);
	*strOut = lpstringptr;
	//释放存储空间
	if ( lpstringptr != defaultbuf )
	{
		errorcode = min( errorcode, lMaxSize );
		if( errorcode > 0 )
		{
			lpstringptr[errorcode-1] = 0;
		}
		*strOut = lpstringptr;
		//modify by liuqy 20150630 需要将new 更换为malloc 
		//delete [] lpstringptr;
		free(lpstringptr);
	}

	return(errorcode);
}
//..............................................................................................................................
int  MFile::WriteString(const char * strIn)
{
	register int							errorcode;

	if ( (errorcode = Write(strIn,strlen(strIn))) < 0 )
	{
		return(errorcode);
	}

	return(Write("\r\n",2));
}
//..............................................................................................................................
int  MFile::WriteString(MString strIn)
{
	strIn += "\r\n";

	return(Write(strIn.c_str(),strIn.GetLength()));
}
//..............................................................................................................................
int  MFile::SetFileLength(unsigned long lSize)
{
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
	if(m_lRecordData == -1)
#endif
	{
		return ERR_MFILE_NOTSIZE;
	}

	#ifndef LINUXCODE

		register int						errorcode;
		
		errorcode = ::SetFilePointer(m_hRecordData,lSize,0,0);
		if ( errorcode == 0xFFFFFFFF )
		{
			return(MErrorCode::GetSysErr());
		}
		
		if ( ::SetEndOfFile(m_hRecordData) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		register int						errorcode;
		
		errorcode = ftruncate(m_lRecordData,lSize);
		if ( errorcode < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::SetFileLength(void)
{
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
	if(m_lRecordData == -1)
#endif
	{
		return ERR_MFILE_NOTSIZE;
	}

	#ifndef LINUXCODE

		if ( ::SetEndOfFile(m_hRecordData) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		int	oldptr = 0;		//	GUOGUO 20090723
		oldptr = lseek(m_lRecordData,0,1);
		if ( oldptr < 0 )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( ftruncate(m_lRecordData,oldptr) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::GetFileLength(void)
{
	register int				ioldposition;
	register int				ifilesize;
#ifndef LINUXCODE
	if(m_hRecordData == INVALID_HANDLE_VALUE)
#else
	if(m_lRecordData == -1)
#endif
	{
		return ERR_MFILE_NOTSIZE;
	}

	ioldposition = Seek(0,1);
	ifilesize = Seek(0,2);
	Seek(ioldposition,0);
	
	return(ifilesize);
}
//..............................................................................................................................
int  MFile::LockFile(unsigned long lOffset,unsigned long lSize)
{

	#ifndef LINUXCODE

		if ( ::LockFile(m_hRecordData,lOffset,0,lSize,0) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}

	#else

		struct flock 				stlock;
		
		stlock.l_type = F_WRLCK;
		stlock.l_whence = SEEK_SET;
		stlock.l_start = lOffset;
		stlock.l_len = lSize;
		
		if ( fcntl(m_lRecordData,F_SETLK,&stlock) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);	
		}

	#endif
}
//..............................................................................................................................
int  MFile::UnLockFile(unsigned long lOffset,unsigned long lSize)
{
	#ifndef LINUXCODE

		if ( ::UnlockFile(m_hRecordData,lOffset,0,lSize,0) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}

	#else

		struct flock 				stlock;
		
		stlock.l_type = F_UNLCK;
		stlock.l_whence = SEEK_SET;
		stlock.l_start = lOffset;
		stlock.l_len = lSize;
		
		if ( fcntl(m_lRecordData,F_SETLK,&stlock) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);	
		}

	#endif
}
//..............................................................................................................................
MDateTime MFile::GetCreateDateTime(void)
{
	#ifndef LINUXCODE

		FILETIME					stcreatetime,stupdatetime,staccesstime,stlocaltime;
		SYSTEMTIME					stsystime;
		
		if ( ::GetFileTime(m_hRecordData,&stcreatetime,&staccesstime,&stupdatetime) == 0 )
		{
			return(MDateTime(0));
		}
		
		::FileTimeToLocalFileTime(&stcreatetime, &stlocaltime );
		::FileTimeToSystemTime(&stlocaltime,&stsystime);
		
		return(		MDateTime(	stsystime.wYear,
								stsystime.wMonth,
								stsystime.wDay,
								stsystime.wHour,
								stsystime.wMinute,
								stsystime.wSecond	)	);

	#else

		struct stat					sttemp;
		
		if ( fstat(m_lRecordData,&sttemp) < 0 )
		{
			return(MDateTime(0));
		}
		
		return(MDateTime(sttemp.st_ctime));

	#endif
}
//..............................................................................................................................
MDateTime MFile::GetUpdateDateTime(void)
{
	#ifndef LINUXCODE
		
		FILETIME					stcreatetime,stupdatetime,staccesstime,stlocaltime;
		SYSTEMTIME					stsystime;
		
		if ( ::GetFileTime(m_hRecordData,&stcreatetime,&staccesstime,&stupdatetime) == 0 )
		{
			return(MDateTime(0));
		}
		
		::FileTimeToLocalFileTime(&stupdatetime, &stlocaltime );
		::FileTimeToSystemTime(&stlocaltime,&stsystime);
		
		return(		MDateTime(	stsystime.wYear,
								stsystime.wMonth,
								stsystime.wDay,
								stsystime.wHour,
								stsystime.wMinute,
								stsystime.wSecond	)	);
		
	#else
		
		struct stat					sttemp;
		
		if ( fstat(m_lRecordData,&sttemp) < 0 )
		{
			return(MDateTime(0));
		}
		
		return(MDateTime(sttemp.st_mtime));
		
	#endif
}
//..............................................................................................................................
MDateTime MFile::GetAccessDateTime(void)
{
	#ifndef LINUXCODE
		
		FILETIME					stcreatetime,stupdatetime,staccesstime,stlocaltime;
		SYSTEMTIME					stsystime;
		
		if ( ::GetFileTime(m_hRecordData,&stcreatetime,&staccesstime,&stupdatetime) == 0 )
		{
			return(MDateTime(0));
		}
		
		::FileTimeToLocalFileTime(&staccesstime, &stlocaltime );
		::FileTimeToSystemTime(&stlocaltime,&stsystime);
		
		return(		MDateTime(	stsystime.wYear,
								stsystime.wMonth,
								stsystime.wDay,
								stsystime.wHour,
								stsystime.wMinute,
								stsystime.wSecond	)	);
		
	#else
		
		struct stat					sttemp;
		
		if ( fstat(m_lRecordData,&sttemp) < 0 )
		{
			return(MDateTime(0));
		}
		
		return(MDateTime(sttemp.st_atime));
		
	#endif
}
//..............................................................................................................................
int  MFile::SetCreateDateTime(MDateTime mIn)
{
	#ifndef LINUXCODE

		FILETIME					stfiletime,stutctime;
		SYSTEMTIME					stsystime;
		
		memset( &stsystime, 0, sizeof(SYSTEMTIME) );
		mIn.DecodeDate(&stsystime.wYear,&stsystime.wMonth,&stsystime.wDay);
		mIn.DecodeTime(&stsystime.wHour,&stsystime.wMinute,&stsystime.wSecond);
		
		::SystemTimeToFileTime(&stsystime,&stfiletime);
		::LocalFileTimeToFileTime( &stfiletime, &stutctime );
		if ( ::SetFileTime(m_hRecordData,&stutctime,NULL,NULL) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}

	#else

		return(ERR_MFILE_LINUXCREATTIME);

	#endif
}
//..............................................................................................................................
int  MFile::SetUpdateDateTime(MDateTime mIn)
{
	#ifndef LINUXCODE
		
		FILETIME					stfiletime,stutctime;
		SYSTEMTIME					stsystime;
		
		memset( &stsystime, 0, sizeof(SYSTEMTIME) );
		mIn.DecodeDate(&stsystime.wYear,&stsystime.wMonth,&stsystime.wDay);
		mIn.DecodeTime(&stsystime.wHour,&stsystime.wMinute,&stsystime.wSecond);
		
		::SystemTimeToFileTime(&stsystime,&stfiletime);
		::LocalFileTimeToFileTime( &stfiletime, &stutctime );
		if ( ::SetFileTime(m_hRecordData,NULL,NULL,&stutctime) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}
		
	#else
		
		struct utimbuf 				sttemp;
		
		sttemp.modtime = mIn.DateTimeToTimet(); 
		if ( utime(m_strFileName.c_str(),&sttemp) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);	
		}
		
	#endif
}
//..............................................................................................................................
int  MFile::SetAccessDateTime(MDateTime mIn)
{
	#ifndef LINUXCODE
		
		FILETIME					stfiletime,stutctime;
		SYSTEMTIME					stsystime;
		
		memset( &stsystime, 0, sizeof(SYSTEMTIME) );
		mIn.DecodeDate(&stsystime.wYear,&stsystime.wMonth,&stsystime.wDay);
		mIn.DecodeTime(&stsystime.wHour,&stsystime.wMinute,&stsystime.wSecond);
		
		::SystemTimeToFileTime(&stsystime,&stfiletime);
		::LocalFileTimeToFileTime( &stfiletime, &stutctime );
		if ( ::SetFileTime(m_hRecordData,NULL,&stutctime,NULL) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}
		
	#else
		
		struct utimbuf 				sttemp;
		
		sttemp.actime = mIn.DateTimeToTimet(); 
		if ( utime(m_strFileName.c_str(),&sttemp) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);	
		}
		
	#endif
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
int  MFile::RenameFileName(MString strSourceFile,MString strNewFile)
{
	#ifndef LINUXCODE

		if ( ::MoveFile(strSourceFile.c_str(),strNewFile.c_str()) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		if ( rename(strSourceFile.c_str(),strNewFile.c_str()) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::DeleteFile(MString strFileName)
{
	#ifndef LINUXCODE

		if ( ::DeleteFile(strFileName.c_str()) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		if ( remove(strFileName.c_str()) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::CreateDirectory(MString strFileName)
{
	#ifndef LINUXCODE

		if ( ::CreateDirectory(strFileName.c_str(),NULL) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		if ( mkdir(strFileName.c_str(),0x1FF) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::DeleteDirectory(MString strFileName)
{
	#ifndef LINUXCODE

		if ( ::RemoveDirectory(strFileName.c_str()) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		if ( rmdir(strFileName.c_str()) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
int  MFile::CreateDirectoryTree(MString strDirTreeName)
{
	char				szdirtree[256];
	char				tempbuf[256];
	register int		i;
	register int		errorcode;
	register int		ilength;
	
	strncpy(szdirtree,strDirTreeName.c_str(),256);
	ilength = (int)strlen(szdirtree);
	for ( i=0;i<ilength;i++ )
	{
		if ( i > 0 && (szdirtree[i] == '\\' || szdirtree[i] == '/') && szdirtree[i-1] != '.' && szdirtree[i-1] != ':' )
		{
			memcpy(tempbuf,szdirtree,i);
			tempbuf[i] = 0;
			
			if ( !MFile::IsExist(tempbuf)&&(errorcode = MFile::CreateDirectory(tempbuf)) < 0 )
			{
 				return(errorcode);
			}
		}
	}
	
	return(MFile::CreateDirectory(szdirtree));
}
//..............................................................................................................................
int  MFile::DeleteDirectoryTree(MString strDirTreeName)
{
	char						szdirtree[256];
	register int				ilength, i;
	register int				errorcode;
	bool						bDelAll = true;

	DeleteDirAllFile(strDirTreeName);
	
	if ( (errorcode = MFile::DeleteDirectory(strDirTreeName)) < 0 )
	{
		return(errorcode);
	}
	
	strncpy(szdirtree,strDirTreeName.c_str(),256);
	ilength = (int)strlen(szdirtree);
	
	for ( i=ilength - 2;i>0;i-- )
	{
		if ( (szdirtree[i] == '\\' || szdirtree[i] == '/') && szdirtree[i-1] != '.' && szdirtree[i-1] != ':' )
		{
			szdirtree[i] = 0;
			
			bDelAll = false;
/*			DeleteDirAllFile(szdirtree);

			if ( (errorcode = MFile::DeleteDirectoryTree(szdirtree)) < 0 )
			{
				return(errorcode);
			}
*/
			break;
		}
	}

	if( bDelAll )
	{
		if ( (errorcode = MFile::DeleteDirectoryTree(szdirtree)) < 0 )
		{
			return(errorcode);
		}
	}
	
	return(1);
}
//..............................................................................................................................
MString MFile::GetCurrentDirectory(void)
{
	#ifndef LINUXCODE

		char						tempbuf[256];
		
		if ( ::GetCurrentDirectory(256,tempbuf) == 0 )
		{
			return("");
		}
		
		if ( strlen(tempbuf) > 0 && tempbuf[strlen(tempbuf)-1] != '\\' )
		{
			my_strncat(tempbuf,"\\",256);	
		}
		
		return(tempbuf);

	#else

		char						tempbuf[256];
		
		if ( getcwd(tempbuf,256) < 0 )
		{
			return("");	
		}
		
		if ( strlen(tempbuf) > 0 && tempbuf[strlen(tempbuf)-1] != '/' )
		{
			strcat(tempbuf,"/");	
		}
		
		return(tempbuf);

	#endif
}
//..............................................................................................................................
int  MFile::SetCurrentDirectory(MString strIn)
{
	#ifndef LINUXCODE

		if ( ::SetCurrentDirectory(strIn.c_str()) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		if ( chdir(strIn.c_str()) < 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#endif
}
//..............................................................................................................................
bool MFile::IsExist(MString strFileName)
{
	#ifndef LINUXCODE

		if ( _access(strFileName.c_str(),0) == -1 )
		{
			return(false);
		}
		
		return(true);

	#else

		if ( access(strFileName.c_str(),0) == -1 )
		{
			return(false);
		}
		
		return(true);

	#endif
}
//..............................................................................................................................
int  MFile::CopyFile(MString strSourceFile,MString strNewFile,bool bFailIfExists)
{
	#ifndef LINUXCODE

		BOOL					bflag;
		
		if ( bFailIfExists == true )
		{
			bflag = TRUE;
		}
		else
		{
			bflag = FALSE;
		}
		
		if ( ::CopyFile(strSourceFile.c_str(),strNewFile.c_str(),bflag) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		//拷贝的交换内存空间大小
		#define MFILE_COPYFILE_MEMORY_SIZE		10240
		
		MFile						msourceptr;
		MFile						mnewptr;
		char						tempbuf[MFILE_COPYFILE_MEMORY_SIZE];
		int							isourcesize;
		int							errorcode;
		int							iframecount;
		int							igetcount;
		register int				i;
		
		if( MFile::IsExist( strNewFile ) == true && bFailIfExists == true )
		{
			return(ERR_MFILE_LINUXCOPYFILE);
		}
		
//		if ( (errorcode = msourceptr.Open(strSourceFile)) < 0 )	GUOGUO 20090723
		if ( (errorcode = msourceptr.OpenRead(strSourceFile)) < 0 )
		{
			return(errorcode);
		}
		
		isourcesize = msourceptr.Seek(0,2);
		
		iframecount = (isourcesize % MFILE_COPYFILE_MEMORY_SIZE == 0)?(isourcesize / MFILE_COPYFILE_MEMORY_SIZE):(isourcesize / MFILE_COPYFILE_MEMORY_SIZE + 1);
		if ( iframecount < 0 )
		{
			msourceptr.Close();
			return(isourcesize);
		}
		
		MFile::DeleteFile( strNewFile );
		if ( (errorcode = mnewptr.Create(strNewFile)) < 0 )
		{
			msourceptr.Close();
			return(errorcode);
		}
		
		msourceptr.Seek(0,0);
		mnewptr.Seek(0,0);
		
		for ( i=0;i<iframecount;i++ )
		{
			igetcount = (i == iframecount - 1)?(isourcesize - i * MFILE_COPYFILE_MEMORY_SIZE):MFILE_COPYFILE_MEMORY_SIZE;
			
			if ( (errorcode = msourceptr.Read(tempbuf,igetcount)) < 0 )
			{
				msourceptr.Close();
				mnewptr.Close();

				return(errorcode);
			}

			if ( (errorcode = mnewptr.Write(tempbuf,igetcount)) < 0 )
			{
				msourceptr.Close();
				mnewptr.Close();
				
				return(errorcode);
			}
		}
		
		msourceptr.Close();
		mnewptr.Close();
		
		return(1);
		
	#endif
}
//..............................................................................................................................
int  MFile::DeleteDirAllFile(MString strDirName)
{
	MFindFile						mfind;
	MString							strfindstring;
	tagMFindFileInfo				stfileinfo;
	register int					errorcode;
	MString							strtemp;
	
	if ( strDirName[strDirName.GetLength() - 1] != '\\' && strDirName[strDirName.GetLength() - 1] != '/' )
	{
		#ifndef LINUXCODE
			strfindstring += strDirName + "\\*.*";
			strDirName += "\\";
		#else
			strfindstring += strDirName + "/*.*";
			strDirName += "/";
		#endif
	}

	if ( (errorcode = mfind.FindFirst(strfindstring,&stfileinfo)) < 0 )
	{
		return(errorcode);
	}

	do 
	{
		strtemp = stfileinfo.szName;

		if ( strtemp != '.' && strtemp != ".." )
		{
			if ( stfileinfo.lAttr == MFILE_ATTR_DIRECTORY )
			{
				if ( (errorcode = DeleteDirAllFile(strDirName + strtemp)) < 0 )
				{
					mfind.CloseFind();
					return(errorcode);
				}
				if ( (errorcode = DeleteDirectory(strDirName + strtemp)) < 0 )
				{
					mfind.CloseFind();
					return(errorcode);
				}
			}
			else if ( (errorcode = MFile::DeleteFile(strDirName +  strtemp)) < 0 )
			{
				mfind.CloseFind();
				return(errorcode);
			}
		}

	} while ( mfind.FindNext(&stfileinfo) >= 0 );

	mfind.CloseFind();
	return(1);
}
//..............................................................................................................................
MString MFile::GetPathFromString(MString strIn)
{
	MString						strtemp = strIn;
	register int				i;

	for ( i=strtemp.GetLength() - 1;i>=0;i-- )
	{
		if ( strtemp[i] == '\\' || strtemp[i] == '/' )
		{
			strtemp.SetAt(i+1,0);

			return(strtemp);
		}
	}

	return("");
}
//..............................................................................................................................
MString MFile::GetFileNameFromString(MString strIn)
{
	MString						strtemp = strIn;
	register int				i;

	for ( i=strtemp.GetLength() - 1;i>=0;i-- )
	{
		if ( strtemp[i] == '\\' || strtemp[i] == '/' )
		{	
			return(strtemp.c_str() + i + 1);
		}
	}
	
	return(strtemp);
}
//..............................................................................................................................
unsigned long MFile::GetFileCheckCode(MString strFilePath)
{
	unsigned long				sum = 0;
	MFile						mfileptr;
	register int				errorcode;
	char						tempbuf[8192];
	register int				i;

	if ( mfileptr.OpenRead(strFilePath) < 0 )
	{
		return(0);
	}

	while ( 1 )
	{
		errorcode = mfileptr.Read(tempbuf,8192);
		if ( errorcode <= 0 )
		{
			break;
		}

		for ( i=0;i<errorcode;i++ )
		{
			sum += (unsigned long)tempbuf[i];
		}
	}

	mfileptr.Close();

	return(sum);
}

unsigned int MFile::GetFileCRC32Code(const char *lpszFileName)
{
	unsigned int				checkcode = 0;
	MFile						mfileptr;
	register int				errorcode;
	char						tempbuf[8192];
	MCRC32						mCRC32;

	if ( mfileptr.OpenRead(lpszFileName) < 0 )
	{
		return(0);
	}

	while ( 1 )
	{
		errorcode = mfileptr.Read(tempbuf,8192);
		if ( errorcode <= 0 )
		{
			break;
		}
		checkcode = mCRC32.CheckCode(tempbuf, errorcode, checkcode);
	}

	mfileptr.Close();

	return(checkcode);
}
//add by liuqy 20110517 for 同时取更新时间和文件大小，以减少IO操作
bool MFile::GetFileSizeUpdateTime(MString in_strFileName, long & out_lFileSize, long & out_lUpdateTime)
{
	#ifndef LINUXCODE
		
	  int		nRes = 0;
	  struct stat stUpDatetime; 
	  
	  if(stat(in_strFileName.c_str(),  &stUpDatetime) < 0)
		  return false;

	out_lUpdateTime = stUpDatetime.st_mtime;
	out_lFileSize = stUpDatetime.st_size;
	#else
		int		nRes = 0;
		struct stat stUpDatetime; 

		if(stat(in_strFileName.c_str(),  &stUpDatetime) < 0)
			return false;
	out_lFileSize = stUpDatetime.st_size;
		out_lUpdateTime =  stUpDatetime.st_mtime;
	#endif
	return true;
}
//end add
//------------------------------------------------------------------------------------------------------------------------------
// 通过stat直接获得文件的更新日期，anzl 20091211
long MFile::GetUpdateDateTimeByStat(MString strFileName)
{
	#ifndef LINUXCODE
		
	  int		nRes = 0;
	  struct stat stUpDatetime; 
	  
	  if(stat(strFileName.c_str(),  &stUpDatetime) < 0)
		  return -1;

	  return stUpDatetime.st_mtime;

	#else
		int		nRes = 0;
		struct stat stUpDatetime; 

		if(stat(strFileName.c_str(),  &stUpDatetime) < 0)
			return -1;

		return stUpDatetime.st_mtime;
	#endif
}

//------------------------------------------------------------------------------------------------------------------------------
// 通过stat直接获得文件的大小，anzl 20100810
long MFile::GetFileSizeByStat(MString strFileName)
{
	#ifndef LINUXCODE
		
	  int		nRes = 0;
	  struct stat stUpDatetime; 
	  
	  if(stat(strFileName.c_str(),  &stUpDatetime) < 0)
		  return -1;

	  return stUpDatetime.st_size;

	#else
		int		nRes = 0;
		struct stat stUpDatetime; 

		if(stat(strFileName.c_str(),  &stUpDatetime) < 0)
			return -1;

		return stUpDatetime.st_size;
	#endif
}

//------------------------------------------------------------------------------------------------------------------------------
MFindFile::MFindFile(void)
{
	#ifndef LINUXCODE
		m_hRecordData = INVALID_HANDLE_VALUE;
	#else
		m_stRecordData = NULL;
	#endif
}
//..............................................................................................................................
MFindFile::~MFindFile()
{
	CloseFind();
}
//..............................................................................................................................
bool MFindFile::inner_checkmatch(const char * strMatch)
{
	register int				i;

	for ( i=0;i<strlen(strMatch);i++ )
	{
		if (	(strMatch[i] == '?' && strMatch[i] == '*')
			 || (strMatch[i] == '*' && strMatch[i] == '?')
			 || (strMatch[i] == '*' && strMatch[i] == '*')	)
		{
			//非法
			return(false);
		}
	}

	return(true);
}
//..............................................................................................................................
bool MFindFile::inner_match(const char * strSource,const char * strMatch)
{

	register int				isourcesize, imatchsize;
	register int				i = 0, j = 0;
	
	isourcesize = strlen(strSource);
	imatchsize = strlen(strMatch);
	
	while ( 1 )
	{
		//判断匹配是否结束
		if ( i >= isourcesize )
		{
			if ( j >= imatchsize )
			{
				//匹配完成
				return(true);
			}
			else
			{
				//匹配失败
				return(false);
			}
		}
		else if ( j >= imatchsize )
		{
			//匹配失败
			return(false);
		}

		//开始匹配
		if ( strMatch[j] == '?' )
		{
			i ++;
			j ++;
		}
		else if ( strMatch[j] == '*' )
		{
			if ( j == imatchsize - 1 )
			{
				//*号是最后一个匹配的字符，那么成功匹配
				return(true);
			}
			else
			{
				j ++;
			}

			while ( i < isourcesize )
			{
				if ( strMatch[j] != strSource[i] )
				{
					i ++;
				}
				else
				{
					break;
				}
			}
		}
		else if ( strMatch[j] == strSource[i] )
		{
			i ++;
			j ++;
		}
		else
		{
			//匹配失败
			return(false);
		}

	}
}
//..............................................................................................................................
int  MFindFile::FindFirst(MString strFindName,tagMFindFileInfo * lpsOut)
{
	CloseFind();

	#ifndef LINUXCODE

		WIN32_FIND_DATA				stfileinfo;
		SYSTEMTIME					stsystime;
		FILETIME					lft;
		
		m_hRecordData = ::FindFirstFile(strFindName.c_str(),&stfileinfo);
		if ( m_hRecordData == INVALID_HANDLE_VALUE )
		{
			return(MErrorCode::GetSysErr());
		}
		
		my_strncpy(lpsOut->szName,stfileinfo.cFileName,32);
		if ( (stfileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			lpsOut->lAttr = MFILE_ATTR_DIRECTORY;
		}
		else
		{
			lpsOut->lAttr = MFILE_ATTR_NORMALFILE;
		}
		lpsOut->lSize = stfileinfo.nFileSizeLow;
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftCreationTime), &lft );
		::FileTimeToSystemTime(&lft,&stsystime);
		lpsOut->mCreateTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftLastWriteTime), &lft );
		::FileTimeToSystemTime(&lft,&stsystime);
		lpsOut->mUpdateTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftLastAccessTime), &lft );
		::FileTimeToSystemTime(&(stfileinfo.ftLastAccessTime),&stsystime);
		lpsOut->mAccessTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		return(1);

	#else

		struct dirent		*		lpep;
		MFile						mfileptr;
		char						tempbuf[512];

		m_szFindPath = MFile::GetPathFromString(strFindName);
		m_szMatchFile = MFile::GetFileNameFromString(strFindName);
		/*
		 *	下面这个函数inner_checkmatch判断有问题和后续的MFindFile操作冲突.
		 *	这里无需判断 GUOGUO 2009-10-28
		 */
#if 0
		if ( inner_checkmatch(m_szMatchFile.c_str()) == false )
		{
			return(ERR_MFILE_LINUXMATCH);
		}
#endif
		m_stRecordData = opendir(m_szFindPath.c_str());
		if ( m_stRecordData == NULL )
		{
			return(MErrorCode::GetSysErr());
		}

		while( ( lpep = readdir( m_stRecordData )) != NULL )
		{
			if ( inner_match(lpep->d_name,m_szMatchFile.c_str()) == true )
			{
				my_strncpy(lpsOut->szName,lpep->d_name,256);
				//if ( (lpep->d_type & S_IFDIR) != 0 )
				/*
				 *	GUOGUO 2009-12-01 readdir 函数返回的
				 *	struct dirent lpep 的d_type字段 是DT_DIR来判断是否是目录
				 */
				if(lpep->d_type & DT_DIR)
				{
					lpsOut->lSize = MFILE_ATTR_DIRECTORY;
				}
				else
				{
					lpsOut->lSize = MFILE_ATTR_NORMALFILE;
				}
				
				lpsOut->lSize = lpep->d_reclen;
				
				snprintf(tempbuf,512,"%s/%s",m_szFindPath.c_str(),lpsOut->szName);
//				if( mfileptr.Open( tempbuf ) < 0 )	GUOGUO 20090723
				if( mfileptr.OpenRead( tempbuf ) < 0 )
				{
					lpsOut->mAccessTime = MDateTime(0);
					lpsOut->mCreateTime = MDateTime(0);
					lpsOut->mUpdateTime = MDateTime(0);
				}
				else
				{
					lpsOut->mAccessTime = mfileptr.GetAccessDateTime();
					lpsOut->mCreateTime = mfileptr.GetCreateDateTime();
					lpsOut->mUpdateTime = mfileptr.GetUpdateDateTime();
					
					mfileptr.Close(); 
				}
				return (1);
			}
		}

		return(MErrorCode::GetSysErr());

	#endif
}
//..............................................................................................................................
int  MFindFile::FindNext(tagMFindFileInfo * lpsOut)
{
	#ifndef LINUXCODE

		WIN32_FIND_DATA				stfileinfo;
		SYSTEMTIME					stsystime;
		FILETIME					lft;
		
		if ( ::FindNextFile(m_hRecordData,&stfileinfo) == 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		my_strncpy(lpsOut->szName,stfileinfo.cFileName,256);
		if ( (stfileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			lpsOut->lAttr = MFILE_ATTR_DIRECTORY;
		}
		else
		{
			lpsOut->lAttr = MFILE_ATTR_NORMALFILE;
		}
		lpsOut->lSize = stfileinfo.nFileSizeLow;
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftCreationTime), &lft );
		::FileTimeToSystemTime(&lft,&stsystime);
		lpsOut->mCreateTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftLastWriteTime), &lft );
		::FileTimeToSystemTime(&lft,&stsystime);
		lpsOut->mUpdateTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		::FileTimeToLocalFileTime( &(stfileinfo.ftLastAccessTime), &lft );
		::FileTimeToSystemTime(&lft,&stsystime);
		lpsOut->mAccessTime = MDateTime(stsystime.wYear,stsystime.wMonth,stsystime.wDay,stsystime.wHour,stsystime.wMinute,stsystime.wSecond);
		
		return(1);

	#else

		struct dirent		*		lpep;
		MFile						mfileptr;
		char						tempbuf[512];
		
		while( ( lpep = readdir( m_stRecordData )) != NULL )
		{
			if ( inner_match(lpep->d_name,m_szMatchFile.c_str()) == true )
			{
				my_strncpy(lpsOut->szName,lpep->d_name,256);
				//if ( (lpep->d_type & S_IFDIR) != 0 )
				/*
				 *	GUOGUO 2009-12-01 readdir 函数返回的
				 *	struct dirent lpep 的d_type字段 是DT_DIR来判断是否是目录
				 */
				if(lpep->d_type & DT_DIR)
				{
					lpsOut->lSize = MFILE_ATTR_DIRECTORY;
				}
				else
				{
					lpsOut->lSize = MFILE_ATTR_NORMALFILE;
				}
				
				lpsOut->lSize = lpep->d_reclen;
				
				snprintf(tempbuf,512,"%s/%s",m_szFindPath.c_str(),lpsOut->szName);
//				if( mfileptr.Open( tempbuf ) < 0 ) GUOGUO 20090723
				if( mfileptr.OpenRead( tempbuf ) < 0 )
				{
					lpsOut->mAccessTime = MDateTime(0);
					lpsOut->mCreateTime = MDateTime(0);
					lpsOut->mUpdateTime = MDateTime(0);
				}
				else
				{
					lpsOut->mAccessTime = mfileptr.GetAccessDateTime();
					lpsOut->mCreateTime = mfileptr.GetCreateDateTime();
					lpsOut->mUpdateTime = mfileptr.GetUpdateDateTime();
					
					mfileptr.Close(); 
				}
				return (1);
			}
		}
		
		return(MErrorCode::GetSysErr());

	#endif
}
//..............................................................................................................................
void MFindFile::CloseFind(void)
{
	#ifndef LINUXCODE

		if ( m_hRecordData != INVALID_HANDLE_VALUE )
		{
			::FindClose(m_hRecordData);
			m_hRecordData = INVALID_HANDLE_VALUE;
		}

	#else

		if ( m_stRecordData != NULL )
		{
			closedir(m_stRecordData);
			m_stRecordData = NULL;
		}
		
	#endif
}

//------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------------------------------
