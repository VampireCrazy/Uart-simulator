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

	//���ڳ�ʼ��
	BOOL		InitPort(CWnd* pPortOwner,
						UINT portnr = 1,
						UINT baud = 9600,
						char parity = 'N',
						UINT databits = 8,
						UINT stopsbits = 1,
						DWORD dwCommEvents = EV_RXCHAR | EV_CTS,
						UINT writeBufferSize = 2048);
	//�رմ���
	void		ClosePort(void);

	//��ʼ/ֹͣ ���ڼ���
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

	//�߳� 
	CWinThread*	m_Thread;

	// synchronisation objects
	CRITICAL_SECTION	m_csCommunicationSync;
	BOOL				m_bThreadAlive;

	//���
	HANDLE				m_hShutdownEvent;
	HANDLE				m_hComm;
	HANDLE				m_hWriteEvent;

	//�¼�����
	//ÿһ������Ԫ����������һ���¼���Ϊÿһ���˿� ����2��ʱ����
	//һ��д�ַ���һ�������ַ��¼��������overlapped�ṹ���У�m_ov.hEvent��
	//���˿ڱ��ر�ʱ����һ������Ĺر��¼�
	HANDLE				m_hEventArray[3];

	//�ṹ��
	OVERLAPPED			m_ov;
	COMMTIMEOUTS		m_CommTimeouts;
	DCB					m_dcb;

	//���ڵ�������
	CWnd*				m_pOwner;

	//misc
	UINT				m_nPortNr;
	char*				m_szWriteBuffer;
	DWORD				m_dwCommEvents;
	DWORD				m_nWriteBufferSize;
};

#endif __SERIALPORT_H__