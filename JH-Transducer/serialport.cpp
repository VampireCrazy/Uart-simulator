/*
**	FILENAME			CSerialPort.cpp
**
**	AUTHOR				Mending by Wang Feng
**
**
*/

#include "stdafx.h"
#include "SerialPort.h"

#include <assert.h>

//Constructor
CSerialPort::CSerialPort()
{
	m_hComm = NULL;

	//initalize overlapped structure members to zero
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;

	//create events
	m_ov.hEvent = NULL;
	m_hWriteEvent = NULL;
	m_hShutdownEvent = NULL;
	m_szWriteBuffer = NULL;
	m_bThreadAlive = FALSE;
	m_nSendLength = 0;

}

CSerialPort::~CSerialPort()
{
	do
	{
		SetEvent(m_hShutdownEvent);
	}while(m_bThreadAlive);

	if(m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	if(m_hShutdownEvent != NULL)
	{
		CloseHandle(m_hShutdownEvent);
	}

	if(m_ov.hEvent != NULL)
	{
		CloseHandle(m_ov.hEvent);
	}

	if(m_hWriteEvent != NULL)
	{
		CloseHandle(m_hWriteEvent);
	}

	TRACE("Thread ended\n");

	delete [] m_szWriteBuffer;

}

BOOL CSerialPort::InitPort(CWnd* pPortOwner, //the owner (CWnd) of the port ��receives message��
							UINT portnr,	 //portnumber (1..10)
							UINT baud,		//baudrate
							char parity,	//parity
							UINT databits,	//databits
							UINT stopbits,	//stopbits
							DWORD dwCommEvents,	//EVRXCHAR,EV_CTS etc
							UINT writebuffersize //size to the writebuffer
							)
{
	assert(portnr > 0 && portnr < 11);
	assert(pPortOwner != NULL);

	//if the thread is alive: Kill
	if(m_bThreadAlive)
	{
		do
		{
			SetEvent(m_hShutdownEvent);
		}while(m_bThreadAlive);
		TRACE("Thread ended\n");
	}

	//�����¼�
	//m_ov �� OVERLAPPED �ṹ����
	//����3�� CreateEvent ����Ӧ�þ�Ϊ�ֹ���λ��ʽ

	if(m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);
	m_ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	if(m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);
	m_hWriteEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	if(m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);
	m_hShutdownEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	//initialize the event objects
	m_hEventArray[0] = m_hShutdownEvent;  //��ߵ����ȼ�
	m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;

	//��ʼ���ؼ�������
	InitializeCriticalSection(&m_csCommunicationSync);

	//set buffersize for writing and save the owner
	m_pOwner = pPortOwner;

	if(m_szWriteBuffer != NULL)
		delete [] m_szWriteBuffer;
	m_szWriteBuffer = new char[writebuffersize];

	m_nPortNr = portnr;

	m_nWriteBufferSize = writebuffersize;
	m_dwCommEvents = dwCommEvents;

	BOOL bResult = FALSE;
	char *szPort = new char[50];
	char *szBaud = new char[50];

	//����ؼ���

	EnterCriticalSection(&m_csCommunicationSync);

	//��������Ǵ򿪵� �ر���
	if(m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	//Ϊ���ڵõ��߼�����������ʼ������ʱ��ֻҪ���ֱ�ʾ���ں�  �� 1 ��ʾ COM1
	sprintf(szPort,"COM%d",portnr);
	sprintf(szBaud,"baud=%d parity=%c data=%d stop=%d",baud, parity, databits, stopbits);

	WCHAR wszPort[256];  
	memset(wszPort,0,sizeof(wszPort));  
	MultiByteToWideChar(CP_ACP,0,szPort,strlen(szPort)+1,wszPort,  
	sizeof(wszPort)/sizeof(wszPort[0]));

	//�õ�Ҫ�򿪵Ĵ��ھ�������Ժ����ʹ��
	m_hComm = CreateFile(wszPort,								//���ں�COMX
						GENERIC_READ | GENERIC_WRITE,			//����ͬʱ��д
						0,										//�Զ�ռ�ķ�ʽ�򿪴���
						NULL,									//�ް�ȫ����
						OPEN_EXISTING,							//�����豸������OPEN_EXISTING,��Ϊ���϶�����
						FILE_FLAG_OVERLAPPED,					//�첽I/O������ʽ
						0);										//�Դ����豸��ģ���ļ��������Ϊ0��NULL��

	//�򿪴��ڲ��ɹ������д�����
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		delete [] szPort;
		delete [] szBaud;

		return FALSE;
	}

	//���ö������ʱ����m_CommTimeouts
	m_CommTimeouts.ReadIntervalTimeout = 100;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

	//��ʼ�����ڣ����ô��ڲ�����setCommState
	if(SetCommTimeouts(m_hComm,&m_CommTimeouts))// ���ó�ʱ
	{
		if(SetCommMask(m_hComm,dwCommEvents))  //����ͨ���¼�
		{
			if(GetCommState(m_hComm, &m_dcb))  //��ȡ��ǰDCB�ṹ����
			{
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;  //����RTC bit high��
				WCHAR wszBaud[256];  
				memset(wszBaud,0,sizeof(wszBaud));  
				MultiByteToWideChar(CP_ACP,0,szBaud,strlen(szBaud)+1,wszBaud,  
				sizeof(wszBaud)/sizeof(wszBaud[0]));
				if(BuildCommDCB(wszBaud,&m_dcb))				//��д DCB �ṹ
				{
					if(SetCommState(m_hComm,&m_dcb))		//������д�õ� DCB ����
						;//normal operation... continue
					else
						ProcessErrorMessage("SetCommState()");
				}
				else
					ProcessErrorMessage("BuildCommDCB()");
			}
			else
				ProcessErrorMessage("GetCommState()");
		}
		else
			ProcessErrorMessage("SetCommMask()");
	}
	else
		ProcessErrorMessage("SetCommTimeouts()");

	delete [] szPort;
	delete [] szBaud;

	//flush the port
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	//release critical section
	LeaveCriticalSection(&m_csCommunicationSync);

	TRACE("Initialisation for communicationport %d completed.\nUse start monitor to communicate.\n",portnr);

	return TRUE;
}

//The CommThread Function.

UINT CSerialPort::CommThread(LPVOID pParam)
{
	//Case the void pointer passed to the thread back to 
	//a pointer of CSerialPort class

	CSerialPort *port = (CSerialPort *)pParam;

	//Set the status variable in the dialog class to 
	//TRUE to indicate the thread is running
	port->m_bThreadAlive = TRUE;

	//Misc. variables
	DWORD BytesTransfered = 0;
	DWORD Event = 0;
	DWORD CommEvent = 0;
	DWORD dwError = 0;
	COMSTAT comstat;
	BOOL bResult = TRUE;

	//���� PurgeComm ������� ������
	if(port->m_hComm)		//����Ƿ��
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	for(;;)
	{
		/*///////////////////////////////////////////////////////////////////// 
		   ͨ���¼����Ӻ��� WaitCommEvent() �ڵ���ʱ���������������ء�
	   ������Ϊ���ھ����ʱ ����Ϊ FILE_FLAG_OVERLAPPED �첽 I/O ������ʽ��
	   
		   WaitCommEvent �����ڼ���ͨ���¼�ʱ������첽����������������ɣ�
	   ��ú������� FALSE(0)�� ���� GetLastError �������� Error_IO_PENDING��
	   �����ò������ں�ִ̨�С�

		   �����������ʱ��ϵͳ�� WaitCommEvent ��������֮ǰ�� OVERLAPPED �ṹ
	   �еĳ�Ա hEvent ����ֵ����Ϊ ���ź� ״̬��֮��ȵ����ض�ͨ���¼�����
	   ����ʱ��ϵͳ�ٽ�������Ϊ ���ź� ״̬������ֻҪ���ַ�����,�ͻ�����¼�.
		/////////////////////////////////////////////////////////////////////*/

		bResult = WaitCommEvent(port->m_hComm,&Event,&port->m_ov);

		if(!bResult)
		{
			//��� WaitCommEvent ����ֵΪNULL�� ���� GetLastError ������ѯ������Ϣ
			switch(dwError = GetLastError())
			{
			case ERROR_IO_PENDING:
				{
					//�������������˵��û���ַ��ɶ�
					break;
				}
			case 87:
				{
					//87������ϵͳ����Error_INVALID_PARAMTER ����Ӱ�����
					break;
				}
			default:
				{
					//����������������򷵻ش�����Ϣ
					port->ProcessErrorMessage("WaitCommEvent()");
					break;
				}

			}
		}
		else
		{
			/* ��� WaitCommEvent() ���� TRUE, ��� ���ջ����� ���Ƿ����ַ��ɶ���  
			
			       ע�⣺�����һ�ξʹӻ������ж�������ֽڣ�����û������������
			   ��Ҫ��WaitForMultipleObjects ������

				   ����һ���ַ����ﻺ����ʱ������ WaitForMultipleObjects ��������
			   ��������ʱ�� ���첽 I/O �����¼���� m_OverlappedStruct.hEvent ����
			   ��λ��������Ϊ ���ź� ״̬��
			   
				   ����ڸ�λ�����󵽵��� ReadFile ����֮���������ַ��ﵽ��
			   m_OverlappedStruct.hEvent�ᱻ��������,�� ����Ϊ���ź�״̬.��ʱ����
			   ReadFile����, ReadFile �������ӻ������ж����������ݣ�Ȼ��ѭ���ص�
			   WaitCommEvent()
			*/

			// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
			// but there are no bytes available to read.  If you proceed and call
			// ReadFile(), it will return immediatly due to the async port setup, but
			// GetOverlappedResults() will not return until the next character arrives.
			//
			// It is not desirable for the GetOverlappedResults() function to be in 
			// this state.  The thread shutdown event (event 0) and the WriteFile()
			// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
			//
			// The solution to this is to check the buffer with a call to ClearCommError().
			// This call will reset the event handle, and if there are no bytes to read
			// we can loop back through WaitCommEvent() again, then proceed.
			// If there are really bytes to read, do nothing and proceed.

			bResult = ClearCommError(port->m_hComm,&dwError,&comstat);

			if(comstat.cbInQue == 0)
				continue;
		}//end if bResult

		//���������ȴ��������������������̣߳�ֱ������Ӧ���¼�����
		Event = WaitForMultipleObjects(3,							//�¼���������е��¼���
										port->m_hEventArray,		//�¼�����
										FALSE,						
										INFINITE);
		switch(Event)
		{
		case 0:
			{
				//�ر��¼�����Ϊ���¼���0 ���Ծ�����ߵ����ȼ��������ȱ�ִ��
				CloseHandle(port->m_hComm);
				port->m_hComm = NULL;
				port->m_bThreadAlive = FALSE;

				//Kill this thread. brrak is not needed ,but makes me fell better.
				AfxEndThread(100);
				break;
			}
		case 1:  //���¼��У�������ĸ�����Ϣ���ͳ�ȥ
			{
				// ����Ӧ����Ϣ������ OnComm �п���������������Ӧ��GetCommMask �������õ�
				GetCommMask(port->m_hComm,&CommEvent);
				
				if(CommEvent & EV_CTS)
					::SendMessage(port->m_pOwner->m_hWnd,WM_COMM_CTS_DETECTED,(WPARAM)0,(LPARAM)port->m_nPortNr);
				if (CommEvent & EV_BREAK)
				::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_BREAK_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);

				if (CommEvent & EV_ERR)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_ERR_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);

				if (CommEvent & EV_RING)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_RING_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);

				if (CommEvent & EV_RXFLAG)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_RXFLAG_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);

				if (CommEvent & EV_TXEMPTY)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_TXEMPTY_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);
				
				if (CommEvent & EV_RXCHAR)
					// Receive character event from port.
					ReceiveChar(port, comstat);
	
				break;
				
			}

		case 2: //�����¼�
			{
				WriteChar(port);
				break;
			}
		}//end switch
	}//close forever loop
	return 0;
}

//��ʼ���Ӵ���Start comm watching
BOOL CSerialPort::StartMonitoring()
{
	if (!(m_Thread = AfxBeginThread(CommThread, this)))
		return FALSE;
	TRACE("Thread started\n");
	return TRUE;	
}

// �������������߳�
BOOL CSerialPort::RestartMonitoring()
{
	TRACE("Thread resumed\n");
	m_Thread->ResumeThread();
	return TRUE;	
}

// ���𴮿��߳�
// ��ǰ ��������߳���������,����κ�һ���̵߳Ĺ���������0,��ô���̲߳��ᱻִ��.
// ������ͨ������ ResumeThread ��Ա����,ʹ�ô��̱߳��ָ�ִ��
BOOL CSerialPort::StopMonitoring()
{
	TRACE("Thread suspended\n");
	m_Thread->SuspendThread(); 
	return TRUE;	
}

//����д��͸�����ȷ����Ϣ
void CSerialPort::ProcessErrorMessage(char * ErrorText)
{
	char * Temp = new char[200];

	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	sprintf(Temp,"WARNING: %s Failed with the following error:\n%s\nPort: %d\n",(char*)ErrorText,lpMsgBuf,m_nPortNr);
	WCHAR wszClassName[256];  
	memset(wszClassName,0,sizeof(wszClassName));  
	MultiByteToWideChar(CP_ACP,0,Temp,strlen(Temp)+1,wszClassName,  
	sizeof(wszClassName)/sizeof(wszClassName[0]));
	MessageBox(NULL,wszClassName,_T("Application Error0"),MB_ICONSTOP);

	LocalFree(lpMsgBuf);
	delete [] Temp;
}

//Write a character
void CSerialPort::WriteChar(CSerialPort* port)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;

	DWORD BytesSent = 0;

	//��λд�¼����
	ResetEvent(port->m_hWriteEvent);
	//Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);

	if(bWrite)
	{
		// Initailize variables
		port->m_ov.Offset = 0;
		port->m_ov.OffsetHigh = 0;
	
	// ��ջ�����
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		bResult = WriteFile(port->m_hComm,							// Handle to COMM Port
							port->m_szWriteBuffer,					// Pointer to message buffer in calling finction
							port->m_nSendLength,					// Length of message to send
							&BytesSent,								// Where to store the number of bytes sent
							&port->m_ov);							// Overlapped structure
		// ���� FALSE��������
		if (!bResult)  
		{
			DWORD dwError = GetLastError();
			switch (dwError)
			{
				case ERROR_IO_PENDING:
					{
						// ���Ǵ��󣬶����첽I/O �������ڽ��У���Ҫ���� GetOverlappedResults ��ѯ���
						BytesSent = 0;
						bWrite = FALSE;
						break;
					}
				default:
					{
						// all other error codes
						port->ProcessErrorMessage("WriteFile()");
					}
			}
		} 
		else
		{
			LeaveCriticalSection(&port->m_csCommunicationSync);
		}
	} // end if(bWrite)

	if (!bWrite)
	{
		bWrite = TRUE;
	
		bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
									  &port->m_ov,		// Overlapped structure
									  &BytesSent,		// Stores number of bytes sent
									  TRUE); 			// Wait flag

		LeaveCriticalSection(&port->m_csCommunicationSync);

		// deal with the error code 
		if (!bResult)  
		{
			port->ProcessErrorMessage("GetOverlappedResults() in WriteFile()");
		}	
	} // end if (!bWrite)

	// Verify that the data size send equals what we tried to send
	if (BytesSent != strlen((char *)port->m_szWriteBuffer))
	{
		TRACE("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, strlen((char *)port->m_szWriteBuffer));
	}

}

//Character received.Inform the owner
void CSerialPort::ReceiveChar(CSerialPort* port,COMSTAT comstat)
{
	BOOL bRead = TRUE;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	RECEIVEDATA data;
	unsigned char *RXBuff = data.RXBuff;

	for(;;)
	{
		/*     critical section �ܱ�֤��ͬһʱ����ֻ��һ���߳��в�
		   ��ĳһ��Դ��Ȩ�ޡ�����Ӧ���� EnterCriticalSection ������
		   ��ô��ڵ� critical section ��һ���̿ɱ�֤�����򣨽��̣�
		   ��û�������������߳���ʹ�ñ�������Դ
		*/

		EnterCriticalSection(&port->m_csCommunicationSync);

		//ʹ�� ClearCommError() ������ COMSTAT �ṹ�� �����ͨ�Ŵ���
		bResult = ClearCommError(port->m_hComm,&dwError,&comstat);

		LeaveCriticalSection(&port->m_csCommunicationSync);

		
		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		//��������ַ������������ж�ѭ��
		if(comstat.cbInQue == 0)
		{
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);

		//
		if(bRead)
		{
			//Ӧ�� ReadFile ���������������е��ֽ�
			bResult = ReadFile(port->m_hComm,			//	Handle to COMM port
								RXBuff,					//  RX Buffer Pointer
								3,						//  Read one  byte
								&BytesRead,				//  Stores number of bytes read
								&port->m_ov);			//	pointer to  the  m_ov structer
			//�� ReadFile ���� FALSE�� ���� GetLastError() ������
			if(!bRead)
			{
				switch(dwError = GetLastError())
				{
					case ERROR_IO_PENDING: 	
						{ 
							// �첽I/O �������ڽ���,��ʱ��Ҫ���� GetOverlappedResults ��������ѯ
							bRead = FALSE;
							break;
						}
					default:
						{
							// ����������
							port->ProcessErrorMessage("ReadFile()");
							break;
						} 

				}
			}
			else
			{
				// ReadFile() ���� TRUE,�������.��ʱ����Ҫ���� GetOverlappedResults()
				bRead = TRUE;
			}
		} //close if(bRead)

		//�첽I/O �������ڽ���,��ʱ��Ҫ���� GetOverlappedResults ��������ѯ
		if(!bRead)
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(port->m_hComm,				//Handle to COMM port
											&port->m_ov,				//Overlapped structure
											&BytesRead,					//Stores number of bytes read
											TRUE);						//Wait Flag

			// deal with the error code 
			if (!bResult)  
			{
				port->ProcessErrorMessage("GetOverlappedResults() in ReadFile()");
			}	
		}//close if(!bRead)
		LeaveCriticalSection(&port->m_csCommunicationSync);

		// notify parent that a byte was received
		data.BytesRead = BytesRead;
		data.portnumber = port->m_nPortNr;
		::SendMessage((port->m_pOwner)->m_hWnd, WM_COMM_RXCHAR, (WPARAM)(&data), (LPARAM)port->m_nPortNr);

	}// end forever loop
}




// Write a string to the port
void CSerialPort::WriteToPort(char* string,UINT len)
{		
	//assert(m_hComm != 0);

	m_nSendLength=len;
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, string, len);

	// set event for write
	SetEvent(m_hWriteEvent);
}

// Return the device control block
DCB CSerialPort::GetDCB()
{
	return m_dcb;
}

// Return the communication event masks
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;
}

// Return the output buffer size
DWORD CSerialPort::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

//�رմ��ڣ��ͷŴ�����Դ
void CSerialPort::ClosePort()
{
	SetEvent(m_hShutdownEvent);
}
