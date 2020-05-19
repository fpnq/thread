//------------------------------------------------------------------------------------------------------------------------------
#ifdef LINUXCODE
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#endif
#include "MProcess.h"
//------------------------------------------------------------------------------------------------------------------------------
MProcess::MProcess(void)
{
	#ifndef LINUXCODE
		m_hRecordData = NULL;
		m_lRecordId = 0;
	#else
		m_hRecordData = -1;
	#endif
}
//------------------------------------------------------------------------------------------------------------------------------
MProcess::~MProcess()
{
	KillProcess();
}
//------------------------------------------------------------------------------------------------------------------------------
int  MProcess::CreateProcess(MString strFileName,MString strRunParam, bool bKill, bool bShow, const char * pszDir)
{
	if( bKill )
	{
		KillProcess();
	}

	#ifndef LINUXCODE

		STARTUPINFO					ststartupinfo;
		PROCESS_INFORMATION			stprocessinfo;
		MString						tempstring;
		
		memset((char *)&ststartupinfo,0x00,sizeof(STARTUPINFO));
		ststartupinfo.cb = sizeof(ststartupinfo);
		ststartupinfo.dwFlags = STARTF_USESHOWWINDOW ;

		if( bShow )
		{
			ststartupinfo.wShowWindow = SW_SHOW;
		}
		else
		{
			ststartupinfo.wShowWindow = SW_HIDE;
		}

		if ( ::CreateProcess(	(char *)strFileName.c_str(),
								(char *)strRunParam.c_str(),
								NULL,
								NULL,
								TRUE,
								CREATE_NEW_CONSOLE,
								NULL,
								pszDir,
								&ststartupinfo,
								&stprocessinfo	) == NULL )
		{
			return(MErrorCode::GetSysErr());
		}

		m_hRecordData = stprocessinfo.hProcess;
		m_lRecordId = stprocessinfo.dwProcessId;
		
		return(1);

	#else

		/*
		 *	���ȼ���ļ��Ƿ������,������Ȩ�޷��ʺ�ִ��
		 *	��Ϊ������fork��,�Ѿ��Ǹ��ӽ�����,
		 *	�ͱȽ��Ѵ������������.
		 *	������ǰ�ھ������ܼ�����п��ܴ��ڵ�����.
		 */
		struct stat _buf;

		if(stat(strFileName.c_str(), &_buf) != 0)
		{
			/*	�ļ������ڻ����ļ�û�з���Ȩ��*/
			return (MErrorCode::GetSysErr());
		}
		if( !(S_IXUSR & _buf.st_mode) )
		{
			/*	û��ִ��Ȩ��*/
			return (MErrorCode::GetSysErr(EACCES));
		}


		m_hRecordData = fork();
		if ( m_hRecordData == -1 )
		{
			//����ʧ��
			return(MErrorCode::GetSysErr());
		}
		else if ( m_hRecordData == 0 )
		{
			//	�����Ǹ�BUG:
			//	add by ����,��Ҫexit
			//	���execlʧ��,���ص����ӽ���ȥִ�и����̵Ĵ���Ƭ��
			//	����һ��exit��֤execlʧ�ܵ����
			//	���execl�ɹ�,����Զ���ᵽexit(0)
			if(setsid() < 0)
			{
				printf("setsid() error %d\n", errno);fflush(stdout);
				exit(errno);	//	Note:ǧ����return,�������ӽ���
			}

			//�����ɹ���ִ���ӽ���
			exit(execl(strFileName.c_str(), strFileName.c_str(), strRunParam.c_str(), NULL));
		}
		else
		{
			return 1;
		}

	#endif
}
//------------------------------------------------------------------------------------------------------------------------------
void MProcess::KillProcess(void)
{
	#ifndef LINUXCODE

		if ( m_hRecordData != NULL )
		{
			::TerminateProcess(m_hRecordData,0);
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}
		m_lRecordId = 0;

	#else

		if ( m_hRecordData != -1 )
		{
			kill(m_hRecordData,SIGKILL);
			m_hRecordData = -1;
		}

	#endif
}
//------------------------------------------------------------------------------------------------------------------------------
bool MProcess::IsActive(void)
{
	#ifndef LINUXCODE
		
		register unsigned long				errorcode = 0;
		
		if ( m_hRecordData == NULL )
		{
			return(false);
		}
		
		::GetExitCodeProcess(m_hRecordData,&errorcode);
		if ( errorcode == STILL_ACTIVE )
		{
			return(true);
		}
		else
		{
			return(false);
		}

	#else

		register  int				errorcode = 0;

		if ( m_hRecordData == -1 )
		{
			return(false);
		}

		if ( waitpid(m_hRecordData,&errorcode,WNOHANG) == 0 )
		{
			return(false);
		}
		else
		{
			return(true);
		}

	#endif
}
//------------------------------------------------------------------------------------------------------------------------------

#ifndef LINUXCODE

unsigned long MProcess::GetProcessId()
{
	return m_lRecordId;
}

#else

pid_t MProcess::GetProcessId()
{
	return m_hRecordData;
}

#endif
