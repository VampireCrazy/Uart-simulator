
// JH-TransducerDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CJHTransducerDlg �Ի���




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
	TYPE_GAS,				//��ȼ����
	TYPE_TEMPERATURE,		//�¶�
	TYPE_HUMIDITY,			//ʪ��
	TYPE_UITRAVIOLET,		//�����
}Transducer_TYPE_E;

//#define random(a,b) {srand((unsigned)time(NULL));(rand()%(b-a+1)+a);}

// CJHTransducerDlg ��Ϣ�������

BOOL CJHTransducerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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
	
	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��


	//���ø����ؼ���ʼֵ
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
		case 0://��ȼ����
			this->unit_data.SetWindowText(_T("ppm"));
			this->min_unit.SetWindowText(_T("ppm"));
			this->max_unit.SetWindowText(_T("ppm"));

			this->min_data.SetWindowText(_T("20"));
			this->max_data.SetWindowText(_T("40"));
			break;
		case 1://�¶�
			this->unit_data.SetWindowText(_T("��"));
			this->min_unit.SetWindowText(_T("��"));
			this->max_unit.SetWindowText(_T("��"));
			break;
		case 2://ʪ��

			break;
		case 3://������

			break;
		default:

			break;
	}

	ShowWindow(SW_NORMAL);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CJHTransducerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CJHTransducerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CJHTransducerDlg::OnCbnSelchangeCombo1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CJHTransducerDlg::OnCbnSelchangeCombo2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CJHTransducerDlg::OnCbnSelchangeCombo6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int transducer_type;

	transducer_type = this->type_data.GetCurSel();

	switch(transducer_type)
	{
		case 0://��ȼ����
			this->unit_data.SetWindowText(_T("ppm"));
			this->min_unit.SetWindowText(_T("ppm"));
			this->max_unit.SetWindowText(_T("ppm"));
			break;
		case 1://�¶�
			this->unit_data.SetWindowText(_T("��"));
			this->min_unit.SetWindowText(_T("��"));
			this->max_unit.SetWindowText(_T("��"));
			break;
		case 2://ʪ��

			break;
		case 3://������

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

	switch(baud_data.GetCurSel())//������
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
		MessageBox(_T("����������Ҫ����10ms"),_T("��ʾ"), MB_ICONWARNING);	
		return FALSE;
	}
	return cycle_num;
}


void CJHTransducerDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HANDLE hFile;
	CString str;
	if(Open_status == FALSE)
	{
		g_PortNum = ChoosePortNum();
		g_iBaud = ChooseBaud();
		//g_iData = ChooseData();
	
		//��ʼ��ʱ�Ĳ���Ҫע�⣡
		if (m_CSPort.InitPort(this, g_PortNum, g_iBaud, 'n', 8, 1, EV_RXCHAR, 2023))
		{
			m_CSPort.StartMonitoring();
			com_data.EnableWindow(FALSE);
			baud_data.EnableWindow(FALSE);
			check_data.EnableWindow(FALSE);
			data_data.EnableWindow(FALSE);
			stop_data.EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("�ر�"));	
			Open_status = TRUE;

		}
		else	// ���ڳ�ʼ���Ƿ�ɹ���else
		{
			MessageBox(_T("û�з��ִ˴��� �� �˴����ѱ�ռ��!"),_T("��ʾ"), MB_ICONWARNING);
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
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("��"));
		Open_status = FALSE;
	}
	

	/*hFile = CreateFile(str,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
    { 
        printf("CreateFile() error:%d", GetLastError());
        //return -1;
		//������  ����
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CButton *pButton_error,*pButton_cycle;
	int locate = 0;
	
	if(!Open_status)
	{
		MessageBox(_T("����δ��"),_T("��ʾ"), MB_ICONWARNING);
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
		GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("����"));
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
			MessageBox(_T("�������ֵ����Сֵ"),_T("��ʾ"), MB_ICONWARNING);

		}
		else
		{
			//��ʼ׼����������
			Send_status = TRUE;
			//��ʼ����ʱ��ֹ���ò���
			if(Send_status == TRUE)
			{
				type_data.EnableWindow(FALSE);
				min_data.EnableWindow(FALSE);
				max_data.EnableWindow(FALSE);
				cycle.EnableWindow(FALSE);
				error_data.EnableWindow(FALSE);		
				GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("ֹͣ����"));
			}
			if(!g_cycle_status)
			{
			
				if(g_TransducerType == TYPE_GAS) //��ȼ����
				{
					int temp1,temp2;
			
					gas_data[len++] = 0x01;		//�豸���
					gas_data[len++] = 0x03;		//��ȡ����
					gas_data[len++] = 0x02;		//���ݳ���
					if(g_error_status)			//�쳣����
					{
						temp1 = random(g_max+1,g_min+100);	//��������
						gas_data[len++] = temp1;
					}
					else
					{
						temp1 = random(g_min,g_max);  //��������
						gas_data[len++] = temp1;
					}
				

					temp2 = random(0,99);		//С������
					if(temp1 == g_max)
					{
						temp2 = 0;				//��ֹ�������������ֵ���Χ
					}
					gas_data[len++] = temp2;
					WORD CRC_t;
					CRC_t=m_CRC.CalCrcFast(gas_data,5);
					gas_data[len++] = CRC_t & 0xff;
					gas_data[len++] = CRC_t>>8 & 0xff;
				}
				else if(g_TransducerType == TYPE_TEMPERATURE) //�¶ȴ�����
				{
	
				}
				else if(g_TransducerType == TYPE_HUMIDITY) //ʪ�ȴ�����
				{
	
				}
				else if(g_TransducerType == TYPE_UITRAVIOLET) //�����ǿ��
				{
	
				}
				m_CSPort.WriteToPort(gas_data,len);
				this->m_nSendCount += len;
				SetSendNum();
				delete [] gas_data;
				//��ɷ���
				Send_status = FALSE;
				//������ɿ��������༭
				if(Send_status == FALSE)   //���η��� ���ж��е�û�У�Ϊ������
				{
					type_data.EnableWindow(TRUE);
					min_data.EnableWindow(TRUE);
					max_data.EnableWindow(TRUE);
					cycle.EnableWindow(TRUE);
					error_data.EnableWindow(TRUE);
					GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("����"));
				}
			
			}
			else//���ڷ���
			{
				if(g_TransducerType == TYPE_GAS) //��ȼ����
				{
					//�쳣���ʴ���
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
						//���������ʱ�ɹ�����ʼ����ʱ��ֹ���ò���
						type_data.EnableWindow(FALSE);
						min_data.EnableWindow(FALSE);
						max_data.EnableWindow(FALSE);
						cycle.EnableWindow(FALSE);
						error_data.EnableWindow(FALSE);
					}
					else
					{
						MessageBox(_T("���ڷ���ʧ��"),_T("��ʾ"), MB_ICONWARNING);
					}
				
				}
				else if(g_TransducerType == TYPE_TEMPERATURE) //�¶ȴ�����
				{
					SetTimer(TYPE_TEMPERATURE,g_cycle,NULL);
				}
				else if(g_TransducerType == TYPE_HUMIDITY) //ʪ�ȴ�����
				{
					SetTimer(TYPE_HUMIDITY,g_cycle,NULL);
				}
				else if(g_TransducerType == TYPE_UITRAVIOLET) //�����ǿ��
				{
					SetTimer(TYPE_UITRAVIOLET,g_cycle,NULL);
				}
			}
		}
	}// end  Send_status
	
}




void CJHTransducerDlg::OnEnChangeEdit4()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CJHTransducerDlg::OnEnChangeEdit5()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CJHTransducerDlg::OnBnClickedCheck2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BOOL cycle_status,error_status;

	CButton *pButton_cycle = (CButton *)GetDlgItem(IDC_CHECK2);
	CButton *pButton_error = (CButton *)GetDlgItem(IDC_CHECK1);
	cycle_status = pButton_cycle->GetCheck();
	error_status = pButton_error->GetCheck();
	if(cycle_status)//���ѡ��ѭ������
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
	m_edt_sendNum.SetWindowText(_T("���ͣ�") + strSendNum);
}


void CJHTransducerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	char * gas_data = new char[256];
	memset(gas_data,0,256);
	int len = 0;
	BOOL send_error_flag=FALSE;

				
	switch(nIDEvent)
	{
		case TYPE_GAS:
				int temp1,temp2;		
				

				gas_data[len++] = 0x01;		//�豸���
				gas_data[len++] = 0x03;		//��ȡ����
				gas_data[len++] = 0x02;		//���ݳ���

				if(g_error_status)			//�쳣����
				{
					for(int i=0;i<g_error;i++)
					{
						if(s_ErrorData.locate[i] == (s_ErrorData.record%100))
							send_error_flag = TRUE;
					}

					if(send_error_flag == TRUE)
					{
						temp1 = random(g_max+1,g_min+100);	//��������
						gas_data[len++] = temp1;
						send_error_flag == FALSE;
					}
					
				}
				else
				{
					temp1 = random(g_min,g_max);  //��������
					gas_data[len++] = temp1;
				}
				

				temp2 = random(0,99);		//С������
				if(temp1 == g_max)
				{
					temp2 = 0;				//��ֹ�������������ֵ���Χ
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	


}


void CJHTransducerDlg::OnEnChangeEdit2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CJHTransducerDlg::OnEnKillfocusEdit5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int cycle_data;
	CString str;
	cycle.GetWindowTextW(str);
	cycle_data = _wtoi(str);
	if(cycle_data<10)
	{
		MessageBox(_T("����������Ҫ����10ms"),_T("��ʾ"), MB_ICONWARNING);	
	}
}


void CJHTransducerDlg::OnEnKillfocusEdit4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int errordata;
	CString str;
	error_data.GetWindowTextW(str);
	errordata = _wtoi(str);
	if(errordata<0 || errordata>100)
	{
		MessageBox(_T("�쳣���ʽ���0%-100%֮��"),_T("��ʾ"), MB_ICONWARNING);	
	}
}

void CJHTransducerDlg::OnBnClickedCheck1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	this->m_nSendCount = 0;
	SetSendNum();
}
