//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ��ַ�����Ԫ
//��Ԫ��������Ҫ�����ַ�����ز���
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
#ifndef __MEngine_MStringH__
#define __MEngine_MStringH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MTypeDefine.h"
//------------------------------------------------------------------------------------------------------------------------------
#define MSTRING_DEFAULT_SIZE					256				//ȱʡ�ַ�������
//..............................................................................................................................
class MString
{
protected:
	char				*	m_lpszStringPtr;
	char					m_szDefaultBuf[MSTRING_DEFAULT_SIZE];
	unsigned long			m_lMaxSize;
protected:
	//��ʼ���ַ���������Ա�ṹ
	__inline void inner_initstring(void);
	//���ַ������·���ռ䣬����ռ��Ѿ����㣬���ٷ���
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
	//��������أ���������
	MString & operator = (const MString & strIn);
	MString & operator = (const char * strIn);
	
	//��������أ��������㣬�����޸�����
	MString & operator += (const MString & strIn);
	MString & operator += (const char * strIn);
	
	//��������أ��������㣬��Ӱ������
	MString   operator + (const MString & strIn);
	MString   operator + (const char * strIn);
	friend MString operator + (const char * strIn1,const MString & strIn2);
public:
	//��������أ��Ƚ�����
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
	//ת��char *����
	const char * c_str(void);
	
	//��ȡ�ַ�������
	unsigned long GetLength(void);
	
	//ת���ַ���λ��д�ַ�
	void Upper(void);
	
	//ת���ַ���λСд�ַ�
	void Lower(void);
	
	//�ж��ַ����Ƿ�Ϊ�գ���==""
	bool IsEmpty(void);
	
	//����ַ���
	void Empty(void);
	
	//��ʽ�������ַ�����ע�⣺���ַ������ȵ����ƣ������ȴʡ�ģ�δ����ģ���ΪMSTRING_DEFAULT_SIZE��
	void Format(const char * szIn,...);
	
	//��ȡָ��λ�õ��ַ������ز�������Խ�緵��0
	char operator [] (unsigned int iNo);
	
	//����ָ��λ�õ��ַ�������Խ�磬���ش���
	int  SetAt(unsigned int iNo,char cValue);
	
	//��ȡָ���м�λ�õ��ַ�����û�л�Խ�緵��""
	MString Mid(unsigned int iFirst,unsigned int iCount);

	//��ȡָ�����λ�õ��ַ�����û�л�Խ�緵��""
	MString Left(unsigned int iCount);

	//��ȡָ���ұ�λ�õ��ַ�����û�л�Խ�緵��""
	MString Right(unsigned int iCount);

	//��ȡ��һ��ָ��ƥ���ַ���λ��ǰ���ַ��������û��ƥ�䷵��""
	MString GetMatchFrontString(MString strMatch);

	//��ȡ��һ��ָ��ƥ���ַ���λ�ú���ַ��������û��ƥ�䷵��""
	MString GetMatchBackString(MString strMatch);

	//��ȡ��һ��ָ��ƥ���ַ���λ��ǰ����ַ��������û��ƥ�䷵��""
	int  GetMatchString(MString strMatch,MString * strFront,MString * strBack);

	//�����ַ����������ַ�������λ�ã���0��ʼ������ʧ�ܷ���<0
	int  StringPosition(MString strFind);

	//�����߿�ʼ�������ո�TAB�ַ� 
	void TrimLeft(void);

	//����ұ߿�ʼ�������ո�TAB�ַ�
	void TrimRight(void);

	//�������Ŀո��TAB�ַ�ת����Ϊ��һ�Ŀո񣬲��ҹ��˵���ߺ��ұߵĿո��TAB�ַ�������"    A   B  C D  "ת����Ϊ"A B C D"
	MString TrimMiddleToSpace(void);
public:
	//����ת������
	long ToLong(void);
	unsigned long ToULong(void);
	double ToDouble(void);
	bool ToBool(void);
public:
	//��ʽ�������ַ�����ע�⣺��MSTRING_DEFAULT_SIZE * 10�ַ����ȵ�����
	static MString FormatString(const char * strFormat,...);

	//�����ַ�����Ϊ��������׼strncpy�����ڲ���iCountʱ������0������
	static char * strncpy(char * strDest,const char * strSource,size_t iCount);
	
	//�����ַ�����Ϊ��������׼strncat�����ڲ���iCountʱ������0������
	static char * strncat(char * strDest,const char * strSource,size_t iCount);

	//��ӡ�ַ�����Ϊ��������׼snprintf�����ڲ���iCountʱ������0������
	static void _snprintf(char * lpszBuffer,size_t lCount,const char * lpszFormat,...);
	
	//����ת������
	static long ToLong(const char * strValue);
	static unsigned long ToULong(const char * strValue);
	static double ToDouble(const char * strValue);
	static bool ToBool(const char * strValue);

	//�����߻��ұ߿�ʼ�������ո�TAB�ַ�
	static void TrimLeft(char * strIn);
	static void TrimRight(char * strIn);
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
