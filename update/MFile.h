//------------------------------------------------------------------------------------------------------------------------------
//单元名称：文件单元
//单元描述：主要处理文件、目录等相关操作
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
#ifndef __MEngine_MFileH__
#define __MEngine_MFileH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MDateTime.h"
#include <sys/stat.h>  // windows 下调用stat函数，anzl 20091211
#ifdef	LINUXCODE
#include <utime.h>
#include <fcntl.h>
#include <dirent.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
enum																					//文件的读取模式
{
	MFILE_OPENMODE_READ = 0x01,															//打开文件读取
	MFILE_OPENMODE_WRITE = 0x02,														//打开文件写
	MFILE_OPENMODE_READWRITE = MFILE_OPENMODE_READ | MFILE_OPENMODE_WRITE,				//打开文件读写
	MFILE_OPENMODE_NOTHING = 0x00,														//不允许共享
	MFILE_OPENMODE_SHAREREAD = 0x04,													//允许共享读
	MFILE_OPENMODE_SHAREWRITE = 0x08,													//允许共享写
	MFILE_OPENMODE_SHAREALL = MFILE_OPENMODE_SHAREREAD | MFILE_OPENMODE_SHAREWRITE,		//允许共享读写
};
//..............................................................................................................................
//为了兼容Windows操作系统和Linux操作系统，所以把他们特别的地方给去掉了（如Windows下的只读属性/Linux是根据群组、个人等而言的属性）
enum																					//文件属性
{
	MFILE_ATTR_NORMALFILE = 0x01,														//普通文件
	MFILE_ATTR_DIRECTORY = 0x02,														//目录
};
//------------------------------------------------------------------------------------------------------------------------------
class MFile
{
protected:
	#ifndef LINUXCODE
		HANDLE						m_hRecordData;
	#else
		long						m_lRecordData;
	#endif
protected:
	MString							m_strFileName;
public:
	MFile(void);
	virtual ~MFile();
public:
	//打开文件
	int  Open(MString strFileName,unsigned long lMode);
	//打开文件读取
	int  OpenRead(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READ | MFILE_OPENMODE_SHAREALL);
	//打开文件写入
	int  OpenWrite(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READWRITE | MFILE_OPENMODE_SHAREALL);
	//创建文件
	int  Create(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READWRITE | MFILE_OPENMODE_SHAREALL);
	//关闭文件
	void Close(void);
	//获取当前文件是否已经打开
	bool IsOpen(void);
	//获取文件名称
	MString GetFileName(void);
public:
	//读取数据、写入数据、设置偏移
	int  Seek(unsigned long lOffset,unsigned long lType);
	//20110712 by tangj 因为使用了负数,所以回退代码,另外新增seek64
	__int64  Seek64(__int64 i64Offset,unsigned long lType);
	
	int  Read(char * lpOut,unsigned long lSize);
	int  Write(const char * lpIn,unsigned long lSize);
	
	//完全读取和写入，一旦读取写入数据字节和预期不一返回错误
	int  ReadAll(char * lpOut,unsigned long lSize);
	int  WriteAll(const char * lpIn,unsigned long lSize);
	
	//将缓冲的数据刷新到文件
	void Flush(void);
public:
	//读取（以行为单位，如果有其他内容则不包括回车换行，如果没有其他内容则仅只有回车换行）、写入字符串
	//例如：
	//
	//第一行：this is a test
	//第二行：
	//第三行：end
	//
	//读取第一行和第三行时仅仅只有英文字符（不包括回车换行），但读取第二行时只有回车换行
	MString ReadString(unsigned long lMaxSize = 512);
	//下面这个函数是全部读取出来，包括回车环行等字符（所有字符）
	int  ReadString(char * strOut,unsigned long lMaxSize);
	int  ReadString(MString * strOut,unsigned long lMaxSize = 512);
	int  WriteString(const char * strIn);
	int  WriteString(MString strIn);
public:
	//获取设置文件长度
	int  SetFileLength(unsigned long lSize);
	//以当前位置作为结束位置
	int  SetFileLength(void);
	int  GetFileLength(void);
public:
	//加锁与解琐文件
	int  LockFile(unsigned long lOffset,unsigned long lSize);
	int  UnLockFile(unsigned long lOffset,unsigned long lSize);
public:
	//获取设置文件的相关日期和时间，注意：Linux下不能设置文件的创建日期
	MDateTime GetCreateDateTime(void);
	MDateTime GetUpdateDateTime(void);
	MDateTime GetAccessDateTime(void);
	//注意：Linux下不能设置创建日期
	int  SetCreateDateTime(MDateTime mIn);
	int  SetUpdateDateTime(MDateTime mIn);
	int  SetAccessDateTime(MDateTime mIn);
	

#ifndef LINUXCODE
	HANDLE	GetFileHandle(){return	m_hRecordData;};
#else
	long	GetFileHandle(){return		m_lRecordData;};
#endif


public:
	//静态函数，对文件操作的函数集合
	// 通过stat直接获得文件的更新日期，anzl 20091211
	static long GetUpdateDateTimeByStat(MString strFileName);
	// 通过stat直接获得文件的大小，anzl 20100810
	static long GetFileSizeByStat(MString strFileName);
	//add by liuqy 20110517 for 同时取更新时间和文件大小，以减少IO操作
	static bool GetFileSizeUpdateTime(MString in_strFileName, long & out_lFileSize, long & out_lUpdateTime); 
	static int  RenameFileName(MString strSourceFile,MString strNewFile);
	static int  DeleteFile(MString strFileName);
	static int  CreateDirectory(MString strFileName);
	static int  DeleteDirectory(MString strFileName);
	static int  CreateDirectoryTree(MString strDirTreeName);
	static int  DeleteDirectoryTree(MString strDirTreeName);
	//注意，下面函数返回的路径包括"\\或/"
	static MString GetCurrentDirectory(void);
	static int  SetCurrentDirectory(MString strIn);
	static bool IsExist(MString strFileName);
	static int  CopyFile(MString strSourceFile,MString strNewFile,bool bFailIfExists);
	static int  DeleteDirAllFile(MString strDirName);
public:
	//校验文件
	static unsigned long GetFileCheckCode(MString strFilePath);
	static unsigned int GetFileCRC32Code(const char *lpszFileName);
public:
	//从字符串中分离出路径信息
	static MString GetPathFromString(MString strIn);
	//从字符串中分离出文件名信息
	static MString GetFileNameFromString(MString strIn);
};
//------------------------------------------------------------------------------------------------------------------------------
#pragma pack(1)
//..............................................................................................................................
typedef struct								//查找文件得到的属性
{
	char				szName[256];		//文件名称
	unsigned long		lSize;				//文件长度
	unsigned long		lAttr;				//文件属性
	MDateTime			mCreateTime;		//创建时间
	MDateTime			mUpdateTime;		//更新时间
	MDateTime			mAccessTime;		//访问时间
} tagMFindFileInfo;
//..............................................................................................................................
#pragma pack()
//------------------------------------------------------------------------------------------------------------------------------
class MFindFile
{
protected:
	#ifndef LINUXCODE

		HANDLE						m_hRecordData;

	#else

		DIR						*	m_stRecordData;
		MString						m_szFindPath;
		MString						m_szMatchFile;

	#endif
protected:
	//Linux检测匹配字符串的合法性
	__inline bool inner_checkmatch(const char * strMatch);
	//注意：Linux操作系统下必须自己匹配如?和*这样的匹配符号
	__inline bool inner_match(const char * strSource,const char * strMatch);
public:
	MFindFile(void);
	virtual ~MFindFile();
public:
	//注意：匹配字符串不允许出现**、*?、?*等非法匹配内容
	int  FindFirst(MString strFindName,tagMFindFileInfo * lpsOut);
	int  FindNext(tagMFindFileInfo * lpsOut);
	void CloseFind(void);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
