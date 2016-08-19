
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
	//获取控件在对话框上的坐标
	m_IsAccess_control_position = Access_control_position();

	//初始化ffmepg
	ffmpegProcess.ffmpeg_init();

	//////////////////////////////////////////////////////////////////////////
	//获取全屏的屏幕宽高  这里只能调用一次 第二次调用时 大小将改变
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_client_exist = 0;
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;

	//初始视频大小
	m_picture_static.GetWindowRect(&m_Picture_rect); 
	m_screen_width = m_Picture_rect.right - m_Picture_rect.left;
	m_screen_height = m_Picture_rect.bottom - m_Picture_rect.top;
	//这里是获取到 STATIC控件的大小 
	//这里做判断是因为 如果改变大小时 的 宽 和高 出现 奇数则图像会不正确 或 中断 要偶数
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
	//关闭ffmpeg
	ffmpegProcess.ffmpeg_exit();

	//关闭RTSP客户端
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
	//窗口不可以改变大小
	ModifyStyle(WS_THICKFRAME, NULL);


	//////////////////////////////////////////////////////////////////////////
	//关闭抓取图片按钮
	GetDlgItem(IDC_btnCatchPic)->EnableWindow(FALSE);
	//关闭暂停图片按钮
	GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(FALSE);

	//////////////////////////////////////////////
	//按钮图标设置
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

	//初始化编辑框
	//rtsp://192.168.23.105:8554/test
	//rtsp://admin:12345@192.168.23.98
	m_editURL = "rtsp://admin:12345@192.168.23.98";

	//初始化全局变量
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
	//打开其他URL时，先释放上一个URL资源
	if (!ffmpegProcess.m_pause_play)
	{
		UinitProgram();
	}

	//初始化变量等
	InitProgram();
	Sleep(500);

	//窗口重绘
	Invalidate(TRUE);

	//关闭抓取图片按钮
	GetDlgItem(IDC_btnCatchPic)->EnableWindow(TRUE);
	//关闭打开图片按钮
	GetDlgItem(btnOpen)->EnableWindow(FALSE);
	//关闭暂停图片按钮
	GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(TRUE);
	//编辑框关闭
	GetDlgItem(IDC_editURL)->EnableWindow(FALSE);

	//////////////////////////////////////////////
	//按钮图标设置

	m_bitmap_open.SetIcon(hIcon_open); 

	m_bitmap_play.SetIcon(hIcon_pause); 

	m_bitmap_catch.SetIcon(hIcon_catch);

	//获取显示区域句柄
	HWND Wnd = this->GetDlgItem(IDC_VIEWFRAME)->GetSafeHwnd();
	//ffmpeg初始化
	ffmpegProcess.ffmpeg_initDecoder(Wnd);

	//获取编辑框中的地址
	GetDlgItem(IDC_editURL)->GetWindowText(szUrl);

	//打开URL链接
	if(RTSPSession.startRTSPClient("test_client", szUrl))
	{
		Sleep(500);
		int startPlay_Flag = ffmpegProcess.startPlay_tidFun();
		if (!startPlay_Flag)
		{
			MessageBox(_T("创建读取线程失败 清楚重新创建"), NULL, MB_OK );

		}
	}

	//重新刷新MFC窗口
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
	//打开按钮初始化的位置
	GetDlgItem(btnOpen)->GetWindowRect(&m_rect_BUTTON_OPEN);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_BUTTON_OPEN);//转化为对话框上的相对位置
	m_width_BUTTON_OPEN = m_rect_BUTTON_OPEN.right - m_rect_BUTTON_OPEN.left;  //width为button的宽
	m_height__BUTTON_OPEN = m_rect_BUTTON_OPEN.bottom - m_rect_BUTTON_OPEN.top; //height为button的高

	//抓取图片按钮初始化的位置
	GetDlgItem(IDC_btnCatchPic)->GetWindowRect(&m_rect_BUTTON_CATCH_PIC);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_BUTTON_CATCH_PIC);//转化为对话框上的相对位置
	m_width_BUTTON_CATCH_PIC = m_rect_BUTTON_CATCH_PIC.right - m_rect_BUTTON_CATCH_PIC.left;  //width为button的宽
	m_height__BUTTON_CATCH_PIC = m_rect_BUTTON_CATCH_PIC.bottom - m_rect_BUTTON_CATCH_PIC.top; //height为button的高

	//编辑框初始化的位置
	GetDlgItem(IDC_editURL)->GetWindowRect(&m_rect_EDIT_URL);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_EDIT_URL);//转化为对话框上的相对位置
	m_width_EDIT_URL = m_rect_EDIT_URL.right - m_rect_EDIT_URL.left;  //width为button的宽
	m_height__EDIT_URL = m_rect_EDIT_URL.bottom - m_rect_EDIT_URL.top; //height为button的高


	//静态文本框初始化的位置
	GetDlgItem(IDC_STATIC)->GetWindowRect(&m_rect_STATIC_URL);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_STATIC_URL);//转化为对话框上的相对位置
	m_width_STATIC_URL = m_rect_STATIC_URL.right - m_rect_STATIC_URL.left;  //width为button的宽
	m_height__STATIC_URL = m_rect_STATIC_URL.bottom - m_rect_STATIC_URL.top; //height为button的高

	//播放区域初始化的位置
	GetDlgItem(IDC_VIEWFRAME)->GetWindowRect(&m_rect_PICURE_CONTROL);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_PICURE_CONTROL);//转化为对话框上的相对位置
	m_width_PICURE_CONTROL = m_rect_PICURE_CONTROL.right - m_rect_PICURE_CONTROL.left;  //width为button的宽
	m_height_PICURE_CONTROL = m_rect_PICURE_CONTROL.bottom - m_rect_PICURE_CONTROL.top; //height为button的高

	//退出按钮初始化的位置
	GetDlgItem(IDC_BUTTON_EXIT)->GetWindowRect(&m_rect_BUTTON_CANCEL);//获取控件相对于屏幕的位置
	ScreenToClient(m_rect_BUTTON_CANCEL);//转化为对话框上的相对位置
	m_width_BUTTON_CANCEL = m_rect_BUTTON_CANCEL.right - m_rect_BUTTON_CANCEL.left;  //width为button的宽
	m_height__BUTTON_CANCEL = m_rect_BUTTON_CANCEL.bottom - m_rect_BUTTON_CANCEL.top; //height为button的高

	////主窗口初始化的位置
	//GetDlgItem(IDD_RTSPCLIENT_DIALOG)->GetWindowRect(&m_rect_MAINFRAME);//获取控件相对于屏幕的位置
	//ScreenToClient(m_rect_MAINFRAME);//转化为对话框上的相对位置
	//m_width_MAINFRAME = m_rect_MAINFRAME.right - m_rect_MAINFRAME.left;  //width为button的宽
	//m_height_BMAINFRAME = m_rect_MAINFRAME.bottom - m_rect_MAINFRAME.top; //height为button的高

	return 1;
}

void CRTSPClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//打开按钮位置调整

	if (m_IsAccess_control_position == 1)
	{
		GetDlgItem(btnOpen)->MoveWindow(m_rect_BUTTON_OPEN.left,cy - m_height__BUTTON_OPEN,
			m_width_BUTTON_OPEN,m_height__BUTTON_OPEN);

		//抓取图片按钮按钮位置调整
		GetDlgItem(IDC_btnCatchPic)->MoveWindow(m_rect_BUTTON_CATCH_PIC.left,cy - m_height__BUTTON_CATCH_PIC,
			m_width_BUTTON_CATCH_PIC,m_height__BUTTON_CATCH_PIC);

		//编辑框位置调整
		GetDlgItem(IDC_editURL)->MoveWindow(m_rect_EDIT_URL.left,cy - m_height__EDIT_URL,
			m_width_EDIT_URL,m_height__EDIT_URL);

		//静态文本框位置调整
		GetDlgItem(IDC_STATIC)->MoveWindow(m_rect_STATIC_URL.left,cy - m_height__STATIC_URL,
			m_width_STATIC_URL,m_height__STATIC_URL);

		//退出按钮位置调整
		GetDlgItem(IDC_BUTTON_EXIT)->MoveWindow(m_rect_BUTTON_CANCEL.left,cy - m_height__BUTTON_CANCEL,
			m_width_BUTTON_CANCEL,m_height__BUTTON_CANCEL);

		if (m_is_full_screen)  //如果全屏需要重新调整
		{
			//播放区域位置调整
			GetDlgItem(IDC_VIEWFRAME)->MoveWindow(m_rect_PICURE_CONTROL.left ,m_rect_PICURE_CONTROL.top,
				cx ,cy);
		}
		else
		{

			//播放区域位置调整
			GetDlgItem(IDC_VIEWFRAME)->MoveWindow(m_rect_PICURE_CONTROL.left ,m_rect_PICURE_CONTROL.top,cx - (m_width_MAINFRAME - m_rect_PICURE_CONTROL.right),cy - (m_height_BMAINFRAME - m_rect_PICURE_CONTROL.bottom));
		}
		//获取显示图像的区域 
		GetDlgItem(IDC_VIEWFRAME)->GetWindowRect(&m_Picture_rect);
		//将窗口区域大小传给要显示的sdl
		m_screen_width  = m_Picture_rect.right - m_Picture_rect.left;
		m_screen_height = m_Picture_rect.bottom - m_Picture_rect.top;

		//这里设置最小化 托盘
		if (m_screen_width == 0 || m_screen_height == 0)
		{
			m_screen_width = m_width_PICURE_CONTROL;    //设置为初始化的 宽
			m_screen_height = m_height_PICURE_CONTROL;  //设置为初始化的 高
		}

		//这里是获取到 STATIC控件的大小 
		//这里做判断是因为 如果改变大小时 的 宽 和高 出现 奇数则图像会不正确 或 中断 要偶数
		if (m_screen_width % 2 != 0)
		{
			m_screen_width -= 1;
		}
		if (m_screen_height % 2 != 0)
		{
			m_screen_height -= 1;
		}



		//这里一定要重新刷新MFC窗口
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

	//销毁变量等
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
		//暂停
		SDL_PauseAudio(1);
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(_T("播放")) ;
		//编辑框使能
		GetDlgItem(IDC_editURL)->EnableWindow(TRUE);
			//关闭抓取图片按钮
    	GetDlgItem(IDC_btnCatchPic)->EnableWindow(FALSE);
		m_bitmap_play.SetIcon(hIcon_play);
		m_bitmap_open.SetIcon(hIcon_open); 
		//打开打开图片按钮
		GetDlgItem(btnOpen)->EnableWindow(TRUE);

		if(m_streamstate->videoq.nb_packets <= 0)
		{
			//释放流缓冲区队列
			ffmpegProcess.ffmpeg_queueDestroy();
		}
		else
		{

		}
	}
	else
	{
		//播放
		SDL_PauseAudio(0);
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(_T("暂停")) ;
		m_bitmap_play.SetIcon(hIcon_pause);
		m_bitmap_open.SetIcon(hIcon_open_b); 
		//关闭打开图片按钮
		GetDlgItem(btnOpen)->EnableWindow(FALSE);
			//使能抓取图片按钮
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
