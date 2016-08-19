
// RTSPClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "RTSPClient.h"
#include "RTSPClientDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
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

// CRTSPClientDlg dialog

CRTSPClientDlg::CRTSPClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRTSPClientDlg::IDD, pParent)
	, m_editURL(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRTSPClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_editURL, m_editURL);
	DDX_Control(pDX, IDC_VIEWFRAME, m_picture_static);
	DDX_Control(pDX, btnOpen, m_bitmap_open);
	DDX_Control(pDX, IDC_btnCatchPic, m_bitmap_catch);
	DDX_Control(pDX, IDC_BUTTON_EXIT, m_bitmap_exit);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_bitmap_play);
}

BEGIN_MESSAGE_MAP(CRTSPClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(btnOpen, &CRTSPClientDlg::OnBnClickedbtnopen)
	ON_BN_CLICKED(IDC_btnCatchPic, &CRTSPClientDlg::OnBnClickedbtncatchpic)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CRTSPClientDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CRTSPClientDlg::OnBnClickedButtonPause)
	ON_WM_TIMER()
	ON_WM_CLOSE()
//	ON_EN_CHANGE(IDC_editURL, &CRTSPClientDlg::OnEnChangeediturl)
END_MESSAGE_MAP()


// CRTSPClientDlg message handlers
int CRTSPClientDlg::InitProgram()
{
	//////////////////////////////////////////////////////////////////////////
	//��ȡ�ؼ��ڶԻ����ϵ�����
	m_IsAccess_control_position = Access_control_position();

	//��ʼ��ffmepg
	ffmpegProcess.ffmpeg_init();

	//////////////////////////////////////////////////////////////////////////
	//��ȡȫ������Ļ���  ����ֻ�ܵ���һ�� �ڶ��ε���ʱ ��С���ı�
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_client_exist = 0;
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;

	//��ʼ��Ƶ��С
	m_picture_static.GetWindowRect(&m_Picture_rect); 
	m_screen_width = m_Picture_rect.right - m_Picture_rect.left;
	m_screen_height = m_Picture_rect.bottom - m_Picture_rect.top;
	//�����ǻ�ȡ�� STATIC�ؼ��Ĵ�С 
	//�������ж�����Ϊ ����ı��Сʱ �� �� �͸� ���� ������ͼ��᲻��ȷ �� �ж� Ҫż��
	if (m_screen_width % 2 != 0)
	{
		m_screen_width -= 1;
	}
	if (m_screen_height % 2 != 0)
	{
		m_screen_height -= 1;
	}

	return 1;
}

int CRTSPClientDlg::UinitProgram()
{

	//////////////////////////////////////////////////////////////////////////;
	//�ر�ffmpeg
	ffmpegProcess.ffmpeg_exit();

	//�ر�RTSP�ͻ���
	RTSPSession.RTSPClient_exit();
	//////////////////////////////////////////////////////////////////////////
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_client_exist = 0;
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;
	
	return 1;
}



BOOL CRTSPClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	UpdateData(TRUE);

	//////////////////////////////////////////////
	//���ڲ����Ըı��С
	ModifyStyle(WS_THICKFRAME, NULL);


	//////////////////////////////////////////////////////////////////////////
	//�ر�ץȡͼƬ��ť
	GetDlgItem(IDC_btnCatchPic)->EnableWindow(FALSE);
	//�ر���ͣͼƬ��ť
	GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(FALSE);

	//////////////////////////////////////////////
	//��ťͼ������
	hIcon_play = AfxGetApp()->LoadIcon(IDI_ICON_PLAY); 
	hIcon_catch = AfxGetApp()->LoadIcon(IDI_ICON_CATCH); 
	hIcon_open = AfxGetApp()->LoadIcon(IDI_ICON_OPEN); 
	hIcon_exit = AfxGetApp()->LoadIcon(IDI_ICON_EXIT);
	hIcon_pause = AfxGetApp()->LoadIcon(IDI_ICON_PAUSE); 

	hIcon_open_b = AfxGetApp()->LoadIcon(IDI_ICON_OPEN_B); 
	hIcon_play_b = AfxGetApp()->LoadIcon(IDI_ICON_PAUSE_B);
	hIcon_catch_b = AfxGetApp()->LoadIcon(IDI_ICON_CATCH_B); 

	m_bitmap_open.SetIcon(hIcon_open); 
	m_bitmap_exit.SetIcon(hIcon_exit); 
	m_bitmap_catch.SetIcon(hIcon_catch_b);
	m_bitmap_play.SetIcon(hIcon_pause); 

	//��ʼ���༭��
	//rtsp://192.168.23.105:8554/test
	//rtsp://admin:12345@192.168.23.98
	m_editURL = "rtsp://admin:12345@192.168.23.98";

	//��ʼ��ȫ�ֱ���
	InitProgram();

	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRTSPClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRTSPClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRTSPClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRTSPClientDlg::OnBnClickedbtnopen()
{
	// TODO: Add your control notification handler code here 
	//������URLʱ�����ͷ���һ��URL��Դ
	if (!ffmpegProcess.m_pause_play)
	{
		UinitProgram();
	}

	//��ʼ��������
	InitProgram();
	Sleep(500);

	//�����ػ�
	Invalidate(TRUE);

	//�ر�ץȡͼƬ��ť
	GetDlgItem(IDC_btnCatchPic)->EnableWindow(TRUE);
	//�رմ�ͼƬ��ť
	GetDlgItem(btnOpen)->EnableWindow(FALSE);
	//�ر���ͣͼƬ��ť
	GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(TRUE);
	//�༭��ر�
	GetDlgItem(IDC_editURL)->EnableWindow(FALSE);

	//////////////////////////////////////////////
	//��ťͼ������

	m_bitmap_open.SetIcon(hIcon_open); 

	m_bitmap_play.SetIcon(hIcon_pause); 

	m_bitmap_catch.SetIcon(hIcon_catch);

	//��ȡ��ʾ������
	HWND Wnd = this->GetDlgItem(IDC_VIEWFRAME)->GetSafeHwnd();
	//ffmpeg��ʼ��
	ffmpegProcess.ffmpeg_initDecoder(Wnd);

	//��ȡ�༭���еĵ�ַ
	GetDlgItem(IDC_editURL)->GetWindowText(szUrl);

	//��URL����
	if(RTSPSession.startRTSPClient("test_client", szUrl))
	{
		Sleep(500);
		int startPlay_Flag = ffmpegProcess.startPlay_tidFun();
		if (!startPlay_Flag)
		{
			MessageBox(_T("������ȡ�߳�ʧ�� ������´���"), NULL, MB_OK );

		}
	}

	//����ˢ��MFC����
	Invalidate(TRUE);

	SetTimer(1,100,0);
}


void CRTSPClientDlg::OnBnClickedbtncatchpic()
{
	// TODO: Add your control notification handler code here

	ffmpegProcess.catch_frameFinished = 1;                     
}
int CRTSPClientDlg::Access_control_position()
{
	//�򿪰�ť��ʼ����λ��
	GetDlgItem(btnOpen)->GetWindowRect(&m_rect_BUTTON_OPEN);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_BUTTON_OPEN);//ת��Ϊ�Ի����ϵ����λ��
	m_width_BUTTON_OPEN = m_rect_BUTTON_OPEN.right - m_rect_BUTTON_OPEN.left;  //widthΪbutton�Ŀ�
	m_height__BUTTON_OPEN = m_rect_BUTTON_OPEN.bottom - m_rect_BUTTON_OPEN.top; //heightΪbutton�ĸ�

	//ץȡͼƬ��ť��ʼ����λ��
	GetDlgItem(IDC_btnCatchPic)->GetWindowRect(&m_rect_BUTTON_CATCH_PIC);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_BUTTON_CATCH_PIC);//ת��Ϊ�Ի����ϵ����λ��
	m_width_BUTTON_CATCH_PIC = m_rect_BUTTON_CATCH_PIC.right - m_rect_BUTTON_CATCH_PIC.left;  //widthΪbutton�Ŀ�
	m_height__BUTTON_CATCH_PIC = m_rect_BUTTON_CATCH_PIC.bottom - m_rect_BUTTON_CATCH_PIC.top; //heightΪbutton�ĸ�

	//�༭���ʼ����λ��
	GetDlgItem(IDC_editURL)->GetWindowRect(&m_rect_EDIT_URL);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_EDIT_URL);//ת��Ϊ�Ի����ϵ����λ��
	m_width_EDIT_URL = m_rect_EDIT_URL.right - m_rect_EDIT_URL.left;  //widthΪbutton�Ŀ�
	m_height__EDIT_URL = m_rect_EDIT_URL.bottom - m_rect_EDIT_URL.top; //heightΪbutton�ĸ�


	//��̬�ı����ʼ����λ��
	GetDlgItem(IDC_STATIC)->GetWindowRect(&m_rect_STATIC_URL);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_STATIC_URL);//ת��Ϊ�Ի����ϵ����λ��
	m_width_STATIC_URL = m_rect_STATIC_URL.right - m_rect_STATIC_URL.left;  //widthΪbutton�Ŀ�
	m_height__STATIC_URL = m_rect_STATIC_URL.bottom - m_rect_STATIC_URL.top; //heightΪbutton�ĸ�

	//���������ʼ����λ��
	GetDlgItem(IDC_VIEWFRAME)->GetWindowRect(&m_rect_PICURE_CONTROL);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_PICURE_CONTROL);//ת��Ϊ�Ի����ϵ����λ��
	m_width_PICURE_CONTROL = m_rect_PICURE_CONTROL.right - m_rect_PICURE_CONTROL.left;  //widthΪbutton�Ŀ�
	m_height_PICURE_CONTROL = m_rect_PICURE_CONTROL.bottom - m_rect_PICURE_CONTROL.top; //heightΪbutton�ĸ�

	//�˳���ť��ʼ����λ��
	GetDlgItem(IDC_BUTTON_EXIT)->GetWindowRect(&m_rect_BUTTON_CANCEL);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(m_rect_BUTTON_CANCEL);//ת��Ϊ�Ի����ϵ����λ��
	m_width_BUTTON_CANCEL = m_rect_BUTTON_CANCEL.right - m_rect_BUTTON_CANCEL.left;  //widthΪbutton�Ŀ�
	m_height__BUTTON_CANCEL = m_rect_BUTTON_CANCEL.bottom - m_rect_BUTTON_CANCEL.top; //heightΪbutton�ĸ�

	////�����ڳ�ʼ����λ��
	//GetDlgItem(IDD_RTSPCLIENT_DIALOG)->GetWindowRect(&m_rect_MAINFRAME);//��ȡ�ؼ��������Ļ��λ��
	//ScreenToClient(m_rect_MAINFRAME);//ת��Ϊ�Ի����ϵ����λ��
	//m_width_MAINFRAME = m_rect_MAINFRAME.right - m_rect_MAINFRAME.left;  //widthΪbutton�Ŀ�
	//m_height_BMAINFRAME = m_rect_MAINFRAME.bottom - m_rect_MAINFRAME.top; //heightΪbutton�ĸ�

	return 1;
}

void CRTSPClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//�򿪰�ťλ�õ���

	if (m_IsAccess_control_position == 1)
	{
		GetDlgItem(btnOpen)->MoveWindow(m_rect_BUTTON_OPEN.left,cy - m_height__BUTTON_OPEN,
			m_width_BUTTON_OPEN,m_height__BUTTON_OPEN);

		//ץȡͼƬ��ť��ťλ�õ���
		GetDlgItem(IDC_btnCatchPic)->MoveWindow(m_rect_BUTTON_CATCH_PIC.left,cy - m_height__BUTTON_CATCH_PIC,
			m_width_BUTTON_CATCH_PIC,m_height__BUTTON_CATCH_PIC);

		//�༭��λ�õ���
		GetDlgItem(IDC_editURL)->MoveWindow(m_rect_EDIT_URL.left,cy - m_height__EDIT_URL,
			m_width_EDIT_URL,m_height__EDIT_URL);

		//��̬�ı���λ�õ���
		GetDlgItem(IDC_STATIC)->MoveWindow(m_rect_STATIC_URL.left,cy - m_height__STATIC_URL,
			m_width_STATIC_URL,m_height__STATIC_URL);

		//�˳���ťλ�õ���
		GetDlgItem(IDC_BUTTON_EXIT)->MoveWindow(m_rect_BUTTON_CANCEL.left,cy - m_height__BUTTON_CANCEL,
			m_width_BUTTON_CANCEL,m_height__BUTTON_CANCEL);

		if (m_is_full_screen)  //���ȫ����Ҫ���µ���
		{
			//��������λ�õ���
			GetDlgItem(IDC_VIEWFRAME)->MoveWindow(m_rect_PICURE_CONTROL.left ,m_rect_PICURE_CONTROL.top,
				cx ,cy);
		}
		else
		{

			//��������λ�õ���
			GetDlgItem(IDC_VIEWFRAME)->MoveWindow(m_rect_PICURE_CONTROL.left ,m_rect_PICURE_CONTROL.top,cx - (m_width_MAINFRAME - m_rect_PICURE_CONTROL.right),cy - (m_height_BMAINFRAME - m_rect_PICURE_CONTROL.bottom));
		}
		//��ȡ��ʾͼ������� 
		GetDlgItem(IDC_VIEWFRAME)->GetWindowRect(&m_Picture_rect);
		//�����������С����Ҫ��ʾ��sdl
		m_screen_width  = m_Picture_rect.right - m_Picture_rect.left;
		m_screen_height = m_Picture_rect.bottom - m_Picture_rect.top;

		//����������С�� ����
		if (m_screen_width == 0 || m_screen_height == 0)
		{
			m_screen_width = m_width_PICURE_CONTROL;    //����Ϊ��ʼ���� ��
			m_screen_height = m_height_PICURE_CONTROL;  //����Ϊ��ʼ���� ��
		}

		//�����ǻ�ȡ�� STATIC�ؼ��Ĵ�С 
		//�������ж�����Ϊ ����ı��Сʱ �� �� �͸� ���� ������ͼ��᲻��ȷ �� �ж� Ҫż��
		if (m_screen_width % 2 != 0)
		{
			m_screen_width -= 1;
		}
		if (m_screen_height % 2 != 0)
		{
			m_screen_height -= 1;
		}



		//����һ��Ҫ����ˢ��MFC����
		Invalidate(TRUE);
	}
	else
	{
		m_width_MAINFRAME = cx;
		m_height_BMAINFRAME = cy;
	}


}


void CRTSPClientDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default

	lpMMI->ptMinTrackSize.x = MFCMINWINDOWSIZE_WIDTH; 
	lpMMI->ptMinTrackSize.y = MFCMINWINDOWSIZE_HEIGHT; 

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CRTSPClientDlg::OnBnClickedButtonExit()
{
	// TODO: Add your control notification handler code here

	//���ٱ�����
	UinitProgram();
	//////////////////////////////////////////////////////////////////////////
	avformat_network_deinit();
	SDL_Quit();
	OnCancel();
}


void CRTSPClientDlg::OnBnClickedButtonPause()
{
	// TODO: Add your control notification handler code here
	if (ffmpegProcess.m_pause_play)
	{
		//��ͣ
		SDL_PauseAudio(1);
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(_T("����")) ;
		//�༭��ʹ��
		GetDlgItem(IDC_editURL)->EnableWindow(TRUE);
			//�ر�ץȡͼƬ��ť
    	GetDlgItem(IDC_btnCatchPic)->EnableWindow(FALSE);
		m_bitmap_play.SetIcon(hIcon_play);
		m_bitmap_open.SetIcon(hIcon_open); 
		//�򿪴�ͼƬ��ť
		GetDlgItem(btnOpen)->EnableWindow(TRUE);

		if(m_streamstate->videoq.nb_packets <= 0)
		{
			//�ͷ�������������
			ffmpegProcess.ffmpeg_queueDestroy();
		}
		else
		{

		}
	}
	else
	{
		//����
		SDL_PauseAudio(0);
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(_T("��ͣ")) ;
		m_bitmap_play.SetIcon(hIcon_pause);
		m_bitmap_open.SetIcon(hIcon_open_b); 
		//�رմ�ͼƬ��ť
		GetDlgItem(btnOpen)->EnableWindow(FALSE);
			//ʹ��ץȡͼƬ��ť
    	GetDlgItem(IDC_btnCatchPic)->EnableWindow(TRUE);
	}
	ffmpegProcess.m_pause_play = !ffmpegProcess.m_pause_play;

}


void CRTSPClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
		OnBnClickedbtncatchpic();
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CRTSPClientDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default


	OnBnClickedButtonExit();

	CDialogEx::OnClose();
}


//void CRTSPClientDlg::OnEnChangeediturl()
//{
//	// TODO:  If this is a RICHEDIT control, the control will not
//	// send this notification unless you override the CDialogEx::OnInitDialog()
//	// function and call CRichEditCtrl().SetEventMask()
//	// with the ENM_CHANGE flag ORed into the mask.
//
//	// TODO:  Add your control notification handler code here
//	
//}
