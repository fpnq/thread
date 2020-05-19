//------------------------------------------------------------------------------------------------------------------------------
#include "MShareMem.h"
#include "../BaseUnit/MThread.h"
//------------------------------------------------------------------------------------------------------------------------------
MShareMem::MShareMem(void)
{
	#ifndef LINUXCODE
		m_hRecordData = NULL;
		m_lpData = NULL;
	#else
		m_hRecordData = -1;
		m_lpData = (unsigned char *)-1;
	#endif

	m_iSize = 0;
	m_bCreate = false;
}
//..............................................................................................................................
MShareMem::~MShareMem()
{
	Close();
}
//..............................................................................................................................
int  MShareMem::inner_createkeyfromstring(const char * strIn)
{
	register int					i;
	register int					errorcode = (int)strIn[0];
	
	for ( i=1;i<strlen(strIn);i++ )
	{
		switch ( i % 3 )
		{
			case 0:					errorcode += (int)strIn[i];
									break;
			case 1:					errorcode -= (int)strIn[i];
									break;
			case 2:					errorcode *= (int)strIn[i];
									break;
		}
	}
	
	return((errorcode > 0) ? (errorcode) : ((-1) * errorcode));
}
//..............................................................................................................................
int  MShareMem::Create(MString strName,unsigned long lSize,bool bAnyAccess)
{
	register int				errorcode;

	Close();

	//创建同步对象
	if ( (errorcode = m_mMutex.Create(strName + "Mutex")) < 0 )
	{
		return(errorcode);
	}
	
	#ifndef LINUXCODE

		SECURITY_DESCRIPTOR			sd;
		SECURITY_ATTRIBUTES			sa;
		LPSECURITY_ATTRIBUTES		pSa = NULL;

		if( bAnyAccess )
		{
			InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
			SetSecurityDescriptorDacl( &sd, true, NULL, false );   
			sa.nLength = sizeof( sa );   
			sa.lpSecurityDescriptor = &sd;   
			sa.bInheritHandle = FALSE;   

			pSa = &sa;
		}

		m_hRecordData = ::CreateFileMapping(NULL,pSa,PAGE_READWRITE,0,lSize+4,strName.c_str());
		if ( m_hRecordData == NULL )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( (m_lpData = ::MapViewOfFile(m_hRecordData,FILE_MAP_ALL_ACCESS,0,0,0)) == NULL )
		{
			errorcode = MErrorCode::GetSysErr();
			Close();

			return(errorcode);
		}
		
		memset((char *)m_lpData,0x00,lSize + 4);
		memcpy((char *)m_lpData,(char *)&lSize,4);
		m_iSize = lSize;

		m_bCreate = true;

		return(1);

	#else

		register int				ikey = inner_createkeyfromstring(strName.c_str());

		if ( (m_hRecordData = shmget(ikey,lSize + 4,0666 | IPC_CREAT)) == -1 )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( (long)(m_lpData = (unsigned char *)shmat(m_hRecordData,0,0)) == -1 )
		{
			errorcode = MErrorCode::GetSysErr();
			Close();
			
			return(errorcode);
		}

		memset((char *)m_lpData,0x00,lSize + 4);
		memcpy((char *)m_lpData,(char *)&lSize,4);
		m_iSize = lSize;
		
		m_bCreate = true;
		
		return(1);
		
	#endif
}
//..............................................................................................................................
int  MShareMem::Open(MString strName)
{
	register int					errorcode;

	Close();

	//打开同步对象
	if ( (errorcode = m_mMutex.Open(strName + "Mutex")) < 0 )
	{
		return(errorcode);
	}
	
	#ifndef LINUXCODE

		m_hRecordData = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,strName.c_str());
		if ( m_hRecordData == NULL )
		{
			return(MErrorCode::GetSysErr());
		}
		
		if ( (m_lpData = ::MapViewOfFile(m_hRecordData,FILE_MAP_ALL_ACCESS,0,0,0)) == NULL )
		{
			errorcode = MErrorCode::GetSysErr();
			Close();
			return(errorcode);
		}
		
		memcpy((char *)&m_iSize,m_lpData,4);

		m_bCreate = false;

		return(1);

	#else

		register int				ikey = inner_createkeyfromstring(strName.c_str());

		if ( (m_hRecordData = shmget(ikey,LB_SHARE_MEM_SIZE,SHM_R | SHM_W)) == -1 )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( (long)(m_lpData = (unsigned char *)shmat(m_hRecordData,0,0)) == -1 )
		{
			errorcode = MErrorCode::GetSysErr();
			Close();
			
			return(errorcode);
		}

		memcpy((char *)&m_iSize,m_lpData,4);
		
		m_bCreate = false;
		
		return (1);

	#endif
}
//..............................................................................................................................
void MShareMem::Close(void)
{
	#ifndef LINUXCODE

		if ( m_lpData != NULL )
		{
			UnmapViewOfFile(m_lpData);
			m_lpData = NULL;
		}
	
		if ( m_bCreate == true )
		{
			m_bCreate = false;
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}

	#else

		if((long)m_lpData != -1)
		{
			shmdt(m_lpData);
			m_lpData = (unsigned char *)-1;
		}

		if ( m_hRecordData != -1 )
		{
			shmctl(m_hRecordData,IPC_RMID,0);
			m_hRecordData = -1;
		}

	#endif

	m_mMutex.Close();
}
//..............................................................................................................................
int  MShareMem::Read(unsigned long lOffset,char * lpOut,unsigned long lSize)
{
	MLocalMutex						mlocalmutex;

#ifndef LINUXCODE
	if ( m_hRecordData == NULL || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#else
	if ( m_hRecordData == -1 || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#endif

	if ( (lOffset + lSize) > m_iSize-4 )
	{
		return(ERR_PUBLIC_SLOPOVER);
	}

	mlocalmutex.Attch(&m_mMutex);

	memcpy(lpOut,(char *)m_lpData + lOffset + 4,lSize);

	mlocalmutex.UnAttch();

	return(lSize);
}
//..............................................................................................................................
int  MShareMem::Write(unsigned long lOffset,const char * lpIn,unsigned long lSize)
{
	MLocalMutex						mlocalmutex;

#ifndef LINUXCODE
	if ( m_hRecordData == NULL || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#else
	if ( m_hRecordData == -1 || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#endif
	
	if ( (lOffset + lSize) > m_iSize-4 )
	{
		return(ERR_PUBLIC_SLOPOVER);
	}
	
	mlocalmutex.Attch(&m_mMutex);

	memcpy((char *)m_lpData + lOffset + 4,lpIn,lSize);

	mlocalmutex.UnAttch();
	
	return(lSize);
}
//..............................................................................................................................
int  MShareMem::GetSize(void)
{
	return(m_iSize);
}
//------------------------------------------------------------------------------------------------------------------------------
MShareMemMt::MShareMemMt()
{
	#ifndef LINUXCODE
		m_hRecordData = NULL;
		m_lpData = NULL;
	#else
		m_hRecordData = -1;
		m_lpData = NULL;
	#endif

	m_iSize = 0;
	m_bCreate = false;
}
//------------------------------------------------------------------------------------------------------------------------------
MShareMemMt::~MShareMemMt()
{
	Close();
}
//------------------------------------------------------------------------------------------------------------------------------
void MShareMemMt::Close()
{
#ifndef LINUXCODE
		if ( m_lpData != NULL )
		{
			UnmapViewOfFile(m_lpData);
			m_lpData = NULL;
		}
	
		if ( m_bCreate == true )
		{
			m_bCreate = false;
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}

#else
		if((long)m_lpData != -1)
		{
			shmdt(m_lpData);
			m_lpData = (unsigned char *)-1;
		}

		if ( m_hRecordData != -1 )
		{
			shmctl(m_hRecordData,IPC_RMID,0);
			m_hRecordData = -1;
		}
#endif

	m_mMutex.Close();
}
//------------------------------------------------------------------------------------------------------------------------------
int MShareMemMt::Create( MString strName, unsigned long ulSize )
{
	int						nErrorCode = -1;
	
	Close();

	//创建同步对象
	if ( ( nErrorCode = m_mMutex.Create( strName + "Mutex" ) ) < 0 )
	{
		return( nErrorCode );
	}

#ifndef LINUXCODE
		m_hRecordData = ::CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
											 0, ulSize+8, strName.c_str() );
		if ( m_hRecordData == NULL )
		{
			return( MErrorCode::GetSysErr() );
		}

		if ( ( m_lpData = ::MapViewOfFile( m_hRecordData, FILE_MAP_ALL_ACCESS,
										   0, 0, 0 ) ) == NULL )
		{
			nErrorCode = MErrorCode::GetSysErr();
			Close();
			return( nErrorCode );
		}
		
		m_iSize = ulSize+8;
		memset( m_lpData, 0, m_iSize  );
		memcpy( (char *)m_lpData+4, &ulSize, 4 );
		
#else
		int						ikey = inner_createkeyfromstring( strName.c_str() );

		if ( ( m_hRecordData = shmget( ikey, ulSize + 4, 0666 | IPC_CREAT ) ) == -1 )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( (long)(m_lpData = (unsigned char *)shmat( m_hRecordData, 0, 0 ) ) == -1 )
		{
			nErrorCode = MErrorCode::GetSysErr();
			Close();
			
			return(nErrorCode);
		}

		memset( (char *)m_lpData, 0x00, ulSize + 4 );
		memcpy( (char *)m_lpData+4, (char *)&ulSize, 4 );
		m_iSize = ulSize;
#endif
		m_bCreate = true;
		return(1);
}
//------------------------------------------------------------------------------------------------------------------------------
int  MShareMemMt::Open(MString strName)
{
	int						nErroCode = -1;

	Close();

	//打开同步对象
	if ( ( nErroCode = m_mMutex.Open(strName + "Mutex") ) < 0 )
	{
		return( nErroCode );
	}

#ifndef LINUXCODE

		m_hRecordData = ::OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, strName.c_str() );
		if ( m_hRecordData == NULL )
		{
			return (MErrorCode::GetSysErr() );
		}
		if ( (m_lpData = ::MapViewOfFile( m_hRecordData, FILE_MAP_ALL_ACCESS, 
										  0, 0, 0 ) ) == NULL )
		{
			nErroCode = MErrorCode::GetSysErr();
			Close();
			return(nErroCode);
		}
		
		memcpy( &m_iSize, (char *)m_lpData+4, 4 );
#else
		int					ikey = inner_createkeyfromstring(strName.c_str());

		if ( (m_hRecordData = shmget(ikey,LB_SHARE_MEM_SIZE,SHM_R | SHM_W)) == -1 )
		{
			return(MErrorCode::GetSysErr());
		}

		if ( (long)(m_lpData = (unsigned char *)shmat(m_hRecordData,0,0)) == -1 )
		{
			nErroCode = MErrorCode::GetSysErr();
			Close();
			
			return(nErroCode);
		}

		memcpy((char *)&m_iSize,(char *)m_lpData+4,4);
#endif
		m_bCreate = false;
		return(1);
}
//------------------------------------------------------------------------------------------------------------------------------
int  MShareMemMt::GetSize(void)
{
	return(m_iSize);
}
//------------------------------------------------------------------------------------------------------------------------------
int  MShareMemMt::Read(unsigned long ulOffset,char * lpOut,unsigned long ulSize)
{
	MLocalMutex					mlocalmutex;
	char *						pShareMem = NULL;
	bool						bWaitRead = false;

#ifndef LINUXCODE
	if ( m_hRecordData == NULL || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#else
	if ( m_hRecordData == -1 || m_lpData == NULL )
	{
		return(ERR_MSHAREMEM_NOINS);
	}
#endif

	if ( ( ulOffset + ulSize ) > m_iSize - 8 )
	{
		return(ERR_PUBLIC_SLOPOVER);
	}

	//先拿到读许可，然后读共享内存，读完后交还许可;
	//共享内存前四个字节为读写标志,依次为读进程数量，
	//等待读的进程数量,当前写标志,等待写的进程数量．
	pShareMem = (char *)m_lpData;
	while( true )
	{
		mlocalmutex.Attch( &m_mMutex );
		if( pShareMem[2] > 0 )
		{
			mlocalmutex.UnAttch();
			MThread::Sleep( 50 );
			continue;
		}
		else 
		{
			if( pShareMem[0] < 127 )
			{
				++pShareMem[0];
				if( bWaitRead )
				{
					--pShareMem[1];
				}
				mlocalmutex.UnAttch();
				break;
			}
			else
			{
				if( !bWaitRead )
				{
					bWaitRead = true;
					++pShareMem[1];
				}
				mlocalmutex.UnAttch();
				MThread::Sleep( 50 );
				continue;
			}
		}
	}

	memcpy( lpOut, (char *)m_lpData + ulOffset + 8, ulSize );
	
	mlocalmutex.Attch( &m_mMutex );
	assert( pShareMem[0] > 0 );
	if( pShareMem[0] > 0 )
	{
		--pShareMem[0];
	}
	mlocalmutex.UnAttch();

	return(ulSize);
}
//------------------------------------------------------------------------------------------------------------------------------
int  MShareMemMt::Write( unsigned long ulOffset,const char * lpIn,unsigned long ulSize)
{
	MLocalMutex					mlocalmutex;
	char *						pShareMem = NULL;
	bool						bWaitWrite = false;

#ifndef LINUXCODE
	if ( m_hRecordData == NULL || m_lpData == NULL )
	{
		return( ERR_MSHAREMEM_NOINS );
	}
#else
	if ( m_hRecordData == -1 || m_lpData == NULL )
	{
		return( ERR_MSHAREMEM_NOINS );
	}
#endif

	if( lpIn == NULL )
	{
		return ( ERR_PUBLIC_SLOPOVER );
	}
	
	if ( ( ulOffset + ulSize ) > m_iSize - 8 )
	{
		return(ERR_PUBLIC_SLOPOVER);
	}
	
	pShareMem = (char *)m_lpData;
	while( true )
	{
		mlocalmutex.Attch( &m_mMutex );
		
		if( pShareMem[0] > 0 || pShareMem[1] > 0 )
		{
			mlocalmutex.UnAttch();
			MThread::Sleep( 50 );
			continue;
		}
		else
		{
			if( pShareMem[2] > 0 )
			{
				if( !bWaitWrite )
			
				{
					bWaitWrite = true;
					++pShareMem[3];
				}
				mlocalmutex.UnAttch();
				MThread::Sleep( 50 );
				continue;
			}
			else
			{
				pShareMem[2] = 1;
				if( bWaitWrite )
				{
					--pShareMem[3];
				}
				mlocalmutex.UnAttch();
				break;
			}
		}

	}

	memcpy( (char *)m_lpData + ulOffset + 8, lpIn, ulSize );

	mlocalmutex.Attch( &m_mMutex );
	assert( pShareMem[2] > 0 );
	if( pShareMem[2] > 0 )
	{
		pShareMem[2] = -1;
	}
	mlocalmutex.UnAttch();

	return(ulSize);
}
//------------------------------------------------------------------------------------------------------------------------------
int  MShareMemMt::inner_createkeyfromstring( const char * strIn )
{
	register int					i;
	register int					errorcode = (int)strIn[0];
	
	for ( i=1;i<strlen(strIn);i++ )
	{
		switch ( i % 3 )
		{
			case 0:					errorcode += (int)strIn[i];
									break;
			case 1:					errorcode -= (int)strIn[i];
									break;
			case 2:					errorcode *= (int)strIn[i];
									break;
		}
	}
	
	return((errorcode > 0) ? (errorcode) : ((-1) * errorcode));
}
