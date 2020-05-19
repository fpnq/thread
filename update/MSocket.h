//------------------------------------------------------------------------------------------------------------------------------
//��Ԫ���ƣ�socket��Ԫ
//��Ԫ��������Ҫ����socket�׽������
//�������ڣ�2007.3.16
//������Ա��¬��Զ
//�޸ļ�¼
//�޸�����		�޸���Ա	�޸����ݺ�ԭ��
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MSocketH__
#define __MEngine_MSocketH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
//..............................................................................................................................
#ifndef LINUXCODE
//..............................................................................................................................
#include <winsock2.h>
//..............................................................................................................................
#else
//..............................................................................................................................
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
//..............................................................................................................................
#endif
//------------------------------------------------------------------------------------------------------------------------------
//�������Ҫ�Ƕ�TCP��UDPӦ�ý�����������չ�����Ժܶຯ���������TCP��UDP�ģ����Ҫʹ������Э�飬��ֱ��ʹ�ñ�׼socket��������
//��Ҫʹ�ø���𣬸������ҪΪ�����TCP/UDPӦ�õĿ����ٶȣ����������߾��ǲ��õ���WSAGetLastError()��
//..............................................................................................................................
class MSocket
{
protected:
	MSocket(void);
public:
	virtual ~MSocket();
public:
	//��ʼ�����绷��
	static int  InstanceCondition();
	static int  InstanceCondition(unsigned char in_uchVer1 , unsigned char in_uchVer2 );


	//�ͷ����绷��
	static void ReleaseCondition(void);
public:
	//����ͬ��
	static int  SetSync(SOCKET stIn);

	//�����첽
	static int  SetASync(SOCKET stIn);
public:
	//IP��ַ�ַ�����ʽת��Ϊunsigned long��
	static unsigned long IPToUL(MString strIp);

	//IP��ַunsigned longת��Ϊ�ַ�����ʽ
	static MString ULToIP(unsigned long lIn);

	//����ת��ΪIP��ַ
	static MString DNSToIP(MString strDns);
public:
	//����SOCKET��SOCK_STREAM��SOCK_DGRAM��
	static int  CreateSocket(SOCKET * lpSocketOut,int iType = SOCK_STREAM);
	static void Close( SOCKET s );

	//���ý��ճ�ʱʱ�䣨���룩
	static int  SetRecvTimeOut(SOCKET stIn,unsigned long lTime);

	//���÷��ͳ�ʱʱ�䣨���룩
	static int  SetSendTimeOut(SOCKET stIn,unsigned long lTime);

	//����TCP�����ӳ�ȡ������ȡ��Nagle�㷨��������һ���ֽ�ʱTCP��װ��ʱ����40�ֽڰ�ͷ����������̫С��Nagle�㷨�Ƿ���ʱ�����ݲ��������ֵʱ��1400�ֽڣ���Ҫ��40���룬�Ե��㹻����������ⷢ�ͣ�
	static void Set_Tcp_Nodelay(SOCKET in_lSocket) ;

	//ͬ�����ӷ����� ����������ַ������IP��ַ��������ַ
	static int  SyncConnect(SOCKET stSocket,MString strAddr,unsigned long lPort);

	//�������ӵ���
	static SOCKET Accept(SOCKET stSocket,unsigned long * lpIpOut,unsigned short * lpPortOut);

	//��˿ں�
	static int  Bind(SOCKET stSocket,unsigned long lPort);

	//listen�˿�
	static int  Listen(SOCKET stSocket,unsigned long lLength = 100);

	//select�ķ�װ[��ʱ��λΪ����]
	static int  Select(unsigned long lMaxSocket,fd_set * stRead,fd_set * stWrite,fd_set * stExcept,unsigned long lTime);

	//д��ķ�װ��ע�⣺���͵�0�ֽڱ�ʾ���ӶϿ�
	static int  Send(SOCKET stSocket,const char * lpIn,unsigned long lSize);

	//��ȡ�ķ�װ��ע�⣺���յ�0�ֽڱ�ʾ���ӶϿ�
	static int  Recv(SOCKET stSocket,char * lpOut,unsigned long lSize);
public:
	//ͬ���������ݣ�ֱ���������Ϊֹ�����ظ����������ڣ���ע�⣺���͵�0�ֽڱ�ʾ���ӶϿ�
	static int  SyncSend(SOCKET stSocket,const char * lpIn,unsigned long lSize,int iTryTime = 3);
	
	//ͬ���������ݣ�ֱ���������Ϊֹ�����ظ����������ڣ���ע�⣺���յ�0�ֽڱ�ʾ���ӶϿ�
	static int  SyncRecv(SOCKET stSocket,char * lpOut,unsigned long lSize,int iTryTime = 3);
	//add by liuqy 20100129 for ���ӿ��ٹر��׽���, ���д������Ӳ��Ͽ�ʱclosesocket��ϵͳ���Ӳ�û�������ͷţ����ϵͳ������Դ�󶼴���TIME_WAIT�����������µ����ӣ������Ҫ�ڽ����޸�Ϊ�ܿ��ͷţ��Գ������кܺõİ����������ͬ���������Ǿ��Բ��У�������ɽ������ݲ�������
	//���ٹر��׽���,ע�⣺ͬ�������ӽ�ֹ���øýӿڣ������ܱ�֤��ɷ��͸��Է�
	static void rv_FastClose( SOCKET in_lSocket );
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
