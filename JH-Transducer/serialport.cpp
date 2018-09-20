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

BOOL CSerialPort::InitPort(CWnd* pPortOwner, //the owner (CWnd) of the port （receives message）
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

	//创建事件
	//m_ov 是 OVERLAPPED 结构变量
	//以下3个 CreateEvent 函数应用均为手工复位方式

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
	m_hEventArray[0] = m_hShutdownEvent;  //最高的优先级
	m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;

	//初始化关键区代码
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

	//进入关键区

	EnterCriticalSection(&m_csCommunicationSync);

	//串口如果是打开的 关闭它
	if(m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	//为串口得到逻辑名，这样初始化函数时，只要数字表示串口号  如 1 表示 COM1
	sprintf(szPort,"COM%d",portnr);
	sprintf(szBaud,"baud=%d parity=%c data=%d stop=%d",baud, parity, databits, stopbits);

	WCHAR wszPort[256];  
	memset(wszPort,0,sizeof(wszPort));  
	MultiByteToWideChar(CP_ACP,0,szPort,strlen(szPort)+1,wszPort,  
	sizeof(wszPort)/sizeof(wszPort[0]));

	//得到要打开的串口句柄，供以后操作使用
	m_hComm = CreateFile(wszPort,								//串口号COMX
						GENERIC_READ | GENERIC_WRITE,			//可以同时读写
						0,										//以独占的方式打开串口
						NULL,									//无安全属性
						OPEN_EXISTING,							//串口设备必须用OPEN_EXISTING,因为它肯定存在
						FILE_FLAG_OVERLAPPED,					//异步I/O操作方式
						0);										//对串口设备，模板文件句柄必须为0（NULL）

	//打开串口不成功，进行错误处理
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		delete [] szPort;
		delete [] szBaud;

		return FALSE;
	}

	//设置读间隔超时变量m_CommTimeouts
	m_CommTimeouts.ReadIntervalTimeout = 100;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

	//初始化串口，设置串口参数：setCommState
	if(SetCommTimeouts(m_hComm,&m_CommTimeouts))// 设置超时
	{
		if(SetCommMask(m_hComm,dwCommEvents))  //设置通信事件
		{
			if(GetCommState(m_hComm, &m_dcb))  //获取当前DCB结构参数
			{
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;  //设置RTC bit high！
				WCHAR wszBaud[256];  
				memset(wszBaud,0,sizeof(wszBaud));  
				MultiByteToWideChar(CP_ACP,0,szBaud,strlen(szBaud)+1,wszBaud,  
				sizeof(wszBaud)/sizeof(wszBaud[0]));
				if(BuildCommDCB(wszBaud,&m_dcb))				//填写 DCB 结构
				{
					if(SetCommState(m_hComm,&m_dcb))		//利用填写好的 DCB 配置
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

	//利用 PurgeComm 函数清空 缓冲区
	if(port->m_hComm)		//检查是否打开
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	for(;;)
	{
		/*///////////////////////////////////////////////////////////////////// 
		   通信事件监视函数 WaitCommEvent() 在调用时，函数会立即返回。
	   这是因为串口句柄打开时 设置为 FILE_FLAG_OVERLAPPED 异步 I/O 操作方式。
	   
		   WaitCommEvent 函数在监视通信事件时，如果异步读操作不能立即完成，
	   则该函数返回 FALSE(0)， 并且 GetLastError 函数返回 Error_IO_PENDING，
	   表明该操作正在后台执行。

		   发生这种情况时，系统在 WaitCommEvent 函数返回之前将 OVERLAPPED 结构
	   中的成员 hEvent 参数值设置为 无信号 状态，之后等到有特定通信事件或发生
	   错误时，系统再将其设置为 有信号 状态。所以只要有字符到达,就会产生事件.
		/////////////////////////////////////////////////////////////////////*/

		bResult = WaitCommEvent(port->m_hComm,&Event,&port->m_ov);

		if(!bResult)
		{
			//如果 WaitCommEvent 返回值为NULL， 调用 GetLastError 函数查询错误信息
			switch(dwError = GetLastError())
			{
			case ERROR_IO_PENDING:
				{
					//这是正常情况，说明没有字符可读
					break;
				}
			case 87:
				{
					//87错误是系统错误，Error_INVALID_PARAMTER 并不影响接收
					break;
				}
			default:
				{
					//如果发生其他错误，则返回错误信息
					port->ProcessErrorMessage("WaitCommEvent()");
					break;
				}

			}
		}
		else
		{
			/* 如果 WaitCommEvent() 返回 TRUE, 检查 接收缓冲区 中是否有字符可读。  
			
			       注意：如果是一次就从缓冲区中读出多个字节（本类没有这样做），
			   就要用WaitForMultipleObjects 函数。

				   当第一个字符到达缓冲区时，引发 WaitForMultipleObjects 函数，当
			   函数返回时， 将异步 I/O 操作事件句柄 m_OverlappedStruct.hEvent 变量
			   复位，即设置为 无信号 状态。
			   
				   如果在复位操作后到调用 ReadFile 函数之间有其他字符达到，
			   m_OverlappedStruct.hEvent会被重新设置,即 设置为有信号状态.这时调用
			   ReadFile函数, ReadFile 函数将从缓冲区中读出所有数据，然后本循环回到
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

		//下面是主等待函数，本函数将阻塞线程，直到有相应的事件发生
		Event = WaitForMultipleObjects(3,							//事件句柄数组中的事件数
										port->m_hEventArray,		//事件数组
										FALSE,						
										INFINITE);
		switch(Event)
		{
		case 0:
			{
				//关闭事件，因为此事件是0 所以具有最高的优先级并且首先被执行
				CloseHandle(port->m_hComm);
				port->m_hComm = NULL;
				port->m_bThreadAlive = FALSE;

				//Kill this thread. brrak is not needed ,but makes me fell better.
				AfxEndThread(100);
				break;
			}
		case 1:  //读事件中，将定义的各种消息发送出去
			{
				// 在相应的消息处理函数 OnComm 中可以做出处理，这里应用GetCommMask 函数来得到
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

		case 2: //发送事件
			{
				WriteChar(port);
				break;
			}
		}//end switch
	}//close forever loop
	return 0;
}

//开始监视串口Start comm watching
BOOL CSerialPort::StartMonitoring()
{
	if (!(m_Thread = AfxBeginThread(CommThread, this)))
		return FALSE;
	TRACE("Thread started\n");
	return TRUE;	
}

// 重新启动串口线程
BOOL CSerialPort::RestartMonitoring()
{
	TRACE("Thread resumed\n");
	m_Thread->ResumeThread();
	return TRUE;	
}

// 挂起串口线程
// 当前 被挂起的线程数的增量,如果任何一个线程的挂起数大于0,那么此线程不会被执行.
// 但可以通过调用 ResumeThread 成员函数,使得此线程被恢复执行
BOOL CSerialPort::StopMonitoring()
{
	TRACE("Thread suspended\n");
	m_Thread->SuspendThread(); 
	return TRUE;	
}

//如果有错，就给出正确的信息
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

	//复位写事件句柄
	ResetEvent(port->m_hWriteEvent);
	//Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);

	if(bWrite)
	{
		// Initailize variables
		port->m_ov.Offset = 0;
		port->m_ov.OffsetHigh = 0;
	
	// 清空缓冲区
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		bResult = WriteFile(port->m_hComm,							// Handle to COMM Port
							port->m_szWriteBuffer,					// Pointer to message buffer in calling finction
							port->m_nSendLength,					// Length of message to send
							&BytesSent,								// Where to store the number of bytes sent
							&port->m_ov);							// Overlapped structure
		// 返回 FALSE，错误处理
		if (!bResult)  
		{
			DWORD dwError = GetLastError();
			switch (dwError)
			{
				case ERROR_IO_PENDING:
					{
						// 并非错误，而是异步I/O 操作正在进行，需要调用 GetOverlappedResults 查询结果
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
		/*     critical section 能保证在同一时间内只有一个线程有操
		   作某一资源的权限。下面应用了 EnterCriticalSection 函数来
		   获得串口的 critical section 这一过程可保证本程序（进程）
		   中没有其他函数或线程来使用本串口资源
		*/

		EnterCriticalSection(&port->m_csCommunicationSync);

		//使用 ClearCommError() 来更新 COMSTAT 结构， 并清除通信错误
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

		//如果所有字符被读出，就中断循环
		if(comstat.cbInQue == 0)
		{
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);

		//
		if(bRead)
		{
			//应用 ReadFile 函数独处缓冲区中的字节
			bResult = ReadFile(port->m_hComm,			//	Handle to COMM port
								RXBuff,					//  RX Buffer Pointer
								3,						//  Read one  byte
								&BytesRead,				//  Stores number of bytes read
								&port->m_ov);			//	pointer to  the  m_ov structer
			//若 ReadFile 返回 FALSE， 调用 GetLastError() 错误处理
			if(!bRead)
			{
				switch(dwError = GetLastError())
				{
					case ERROR_IO_PENDING: 	
						{ 
							// 异步I/O 操作仍在进行,这时需要调用 GetOverlappedResults 函数来查询
							bRead = FALSE;
							break;
						}
					default:
						{
							// 其他错误处理
							port->ProcessErrorMessage("ReadFile()");
							break;
						} 

				}
			}
			else
			{
				// ReadFile() 返回 TRUE,操作完成.这时不需要调用 GetOverlappedResults()
				bRead = TRUE;
			}
		} //close if(bRead)

		//异步I/O 操作仍在进行,这时需要调用 GetOverlappedResults 函数来查询
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

//关闭串口，释放串口资源
void CSerialPort::ClosePort()
{
	SetEvent(m_hShutdownEvent);
}
