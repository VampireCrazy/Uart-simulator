/*************************************************
Copyright (C), 2010-2028, Jinhe Tech. Co., Ltd.

File name:       SerialPort.h

Date:    		 Last Modified 2018-08-21

*************************************************/

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include "stdafx.h"

#define WM_COMM_BREAK_DETECTED		WM_USER+201	// A break was detected on input.

#define WM_COMM_CTS_DETECTED		WM_USER+202	// The CTS (clear-to-send) signal changed state. 

#define WM_COMM_DSR_DETECTED		WM_USER+203	// The DSR (data-set-ready) signal changed state. 

#define WM_COMM_ERR_DETECTED		WM_USER+204	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 

#define WM_COMM_RING_DETECTED		WM_USER+205	// A ring indicator was detected. 

#define WM_COMM_RLSD_DETECTED		WM_USER+206	// The RLSD (receive-line-signal-detect) signal changed state. 

#define WM_COMM_RXCHAR				WM_USER+207	// A character was received and placed in the input buffer. 

#define WM_COMM_RXFLAG_DETECTED		WM_USER+208	// The event character was received and placed in the input buffer.  

#define WM_COMM_TXEMPTY_DETECTED	WM_USER+209	// The last character in the output buffer was sent.  




typedef struct _tagRECEIVEDATA
{
	int BytesRead;
	int portnumber;
	unsigned char RXBuff[16];
}RECEIVEDATA;

class CSerialPort
{	
public:
	//contruction and destrction
	CSerialPort(void);
	virtual		~CSerialPort(void);

	//串口初始化
	BOOL		InitPort(CWnd* pPortOwner,
						UINT portnr = 1,
						UINT baud = 9600,
						char parity = 'N',
						UINT databits = 8,
						UINT stopsbits = 1,
						DWORD dwCommEvents = EV_RXCHAR | EV_CTS,
						UINT writeBufferSize = 2048);
	//关闭串口
	void		ClosePort(void);

	//开始/停止 串口监视
	BOOL		StartMonitoring(void);
	BOOL		RestartMonitoring(void);
	BOOL		StopMonitoring(void);

	DWORD		GetWriteBufferSize(void);
	DWORD		GetCommEvents(void);
	DCB			GetDCB(void);

	void		WriteToPort(char* string,UINT len);

protected:
	//protected member functions
	void	ProcessErrorMessage(char* ErrorText);
	static UINT CommThread(LPVOID pParam);
	static void ReceiveChar(CSerialPort* port,COMSTAT comstat);
	static void WriteChar(CSerialPort* port);

	UINT		m_nSendLength;

	//线程 
	CWinThread*	m_Thread;

	// synchronisation objects
	CRITICAL_SECTION	m_csCommunicationSync;
	BOOL				m_bThreadAlive;

	//句柄
	HANDLE				m_hShutdownEvent;
	HANDLE				m_hComm;
	HANDLE				m_hWriteEvent;

	//事件数组
	//每一个数组元素用来代表一个事件，为每一个端口 分配2个时间句柄
	//一个写字符和一个接受字符事件被分配的overlapped结构体中（m_ov.hEvent）
	//当端口被关闭时就有一个常规的关闭事件
	HANDLE				m_hEventArray[3];

	//结构体
	OVERLAPPED			m_ov;
	COMMTIMEOUTS		m_CommTimeouts;
	DCB					m_dcb;

	//窗口的所属者
	CWnd*				m_pOwner;

	//misc
	UINT				m_nPortNr;
	char*				m_szWriteBuffer;
	DWORD				m_dwCommEvents;
	DWORD				m_nWriteBufferSize;
};

#endif __SERIALPORT_H__