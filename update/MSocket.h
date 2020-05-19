//------------------------------------------------------------------------------------------------------------------------------
//单元名称：socket单元
//单元描述：主要处理socket套节字相关
//创建日期：2007.3.16
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
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
//该类别主要是对TCP和UDP应用进行易用型扩展，所以很多函数都是针对TCP和UDP的，如果要使用其它协议，请直接使用标准socket函数，不
//需要使用该类别，该类别主要为了提高TCP/UDP应用的开发速度（最基本的提高就是不用调用WSAGetLastError()）
//..............................................................................................................................
class MSocket
{
protected:
	MSocket(void);
public:
	virtual ~MSocket();
public:
	//初始化网络环境
	static int  InstanceCondition();
	static int  InstanceCondition(unsigned char in_uchVer1 , unsigned char in_uchVer2 );


	//释放网络环境
	static void ReleaseCondition(void);
public:
	//设置同步
	static int  SetSync(SOCKET stIn);

	//设置异步
	static int  SetASync(SOCKET stIn);
public:
	//IP地址字符串形式转换为unsigned long型
	static unsigned long IPToUL(MString strIp);

	//IP地址unsigned long转换为字符串形式
	static MString ULToIP(unsigned long lIn);

	//域名转换为IP地址
	static MString DNSToIP(MString strDns);
public:
	//创建SOCKET（SOCK_STREAM、SOCK_DGRAM）
	static int  CreateSocket(SOCKET * lpSocketOut,int iType = SOCK_STREAM);
	static void Close( SOCKET s );

	//设置接收超时时间（毫秒）
	static int  SetRecvTimeOut(SOCKET stIn,unsigned long lTime);

	//设置发送超时时间（毫秒）
	static int  SetSendTimeOut(SOCKET stIn,unsigned long lTime);

	//设置TCP发送延迟取消，即取消Nagle算法，当发送一个字节时TCP封装包时会变成40字节包头，数据载量太小，Nagle算法是发送时当数据不到包最大值时（1400字节）需要待40毫秒，以等足够多的数据向外发送）
	static void Set_Tcp_Nodelay(SOCKET in_lSocket) ;

	//同步连接服务器 ，服务器地址可以是IP地址或域名地址
	static int  SyncConnect(SOCKET stSocket,MString strAddr,unsigned long lPort);

	//接受连接到达
	static SOCKET Accept(SOCKET stSocket,unsigned long * lpIpOut,unsigned short * lpPortOut);

	//邦定端口号
	static int  Bind(SOCKET stSocket,unsigned long lPort);

	//listen端口
	static int  Listen(SOCKET stSocket,unsigned long lLength = 100);

	//select的封装[超时单位为毫秒]
	static int  Select(unsigned long lMaxSocket,fd_set * stRead,fd_set * stWrite,fd_set * stExcept,unsigned long lTime);

	//写入的封装，注意：发送到0字节表示连接断开
	static int  Send(SOCKET stSocket,const char * lpIn,unsigned long lSize);

	//读取的封装，注意：接收到0字节表示连接断开
	static int  Recv(SOCKET stSocket,char * lpOut,unsigned long lSize);
public:
	//同步发生数据，直到发送完毕为止（在重复次数限制内），注意：发送到0字节表示连接断开
	static int  SyncSend(SOCKET stSocket,const char * lpIn,unsigned long lSize,int iTryTime = 3);
	
	//同步接收数据，直到接收完毕为止（在重复次数限制内），注意：接收到0字节表示连接断开
	static int  SyncRecv(SOCKET stSocket,char * lpOut,unsigned long lSize,int iTryTime = 3);
	//add by liuqy 20100129 for 增加快速关闭套接字, 在有大量连接并断开时closesocket，系统连接并没有立即释放，造成系统连接资源大都处于TIME_WAIT，而不接收新的连接，因此需要在将其修改为很快释放，对长连接有很好的帮助，但这对同步短连接是绝对不行，可能造成接收数据不完整。
	//快速关闭套接字,注意：同步短连接禁止调用该接口，否则不能保证完成发送给对方
	static void rv_FastClose( SOCKET in_lSocket );
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
