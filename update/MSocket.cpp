//------------------------------------------------------------------------------------------------------------------------------
#include "MSocket.h"
//------------------------------------------------------------------------------------------------------------------------------
#ifndef LINUXCODE
#pragma comment(lib,"ws2_32.lib")
#else
#include	<netinet/tcp.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
MSocket::MSocket(void)
{

}
//..............................................................................................................................
MSocket::~MSocket()
{

}
//..............................................................................................................................
int  MSocket::InstanceCondition()
{
	return InstanceCondition(1, 1);
}
int  MSocket::InstanceCondition(unsigned char in_uchVer1, unsigned char in_uchVer2 )
{
	#ifndef LINUXCODE

		WORD						wversion;
		WSADATA						wsadata;
		
		wversion = MAKEWORD(in_uchVer1, in_uchVer2);
		if ( ::WSAStartup(wversion,&wsadata) != 0 )
		{
			return(MErrorCode::GetSysErr());
		}
		
		return(1);

	#else

		return(1);
		
	#endif
}
//..............................................................................................................................
void MSocket::ReleaseCondition(void)
{
	#ifndef LINUXCODE
		
		::WSACleanup();

	#else
		
	#endif
}
//..............................................................................................................................
int  MSocket::SetSync(SOCKET stIn)
{
	#ifndef LINUXCODE

		register unsigned long						errorcode = 0;
		
		if ( ::ioctlsocket(stIn,FIONBIO,&errorcode) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}

	#else

		fcntl( stIn,F_SETFL,O_SYNC);
		return(1);

	#endif
}
//..............................................................................................................................
int  MSocket::SetASync(SOCKET stIn)
{
	#ifndef LINUXCODE

		register unsigned long						errorcode = 1;
		
		if ( ::ioctlsocket(stIn,FIONBIO,&errorcode) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}
		else
		{
			return(1);
		}

	#else

		fcntl( stIn,F_SETFL,O_NONBLOCK);
		return(1);

	#endif
}
//..............................................................................................................................
unsigned long MSocket::IPToUL(MString strIp)
{
	return(inet_addr(strIp.c_str()));
}
//..............................................................................................................................
MString MSocket::ULToIP(unsigned long lIn)
{
	struct in_addr			sttempaddr;
	
	sttempaddr.s_addr = lIn;
	return(inet_ntoa(sttempaddr));
}
//..............................................................................................................................
MString MSocket::DNSToIP(MString strDns)
{
	struct hostent				*	lpstHost;
	sockaddr_in                     stAddr;

	if ( inet_addr(strDns.c_str()) == INADDR_NONE )
	{
		if ( (lpstHost = gethostbyname(strDns.c_str())) == NULL )
		{
			//��Ч������
			return("");
		}

		memcpy((char *)&stAddr.sin_addr,(char *)lpstHost->h_addr,lpstHost->h_length);

		return(ULToIP(stAddr.sin_addr.s_addr));
	}
	else
	{
		//��������IP��ַ
		return(strDns);
	}
}
//..............................................................................................................................
int  MSocket::CreateSocket(SOCKET * lpSocketOut,int iType)
{
	(* lpSocketOut) = socket(AF_INET,iType,0);
	if ( (* lpSocketOut) == INVALID_SOCKET )
	{
		return(MErrorCode::GetSysErr());
	}

	return(1);
}
//..............................................................................................................................
int  MSocket::SetRecvTimeOut(SOCKET stIn,unsigned long lTime)
{
	register int					errorcode;

	#ifndef LINUXCODE

		errorcode = lTime;
		if ( setsockopt(stIn,SOL_SOCKET,SO_RCVTIMEO,(char *)&errorcode,sizeof(int)) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}

	#else
		//	GUOGUO 20090723
		struct timeval				sttimeset;

		sttimeset.tv_sec = lTime / 1000;
		sttimeset.tv_usec = 0;
		if ( setsockopt(stIn,SOL_SOCKET,SO_RCVTIMEO,(char *)&sttimeset,sizeof(sttimeset)) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}
			
	#endif

	return(1);
		
}
//..............................................................................................................................
int  MSocket::SetSendTimeOut(SOCKET stIn,unsigned long lTime)
{
	register int					errorcode;
	
	#ifndef LINUXCODE
		
		errorcode = lTime;
		if ( setsockopt(stIn,SOL_SOCKET,SO_SNDTIMEO,(char *)&errorcode,sizeof(int)) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}
		
	#else
		
		struct timeval				sttimeset;		// GUOGUO 20090723
		
		sttimeset.tv_sec = lTime / 1000;
		sttimeset.tv_usec = 0;
		if ( setsockopt(stIn,SOL_SOCKET,SO_SNDTIMEO,(char *)&sttimeset,sizeof(sttimeset)) == SOCKET_ERROR )
		{
			return(MErrorCode::GetSysErr());
		}
		
	#endif
		
	return(1);
}
//..............................................................................................................................
int  MSocket::SyncConnect(SOCKET stSocket,MString strAddr,unsigned long lPort)
{
	MString							stripaddr;
	sockaddr_in                     sttempaddr;

	stripaddr = DNSToIP(strAddr);
	
	sttempaddr.sin_family = AF_INET;
	sttempaddr.sin_addr.s_addr = inet_addr(stripaddr.c_str());
	sttempaddr.sin_port = htons(lPort);

	if ( connect(stSocket,(sockaddr *)&sttempaddr,sizeof(sttempaddr)) == SOCKET_ERROR )
	{
		return(MErrorCode::GetSysErr());
	}

	return(1);
}
//..............................................................................................................................
SOCKET  MSocket::Accept(SOCKET stSocket,unsigned long * lpIpOut,unsigned short * lpPortOut)
{
	struct sockaddr_in				sttempaddr;
	int								itemp;
	SOCKET							tempsocket;

	itemp = sizeof(sockaddr_in);

	#ifndef LINUXCODE
		tempsocket = accept(stSocket,(sockaddr *)&sttempaddr,&itemp);
	#else
		tempsocket = accept(stSocket,(sockaddr *)&sttempaddr,(socklen_t *)&itemp);
	#endif

	* lpIpOut = sttempaddr.sin_addr.s_addr;
	* lpPortOut = ntohs(sttempaddr.sin_port);

	return(tempsocket);
}
//..............................................................................................................................
int  MSocket::Bind(SOCKET stSocket,unsigned long lPort)
{
	struct sockaddr_in				sttempaddr;

	sttempaddr.sin_family = AF_INET;
	sttempaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sttempaddr.sin_port = htons(lPort);

	/*
	 *	GUOGUO 20090826 ���Ӽ���SOCKET��LINUX������
	 */
#ifdef LINUXCODE
	unsigned int	optval;
	struct	 linger	optval1;
	//  ����SO_REUSEADDRѡ��(��������������)
	optval   =   0x1;
	setsockopt(stSocket, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
            
	//����SO_LINGERѡ��(����CLOSE_WAIT��ס�����׽���)
	optval1.l_onoff = 1;
	optval1.l_linger = 60;
	setsockopt(stSocket, SOL_SOCKET, SO_LINGER, &optval1, sizeof(struct linger));
#endif

	if ( bind(stSocket,(sockaddr *)&sttempaddr,sizeof(sttempaddr)) == SOCKET_ERROR )
	{
		return(MErrorCode::GetSysErr());
	}

	return(1);
}
//..............................................................................................................................
int  MSocket::Listen(SOCKET stSocket,unsigned long lLength)
{
	if ( listen(stSocket,lLength) == SOCKET_ERROR )
	{
		return(MErrorCode::GetSysErr());
	}

	return(1);
}
//..............................................................................................................................
int  MSocket::Select(unsigned long lMaxSocket,fd_set * stRead,fd_set * stWrite,fd_set * stExcept,unsigned long lTime)
{
	timeval						sttimeout;
//	add by liuqy 20100115 for �׽��ַǷ�������������ɳ�ϵͳcore
	if(INVALID_SOCKET == lMaxSocket || 0xffffffff == lMaxSocket)
		return -1;
	//end add by liuqy 

	sttimeout.tv_sec = lTime / 1000;
	sttimeout.tv_usec = (lTime % 1000) * 1000;
	
	// modify by zhongjb 20120209 
	// In Linux, Function "select" return 0 when select timeout.
	int ret = select((int)(lMaxSocket + 1), stRead, stWrite, stExcept, &sttimeout);
#ifdef LINUXCODE
	if (0 == ret)
	{
		return 0;
	}
	else if (ret < 0)
	{
		return(MErrorCode::GetSysErr());
	}
#else
	if (ret <= 0)
	{
		return(MErrorCode::GetSysErr());
	}
#endif
	// end modify

	return(1);
}
//..............................................................................................................................
int  MSocket::Send(SOCKET stSocket,const char * lpIn,unsigned long lSize)
{
	register int				errorcode;

	while(1)
	{
		if ( (errorcode = send(stSocket,lpIn,lSize,MSG_NOSIGNAL)) == SOCKET_ERROR )
		{
#ifdef LINUXCODE
			errorcode = MErrorCode::GetSysErr();
			if(errorcode == MErrorCode::GetSysErr(EINTR))
			{
				/*
				*	NOTE:MSG_NOSIGNAL�������ᷢ��EINTR�ж��ź�.
				*/
				continue;
			}
			else
				return errorcode;
#else
			return(MErrorCode::GetSysErr());
#endif
		}
		else
		{
			break;
		}
	}
	
	return(errorcode);
}
//..............................................................................................................................
int  MSocket::Recv(SOCKET stSocket,char * lpOut,unsigned long lSize)
{
	register int				errorcode;

	while(1)
	{
		if ( (errorcode = recv(stSocket,lpOut,lSize,MSG_NOSIGNAL)) == SOCKET_ERROR )
		{
#ifdef LINUXCODE
			errorcode = MErrorCode::GetSysErr();
			if(errorcode == MErrorCode::GetSysErr(EINTR))
			{
				/*
				*	NOTE:MSG_NOSIGNAL�������ᷢ��EINTR�ж��ź�.
				*/
				continue;
			}
			else
				return errorcode;
#else
			return(MErrorCode::GetSysErr());
#endif
		}
		else
		{
			break;
		}
	}

	return(errorcode);
}
//..............................................................................................................................
int  MSocket::SyncSend(SOCKET stSocket,const char * lpIn,unsigned long lSize,int iTryTime)
{
	register int				itemptimes = 0;
	register int				ioffset = 0;
	register int				errorcode;

	while ( itemptimes < iTryTime && ioffset < lSize )
	{
		errorcode = Send(stSocket,lpIn + ioffset,lSize - ioffset);
		if ( errorcode > 0 )
		{
			ioffset += errorcode;
		}
#ifndef LINUXCODE
		else if ( errorcode == MErrorCode::GetSysErr(WSAETIMEDOUT) )
#else
		/*
		 *	Note:ETIMEOUT��������ǳ�ʱ����,�����������
		 *		ϵͳ��TCP���ݴ�������з����˳�ʱ,���������.
		 *		��������������ĳ�ʱ�ĺ���,�������ﳬʱ����EAGAIN
		 *		����.
		 *					--	GUOGUO
		 */
		else if ( errorcode == MErrorCode::GetSysErr(EAGAIN) )
#endif
		{
			itemptimes++;
			//���ͳ�ʱ
			// ��ʱ���˳� modify by deric 2009-12-24
			continue;
			// return(ERR_MSOCKET_TIMEOUT);
			// modify end
		}
		else
		{
			return(errorcode);
		}
	}

	if ( ioffset >= lSize )
	{
		return(lSize);
	}
	else
	{
		return(ERR_MSOCKET_TIMEOUT);
	}
}
//..............................................................................................................................
int  MSocket::SyncRecv(SOCKET stSocket,char * lpOut,unsigned long lSize,int iTryTime)
{
	register int				itemptimes = 0;
	register int				ioffset = 0;
	register int				errorcode;
	
	while ( itemptimes < iTryTime && ioffset < lSize )
	{	
		errorcode = Recv(stSocket,lpOut + ioffset,lSize - ioffset);
		if ( errorcode > 0 )
		{
			ioffset += errorcode;
		}
#ifndef LINUXCODE
		else if ( errorcode == MErrorCode::GetSysErr(WSAETIMEDOUT) )
#else
		/*
		 *	Note:ETIMEOUT��������ǳ�ʱ����,�����������
		 *		ϵͳ��TCP���ݴ�������з����˳�ʱ,���������.
		 *		��������������ĳ�ʱ�ĺ���,�������ﳬʱ����EAGAIN
		 *		����.
		 *					--	GUOGUO
		 */
		else if ( errorcode == MErrorCode::GetSysErr(EAGAIN) )
#endif
		{
			itemptimes++;
			//���ճ�ʱ
			// ��ʱ���˳� modify by deric 2009-12-24
			continue;
			// return(ERR_MSOCKET_TIMEOUT);
			// modify end
		}
		else
		{
			return(errorcode);
		}
	}
	
	if ( ioffset >= lSize )
	{
		return(lSize);
	}
	else
	{
		return(ERR_MSOCKET_TIMEOUT);
	}
}
//------------------------------------------------------------------------------------------------------------------------------
void MSocket::Close( SOCKET s )
{
#ifndef LINUXCODE
	if( s != INVALID_SOCKET )
	{
		closesocket( s );
	}
#else
	/*
	 *	GUOGUO 20090812 �׽��ӵ��ж� Ӧ���� > 0 ��Ϊ�Ϸ����׽���
	 *	��LINUX������ > 0��
	 */
	if( s > 0 )
	{
/*		{
			struct tm					sttemptime;
			time_t	stNow;
			time(&stNow);
			localtime_r(&stNow, &sttemptime);
			
			FILE * p= fopen("./closeex.txt", "a");
			if(NULL != p)
			{
				int size = 16;
				int i;
				void * array[16];
				struct timeb	tp;
				ftime(&tp);
				struct sockaddr_in sa;
				socklen_t len = sizeof(sa);
				if(!getpeername(s, (struct sockaddr *)&sa, &len))
				{
					fprintf(p, "%d-%d %d:%d:%d.%d  [%d] connect [%s:%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
						tp.millitm ,  s, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));				
				}
				else
				{
					fprintf(p, "%d-%d %d:%d:%d.%d [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
						tp.millitm ,  s);
				}
				int stack_num = backtrace(array, size);
				char ** stacktrace = backtrace_symbols(array, stack_num);
				
				if(NULL != stacktrace)
				{
					for (i=0; i<stack_num; i++)
						fprintf(p, "%s\n", stacktrace[i]);
					free(stacktrace);
				}
				fclose(p);	
			}
			else
				printf("open ./closeex.txt error no:%d\n", errno);
		}
*/
		shutdown(s, 2); // �ر�sockfd�Ķ�д���ܡ� add by anzl 20171023
		close( s );
	}
#endif
}
//modify by liuqy 20130105 for ���ٹر��׽���,ֻ�г����ӷ��������ܵ��øú���,ע�⣺ͬ�������ӽ�ֹ���øýӿڣ������ܱ�֤��ɷ��͸��Է�
void MSocket::rv_FastClose( SOCKET in_lSocket )
{
	linger sLinger;
	sLinger.l_onoff = 1;  
	sLinger.l_linger = 0; 
#ifndef LINUXCODE
	if(INVALID_SOCKET != in_lSocket)
#else
	if(0 < in_lSocket)
#endif
		setsockopt(in_lSocket, SOL_SOCKET, SO_LINGER, (const char*)&sLinger, sizeof(linger));
	
#ifndef LINUXCODE
	if( in_lSocket != INVALID_SOCKET )
	{
		closesocket( in_lSocket );
	}
#else
	if( in_lSocket > 0 )
	{
/*		{
			struct tm					sttemptime;
			time_t	stNow;
			time(&stNow);
			localtime_r(&stNow, &sttemptime);
			
			FILE * p= fopen("./closeex.txt", "a");
			if(NULL != p)
			{
				int size = 16;
				int i;
				void * array[16];
				struct timeb	tp;
				ftime(&tp);
				struct sockaddr_in sa;
				socklen_t len = sizeof(sa);
				if(!getpeername(in_lSocket, (struct sockaddr *)&sa, &len))
				{
					fprintf(p, "%d-%d %d:%d:%d.%d  [%d] connect [%s:%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
						tp.millitm , in_lSocket, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));				
				}
				else
				{
					fprintf(p, "%d-%d %d:%d:%d.%d [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
						tp.millitm , in_lSocket);
				}
				int stack_num = backtrace(array, size);
				char ** stacktrace = backtrace_symbols(array, stack_num);
				
				//fprintf(p, "%d-%d %d:%d:%d.%d %s(%d) [%d]\n", sttemptime.tm_mon + 1,  sttemptime.tm_mday, sttemptime.tm_hour,  sttemptime.tm_min, sttemptime.tm_sec, 
				//	tp.millitm , file, line, s);
				if(NULL != stacktrace)
				{
					for (i=0; i<stack_num; i++)
						fprintf(p, "%s\n", stacktrace[i]);
					free(stacktrace);
				}
				fclose(p);	
			}
			else
				printf("open ./closeex.txt error no:%d\n", errno);
 		} */
		close( in_lSocket );
	}
#endif
}

//����TCP�����ӳ�ȡ������ȡ��Nagle�㷨��������һ���ֽ�ʱTCP��װ��ʱ����40�ֽڰ�ͷ����������̫С��Nagle�㷨�Ƿ���ʱ�����ݲ��������ֵʱ��1400�ֽڣ���Ҫ��40���룬�Ե��㹻����������ⷢ�ͣ�
void  MSocket::Set_Tcp_Nodelay(SOCKET in_lSocket) 
{
	int enable = 1;
#ifndef LINUXCODE
	if( in_lSocket != INVALID_SOCKET )
	{
		setsockopt( in_lSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&enable, sizeof(enable));  
	}
#else
	if( in_lSocket > 0 )
	{
		setsockopt(in_lSocket, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
	}
#endif
}

//------------------------------------------------------------------------------------------------------------------------------

