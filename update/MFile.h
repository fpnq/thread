//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ��ļ���Ԫ
//��Ԫ��������Ҫ�����ļ���Ŀ¼����ز���
//�������ڣ�2007.3.14
//������Ա��¬��Զ
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
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
#include <sys/stat.h>  // windows �µ���stat������anzl 20091211
#ifdef	LINUXCODE
#include <utime.h>
#include <fcntl.h>
#include <dirent.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
enum																					//�ļ��Ķ�ȡģʽ
{
	MFILE_OPENMODE_READ = 0x01,															//���ļ���ȡ
	MFILE_OPENMODE_WRITE = 0x02,														//���ļ�д
	MFILE_OPENMODE_READWRITE = MFILE_OPENMODE_READ | MFILE_OPENMODE_WRITE,				//���ļ���д
	MFILE_OPENMODE_NOTHING = 0x00,														//��������
	MFILE_OPENMODE_SHAREREAD = 0x04,													//�������
	MFILE_OPENMODE_SHAREWRITE = 0x08,													//������д
	MFILE_OPENMODE_SHAREALL = MFILE_OPENMODE_SHAREREAD | MFILE_OPENMODE_SHAREWRITE,		//�������д
};
//..............................................................................................................................
//Ϊ�˼���Windows����ϵͳ��Linux����ϵͳ�����԰������ر�ĵط���ȥ���ˣ���Windows�µ�ֻ������/Linux�Ǹ���Ⱥ�顢���˵ȶ��Ե����ԣ�
enum																					//�ļ�����
{
	MFILE_ATTR_NORMALFILE = 0x01,														//��ͨ�ļ�
	MFILE_ATTR_DIRECTORY = 0x02,														//Ŀ¼
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
	//���ļ�
	int  Open(MString strFileName,unsigned long lMode);
	//���ļ���ȡ
	int  OpenRead(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READ | MFILE_OPENMODE_SHAREALL);
	//���ļ�д��
	int  OpenWrite(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READWRITE | MFILE_OPENMODE_SHAREALL);
	//�����ļ�
	int  Create(MString strFileName,unsigned long lMode = MFILE_OPENMODE_READWRITE | MFILE_OPENMODE_SHAREALL);
	//�ر��ļ�
	void Close(void);
	//��ȡ��ǰ�ļ��Ƿ��Ѿ���
	bool IsOpen(void);
	//��ȡ�ļ�����
	MString GetFileName(void);
public:
	//��ȡ���ݡ�д�����ݡ�����ƫ��
	int  Seek(unsigned long lOffset,unsigned long lType);
	//20110712 by tangj ��Ϊʹ���˸���,���Ի��˴���,��������seek64
	__int64  Seek64(__int64 i64Offset,unsigned long lType);
	
	int  Read(char * lpOut,unsigned long lSize);
	int  Write(const char * lpIn,unsigned long lSize);
	
	//��ȫ��ȡ��д�룬һ����ȡд�������ֽں�Ԥ�ڲ�һ���ش���
	int  ReadAll(char * lpOut,unsigned long lSize);
	int  WriteAll(const char * lpIn,unsigned long lSize);
	
	//�����������ˢ�µ��ļ�
	void Flush(void);
public:
	//��ȡ������Ϊ��λ����������������򲻰����س����У����û�������������ֻ�лس����У���д���ַ���
	//���磺
	//
	//��һ�У�this is a test
	//�ڶ��У�
	//�����У�end
	//
	//��ȡ��һ�к͵�����ʱ����ֻ��Ӣ���ַ����������س����У�������ȡ�ڶ���ʱֻ�лس�����
	MString ReadString(unsigned long lMaxSize = 512);
	//�������������ȫ����ȡ�����������س����е��ַ��������ַ���
	int  ReadString(char * strOut,unsigned long lMaxSize);
	int  ReadString(MString * strOut,unsigned long lMaxSize = 512);
	int  WriteString(const char * strIn);
	int  WriteString(MString strIn);
public:
	//��ȡ�����ļ�����
	int  SetFileLength(unsigned long lSize);
	//�Ե�ǰλ����Ϊ����λ��
	int  SetFileLength(void);
	int  GetFileLength(void);
public:
	//����������ļ�
	int  LockFile(unsigned long lOffset,unsigned long lSize);
	int  UnLockFile(unsigned long lOffset,unsigned long lSize);
public:
	//��ȡ�����ļ���������ں�ʱ�䣬ע�⣺Linux�²��������ļ��Ĵ�������
	MDateTime GetCreateDateTime(void);
	MDateTime GetUpdateDateTime(void);
	MDateTime GetAccessDateTime(void);
	//ע�⣺Linux�²������ô�������
	int  SetCreateDateTime(MDateTime mIn);
	int  SetUpdateDateTime(MDateTime mIn);
	int  SetAccessDateTime(MDateTime mIn);
	

#ifndef LINUXCODE
	HANDLE	GetFileHandle(){return	m_hRecordData;};
#else
	long	GetFileHandle(){return		m_lRecordData;};
#endif


public:
	//��̬���������ļ������ĺ�������
	// ͨ��statֱ�ӻ���ļ��ĸ������ڣ�anzl 20091211
	static long GetUpdateDateTimeByStat(MString strFileName);
	// ͨ��statֱ�ӻ���ļ��Ĵ�С��anzl 20100810
	static long GetFileSizeByStat(MString strFileName);
	//add by liuqy 20110517 for ͬʱȡ����ʱ����ļ���С���Լ���IO����
	static bool GetFileSizeUpdateTime(MString in_strFileName, long & out_lFileSize, long & out_lUpdateTime); 
	static int  RenameFileName(MString strSourceFile,MString strNewFile);
	static int  DeleteFile(MString strFileName);
	static int  CreateDirectory(MString strFileName);
	static int  DeleteDirectory(MString strFileName);
	static int  CreateDirectoryTree(MString strDirTreeName);
	static int  DeleteDirectoryTree(MString strDirTreeName);
	//ע�⣬���溯�����ص�·������"\\��/"
	static MString GetCurrentDirectory(void);
	static int  SetCurrentDirectory(MString strIn);
	static bool IsExist(MString strFileName);
	static int  CopyFile(MString strSourceFile,MString strNewFile,bool bFailIfExists);
	static int  DeleteDirAllFile(MString strDirName);
public:
	//У���ļ�
	static unsigned long GetFileCheckCode(MString strFilePath);
	static unsigned int GetFileCRC32Code(const char *lpszFileName);
public:
	//���ַ����з����·����Ϣ
	static MString GetPathFromString(MString strIn);
	//���ַ����з�����ļ�����Ϣ
	static MString GetFileNameFromString(MString strIn);
};
//------------------------------------------------------------------------------------------------------------------------------
#pragma pack(1)
//..............................................................................................................................
typedef struct								//�����ļ��õ�������
{
	char				szName[256];		//�ļ�����
	unsigned long		lSize;				//�ļ�����
	unsigned long		lAttr;				//�ļ�����
	MDateTime			mCreateTime;		//����ʱ��
	MDateTime			mUpdateTime;		//����ʱ��
	MDateTime			mAccessTime;		//����ʱ��
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
	//Linux���ƥ���ַ����ĺϷ���
	__inline bool inner_checkmatch(const char * strMatch);
	//ע�⣺Linux����ϵͳ�±����Լ�ƥ����?��*������ƥ�����
	__inline bool inner_match(const char * strSource,const char * strMatch);
public:
	MFindFile(void);
	virtual ~MFindFile();
public:
	//ע�⣺ƥ���ַ������������**��*?��?*�ȷǷ�ƥ������
	int  FindFirst(MString strFindName,tagMFindFileInfo * lpsOut);
	int  FindNext(tagMFindFileInfo * lpsOut);
	void CloseFind(void);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
