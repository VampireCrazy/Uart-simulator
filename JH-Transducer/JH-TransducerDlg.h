
// JH-TransducerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "serialport.h"
#include "crc.h"

typedef struct _tagError_Data
	{
		unsigned int Num;  //异常数据百分比值10-10% 20-20% ...
		unsigned int locate[100]; //异常数据位置
		unsigned int record;

}Error_Data;

// CJHTransducerDlg 对话框
class CJHTransducerDlg : public CDialogEx
{
// 构造
public:
	CJHTransducerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_JHTRANSDUCER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	unsigned int ChoosePortNum();
	unsigned int ChooseBaud();
	unsigned int ChooseTransducerType();
	unsigned int ChooseMinValue();
	unsigned int ChooseMaxValue();
	unsigned int ChooseErrorNum();
	unsigned int ChooseCycle();
	unsigned int random(int min,int max);
	Error_Data s_ErrorData;
	unsigned int m_nSendCount;

	void SetSendNum();				//发送字节数
	//void CALLBACK SendProc(HWND hWnd,UINT mMsg,UINT nTimeid,DWORD dwtime);
	//unsigned int ChooseData();

	CSerialPort m_CSPort;
	CRC			m_CRC;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();
	CComboBox com_data;
	CComboBox baud_data;
	CComboBox check_data;
	CComboBox data_data;
	CComboBox stop_data;
	CComboBox type_data;
	CEdit unit_data;
	CEdit min_unit;
	CEdit max_unit;
	afx_msg void OnCbnSelchangeCombo6();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CEdit min_data;
	CEdit max_data;
	CEdit error_data;
	CEdit cycle;
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnKillfocusEdit2();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnKillfocusEdit5();
	afx_msg void OnEnKillfocusEdit4();
	afx_msg void OnBnClickedCheck1();
	CStatic m_edt_sendNum;
	afx_msg void OnBnClickedButton3();
};
