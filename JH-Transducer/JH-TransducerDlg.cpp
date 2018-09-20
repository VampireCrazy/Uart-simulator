
// JH-TransducerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "JH-Transducer.h"
#include "JH-TransducerDlg.h"
#include "afxdialogex.h"

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CJHTransducerDlg 对话框




CJHTransducerDlg::CJHTransducerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CJHTransducerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDB_BITMAP1);
}

void CJHTransducerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, com_data);
	DDX_Control(pDX, IDC_COMBO2, baud_data);
	DDX_Control(pDX, IDC_COMBO3, check_data);
	DDX_Control(pDX, IDC_COMBO4, data_data);
	DDX_Control(pDX, IDC_COMBO5, stop_data);
	DDX_Control(pDX, IDC_COMBO6, type_data);
	DDX_Control(pDX, IDC_EDIT1, unit_data);
	DDX_Control(pDX, IDC_EDIT6, min_unit);
	DDX_Control(pDX, IDC_EDIT7, max_unit);
	DDX_Control(pDX, IDC_EDIT2, min_data);
	DDX_Control(pDX, IDC_EDIT3, max_data);
	DDX_Control(pDX, IDC_EDIT4, error_data);
	DDX_Control(pDX, IDC_EDIT5, cycle);
	DDX_Control(pDX, IDC_STATIC20, m_edt_sendNum);
}

BEGIN_MESSAGE_MAP(CJHTransducerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CJHTransducerDlg::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CJHTransducerDlg::OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO6, &CJHTransducerDlg::OnCbnSelchangeCombo6)
	ON_BN_CLICKED(IDC_BUTTON1, &CJHTransducerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CJHTransducerDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT4, &CJHTransducerDlg::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &CJHTransducerDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_CHECK2, &CJHTransducerDlg::OnBnClickedCheck2)
	ON_WM_TIMER()
	ON_EN_KILLFOCUS(IDC_EDIT2, &CJHTransducerDlg::OnEnKillfocusEdit2)
	ON_EN_CHANGE(IDC_EDIT2, &CJHTransducerDlg::OnEnChangeEdit2)
	ON_EN_KILLFOCUS(IDC_EDIT5, &CJHTransducerDlg::OnEnKillfocusEdit5)
	ON_EN_KILLFOCUS(IDC_EDIT4, &CJHTransducerDlg::OnEnKillfocusEdit4)
	ON_BN_CLICKED(IDC_CHECK1, &CJHTransducerDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON3, &CJHTransducerDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


//
UINT g_PortNum = 0;
UINT g_iBaud = 0;
UINT g_iData = 0;

UINT g_TransducerType = 0;
UINT g_min = 0;
UINT g_max = 0;
UINT g_error = 0;
UINT g_cycle = 0;
UINT g_timer;
BOOL g_error_status,g_cycle_status;
BOOL Open_status = FALSE;
BOOL Send_status = FALSE;



typedef enum _tagTransducer_TYPE
{
	TYPE_GAS,				//可燃气体
	TYPE_TEMPERATURE,		//温度
	TYPE_HUMIDITY,			//湿度
	TYPE_UITRAVIOLET,		//紫外光
}Transducer_TYPE_E;

//#define random(a,b) {srand((unsigned)time(NULL));(rand()%(b-a+1)+a);}

// CJHTransducerDlg 消息处理程序

BOOL CJHTransducerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	//设置各个控件初始值
	this->com_data.SetCurSel(0);
	this->baud_data.SetCurSel(0);
	this->check_data.SetCurSel(0);
	this->data_data.SetCurSel(3);
	this->stop_data.SetCurSel(0);
	this->type_data.SetCurSel(0);
	this->error_data.EnableWindow(FALSE);
	this->cycle.EnableWindow(FALSE);
	this->m_nSendCount = 0;
	this->error_data.SetWindowText(_T("0"));
	this->cycle.SetWindowTextW(_T("1000"));
	memset(&this->s_ErrorData,0,sizeof(this->s_ErrorData));




	srand((unsigned)time(NULL));
	
	int transducer_type;

	transducer_type = this->type_data.GetCurSel();

	switch(transducer_type)
	{
		case 0://可燃气体
			this->unit_data.SetWindowText(_T("ppm"));
			this->min_unit.SetWindowText(_T("ppm"));
			this->max_unit.SetWindowText(_T("ppm"));

			this->min_data.SetWindowText(_T("20"));
			this->max_data.SetWindowText(_T("40"));
			break;
		case 1://温度
			this->unit_data.SetWindowText(_T("℃"));
			this->min_unit.SetWindowText(_T("℃"));
			this->max_unit.SetWindowText(_T("℃"));
			break;
		case 2://湿度

			break;
		case 3://紫外线

			break;
		default:

			break;
	}

	ShowWindow(SW_NORMAL);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
unsigned int CJHTransducerDlg::random(int min, int max)
{
	unsigned int result;
	result = (rand()%(max-min+1)+min);
	
	return result;
}
void CJHTransducerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CJHTransducerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CJHTransducerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CJHTransducerDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CJHTransducerDlg::OnCbnSelchangeCombo2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CJHTransducerDlg::OnCbnSelchangeCombo6()
{
	// TODO: 在此添加控件通知处理程序代码
	int transducer_type;

	transducer_type = this->type_data.GetCurSel();

	switch(transducer_type)
	{
		case 0://可燃气体
			this->unit_data.SetWindowText(_T("ppm"));
			this->min_unit.SetWindowText(_T("ppm"));
			this->max_unit.SetWindowText(_T("ppm"));
			break;
		case 1://温度
			this->unit_data.SetWindowText(_T("℃"));
			this->min_unit.SetWindowText(_T("℃"));
			this->max_unit.SetWindowText(_T("℃"));
			break;
		case 2://湿度

			break;
		case 3://紫外线

			break;
		default:

			break;
	}
}

unsigned int CJHTransducerDlg::ChoosePortNum()
{
	unsigned int PortNum = 0;
	
	switch(com_data.GetCurSel())
	{
		case 0:
			PortNum = 1;
			break;
		case 1:
			PortNum = 2;
			break;
		case 2:
			PortNum = 3;
			break;
		case 3:
			PortNum = 4;
			break;
		case 4:
			PortNum = 5;
			break;
		case 5:
			PortNum = 6;
			break;
		case 6:
			PortNum = 7;
			break;
		case 7:
			PortNum = 8;
			break;
		case 8:
			PortNum = 9;
			break;
		case 9:
			PortNum = 10;
		default:
			break;
	}
		return PortNum;
}



unsigned int CJHTransducerDlg::ChooseBaud()
{
	unsigned int Baud = 0;

	switch(baud_data.GetCurSel())//波特率
	{
	case 0: 
		Baud = 9600;   
		break;
	case 1: 
		Baud = 14400;  
		break;
	case 2: 
		Baud = 19200;  
		break;
	case 3: 
		Baud = 38400;  
		break;
	case 4: 
		Baud = 56000;  
		break;
	case 5: 
		Baud = 57600;  
		break;
	case 6: 
		Baud = 115200; 
		break;
	case 7: 
		Baud = 128000; 
		break;
	
	default:
		break;
	}
	return Baud;
}

unsigned int CJHTransducerDlg::ChooseTransducerType()
{
	unsigned int TransducerType = 0;
	
	switch(type_data.GetCurSel())
	{
		case 0:
			TransducerType = TYPE_GAS;
			break;
		case 1:
			TransducerType = TYPE_TEMPERATURE;
			break;
		case 2:
			TransducerType = TYPE_HUMIDITY;
			break;
		case 3:
			TransducerType = TYPE_UITRAVIOLET;
			break;
		default:
			break;
	}
		return TransducerType;
}

unsigned int CJHTransducerDlg::ChooseMinValue()
{
	unsigned int min = 0;
	CString str;
	min_data.GetWindowTextW(str);
	min = _wtoi(str);
	return min;
}

unsigned int CJHTransducerDlg::ChooseMaxValue()
{
	unsigned int max = 0;
	CString str;
	max_data.GetWindowTextW(str);
	max = _wtoi(str);
	return max;
}

unsigned int CJHTransducerDlg::ChooseErrorNum()
{
	unsigned int error_num = 0;
	CString str;
	error_data.GetWindowTextW(str);
	error_num = _wtoi(str);
	return error_num;
}


unsigned int CJHTransducerDlg::ChooseCycle()
{
	unsigned int cycle_num = 0;
	CString str;
	cycle.GetWindowTextW(str);
	cycle_num = _wtoi(str);
	if(cycle_num<10)
	{
		MessageBox(_T("发送周期需要大于10ms"),_T("提示"), MB_ICONWARNING);	
		return FALSE;
	}
	return cycle_num;
}


void CJHTransducerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hFile;
	CString str;
	if(Open_status == FALSE)
	{
		g_PortNum = ChoosePortNum();
		g_iBaud = ChooseBaud();
		//g_iData = ChooseData();
	
		//初始化时的参数要注意！
		if (m_CSPort.InitPort(this, g_PortNum, g_iBaud, 'n', 8, 1, EV_RXCHAR, 2023))
		{
			m_CSPort.StartMonitoring();
			com_data.EnableWindow(FALSE);
			baud_data.EnableWindow(FALSE);
			check_data.EnableWindow(FALSE);
			data_data.EnableWindow(FALSE);
			stop_data.EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("关闭"));	
			Open_status = TRUE;

		}
		else	// 串口初始化是否成功的else
		{
			MessageBox(_T("没有发现此串口 或 此串口已被占用!"),_T("提示"), MB_ICONWARNING);
		}
	}
	else
	{
		m_CSPort.ClosePort();
		com_data.EnableWindow(TRUE);
		baud_data.EnableWindow(TRUE);
		check_data.EnableWindow(TRUE);
		data_data.EnableWindow(TRUE);
		stop_data.EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("打开"));
		Open_status = FALSE;
	}
	

	/*hFile = CreateFile(str,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
    { 
        printf("CreateFile() error:%d", GetLastError());
        //return -1;
		//错误处理  跳出
    }
	BOOL bErrorFlag = FALSE;
    char DataBuffer[] = "This is some test data to write to the file.";
    DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
    DWORD dwBytesWritten = 0;
	bErrorFlag = WriteFile( 
        hFile,           // open file handle
        DataBuffer,      // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure
    if (FALSE == bErrorFlag)
    {
        printf("Terminal failure: Unable to write to file.\n");
    }
	else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            printf("Error: dwBytesWritten != dwBytesToWrite\n");
        }
        else
        {
            printf("Wrote %d bytes to test.txt successfully.\n", dwBytesWritten);
        }
    }

    CloseHandle(hFile);*/


}



void CJHTransducerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton *pButton_error,*pButton_cycle;
	int locate = 0;
	
	if(!Open_status)
	{
		MessageBox(_T("串口未打开"),_T("提示"), MB_ICONWARNING);
		return;
	}

	if(Send_status == TRUE)
	{
		KillTimer(TYPE_GAS);
		Send_status = FALSE;
		type_data.EnableWindow(TRUE);
		min_data.EnableWindow(TRUE);
		max_data.EnableWindow(TRUE);
		cycle.EnableWindow(TRUE);
		error_data.EnableWindow(TRUE);
		memset(&this->s_ErrorData,0,sizeof(this->s_ErrorData));
		GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("发送"));
		Send_status = FALSE;
	}
	else if(Send_status == FALSE)
	{
		g_TransducerType = ChooseTransducerType();
		g_min = ChooseMinValue();
		g_max = ChooseMaxValue();

		pButton_cycle = (CButton *)GetDlgItem(IDC_CHECK2);
		pButton_error = (CButton *)GetDlgItem(IDC_CHECK1);
		g_error_status = pButton_error->GetCheck();
		g_cycle_status = pButton_cycle->GetCheck();
		if(g_error_status)
		{
			if(g_cycle_status)
			{
				g_error = ChooseErrorNum();
				g_cycle = ChooseCycle();
				
			}
		
		}
		else if(g_cycle_status)
		{	
			g_cycle = ChooseCycle();

		}

		int len = 0;
		char * gas_data = new char[256];
		memset(gas_data,0,256);
		if(g_min>g_max)
		{
			MessageBox(_T("请检查最大值和最小值"),_T("提示"), MB_ICONWARNING);

		}
		else
		{
			//开始准备发送数据
			Send_status = TRUE;
			//开始发送时禁止设置参数
			if(Send_status == TRUE)
			{
				type_data.EnableWindow(FALSE);
				min_data.EnableWindow(FALSE);
				max_data.EnableWindow(FALSE);
				cycle.EnableWindow(FALSE);
				error_data.EnableWindow(FALSE);		
				GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("停止发送"));
			}
			if(!g_cycle_status)
			{
			
				if(g_TransducerType == TYPE_GAS) //可燃气体
				{
					int temp1,temp2;
			
					gas_data[len++] = 0x01;		//设备编号
					gas_data[len++] = 0x03;		//读取数据
					gas_data[len++] = 0x02;		//数据长度
					if(g_error_status)			//异常数据
					{
						temp1 = random(g_max+1,g_min+100);	//整数部分
						gas_data[len++] = temp1;
					}
					else
					{
						temp1 = random(g_min,g_max);  //整数部分
						gas_data[len++] = temp1;
					}
				

					temp2 = random(0,99);		//小数部分
					if(temp1 == g_max)
					{
						temp2 = 0;				//防止随机数超过正常值最大范围
					}
					gas_data[len++] = temp2;
					WORD CRC_t;
					CRC_t=m_CRC.CalCrcFast(gas_data,5);
					gas_data[len++] = CRC_t & 0xff;
					gas_data[len++] = CRC_t>>8 & 0xff;
				}
				else if(g_TransducerType == TYPE_TEMPERATURE) //温度传感器
				{
	
				}
				else if(g_TransducerType == TYPE_HUMIDITY) //湿度传感器
				{
	
				}
				else if(g_TransducerType == TYPE_UITRAVIOLET) //紫外光强度
				{
	
				}
				m_CSPort.WriteToPort(gas_data,len);
				this->m_nSendCount += len;
				SetSendNum();
				delete [] gas_data;
				//完成发送
				Send_status = FALSE;
				//发送完成开启参数编辑
				if(Send_status == FALSE)   //单次发送 做判断有点没有，为了明了
				{
					type_data.EnableWindow(TRUE);
					min_data.EnableWindow(TRUE);
					max_data.EnableWindow(TRUE);
					cycle.EnableWindow(TRUE);
					error_data.EnableWindow(TRUE);
					GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("发送"));
				}
			
			}
			else//周期发送
			{
				if(g_TransducerType == TYPE_GAS) //可燃气体
				{
					//异常概率处理
					if(g_error != 0)
					{
						std::vector<int> temp;
						for (int i = 0; i < 100; ++i)
						{
							temp.push_back(i + 1);
						}
						std::random_shuffle(temp.begin(), temp.end());

						for(int i = 0;i<g_error;i++)
						{
							s_ErrorData.locate[i] = temp[i];
						}
					}


					g_timer = SetTimer(TYPE_GAS,g_cycle,NULL);
					if(g_timer)
					{
						Send_status = TRUE;
						//如果开启定时成功，开始发送时禁止设置参数
						type_data.EnableWindow(FALSE);
						min_data.EnableWindow(FALSE);
						max_data.EnableWindow(FALSE);
						cycle.EnableWindow(FALSE);
						error_data.EnableWindow(FALSE);
					}
					else
					{
						MessageBox(_T("周期发送失败"),_T("提示"), MB_ICONWARNING);
					}
				
				}
				else if(g_TransducerType == TYPE_TEMPERATURE) //温度传感器
				{
					SetTimer(TYPE_TEMPERATURE,g_cycle,NULL);
				}
				else if(g_TransducerType == TYPE_HUMIDITY) //湿度传感器
				{
					SetTimer(TYPE_HUMIDITY,g_cycle,NULL);
				}
				else if(g_TransducerType == TYPE_UITRAVIOLET) //紫外光强度
				{
					SetTimer(TYPE_UITRAVIOLET,g_cycle,NULL);
				}
			}
		}
	}// end  Send_status
	
}




void CJHTransducerDlg::OnEnChangeEdit4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CJHTransducerDlg::OnEnChangeEdit5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CJHTransducerDlg::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL cycle_status,error_status;

	CButton *pButton_cycle = (CButton *)GetDlgItem(IDC_CHECK2);
	CButton *pButton_error = (CButton *)GetDlgItem(IDC_CHECK1);
	cycle_status = pButton_cycle->GetCheck();
	error_status = pButton_error->GetCheck();
	if(cycle_status)//如果选中循环发送
	{
		if(error_status)
			this->error_data.EnableWindow(TRUE);
		this->cycle.EnableWindow(TRUE);
		this->cycle.SetWindowTextW(_T("1000"));
		this->error_data.SetWindowTextW(_T("0"));
	}
	else
	{
		this->error_data.EnableWindow(FALSE);
		this->cycle.EnableWindow(FALSE);
	}
}


void CJHTransducerDlg::SetSendNum()
{
	CString strSendNum;
	strSendNum.Format(_T("%d"), m_nSendCount);
	m_edt_sendNum.SetWindowText(_T("发送：") + strSendNum);
}


void CJHTransducerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char * gas_data = new char[256];
	memset(gas_data,0,256);
	int len = 0;
	BOOL send_error_flag=FALSE;

				
	switch(nIDEvent)
	{
		case TYPE_GAS:
				int temp1,temp2;		
				

				gas_data[len++] = 0x01;		//设备编号
				gas_data[len++] = 0x03;		//读取数据
				gas_data[len++] = 0x02;		//数据长度

				if(g_error_status)			//异常数据
				{
					for(int i=0;i<g_error;i++)
					{
						if(s_ErrorData.locate[i] == (s_ErrorData.record%100))
							send_error_flag = TRUE;
					}

					if(send_error_flag == TRUE)
					{
						temp1 = random(g_max+1,g_min+100);	//整数部分
						gas_data[len++] = temp1;
						send_error_flag == FALSE;
					}
					
				}
				else
				{
					temp1 = random(g_min,g_max);  //整数部分
					gas_data[len++] = temp1;
				}
				

				temp2 = random(0,99);		//小数部分
				if(temp1 == g_max)
				{
					temp2 = 0;				//防止随机数超过正常值最大范围
				}
				gas_data[len++] = temp2;
				WORD CRC_t;
				CRC_t=m_CRC.CalCrcFast(gas_data,5);
				gas_data[len++] = CRC_t & 0xff;
				gas_data[len++] = CRC_t>>8 & 0xff;
				m_CSPort.WriteToPort(gas_data,len);
				this->m_nSendCount += len;
				SetSendNum();
				s_ErrorData.record++;
			break;
		case TYPE_TEMPERATURE:

			break;
		case TYPE_HUMIDITY:

			break;
		case TYPE_UITRAVIOLET:

			break;
	}
	delete [] gas_data;	
	//CDialogEx::OnTimer(nIDEvent);
}


void CJHTransducerDlg::OnEnKillfocusEdit2()
{
	// TODO: 在此添加控件通知处理程序代码
	


}


void CJHTransducerDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CJHTransducerDlg::OnEnKillfocusEdit5()
{
	// TODO: 在此添加控件通知处理程序代码
	int cycle_data;
	CString str;
	cycle.GetWindowTextW(str);
	cycle_data = _wtoi(str);
	if(cycle_data<10)
	{
		MessageBox(_T("发送周期需要大于10ms"),_T("提示"), MB_ICONWARNING);	
	}
}


void CJHTransducerDlg::OnEnKillfocusEdit4()
{
	// TODO: 在此添加控件通知处理程序代码
	int errordata;
	CString str;
	error_data.GetWindowTextW(str);
	errordata = _wtoi(str);
	if(errordata<0 || errordata>100)
	{
		MessageBox(_T("异常概率介于0%-100%之间"),_T("提示"), MB_ICONWARNING);	
	}
}

void CJHTransducerDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL cycle_status,error_status;

	CButton *pButton_cycle = (CButton *)GetDlgItem(IDC_CHECK2);
	cycle_status = pButton_cycle->GetCheck();

	if(cycle_status)
	{
		error_data.EnableWindow(TRUE);
	}
	CButton *pButton_error = (CButton *)GetDlgItem(IDC_CHECK1);
	error_status = pButton_error->GetCheck();
	if(!error_status)
	{
		error_data.SetWindowText(_T("0"));
		error_data.EnableWindow(FALSE);
	}

}


void CJHTransducerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	this->m_nSendCount = 0;
	SetSendNum();
}
