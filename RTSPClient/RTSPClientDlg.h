
// RTSPClientDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#include "CffmpegProcess.h"



#define MFCMINWINDOWSIZE_WIDTH  1700
#define MFCMINWINDOWSIZE_HEIGHT 700



// CRTSPClientDlg dialog
class CRTSPClientDlg : public CDialogEx
{
	// Construction
public:
	CRTSPClientDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	enum { IDD = IDD_RTSPCLIENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedbtnopen();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedbtncatchpic();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonPause();

	CString m_editURL;
	CString szUrl;
	//CEdit m_ctrEditURL;

	int m_IsAccess_control_position;        //是否已经获取了控件的初始化位置

	CRect m_rect_BUTTON_OPEN;               //打开按钮初始化的位置
	int m_width_BUTTON_OPEN;                //打开按钮的宽
	int m_height__BUTTON_OPEN;              //打开按钮的高

	CRect m_rect_BUTTON_CATCH_PIC;         //抓取图片按钮初始化的位置
	int m_width_BUTTON_CATCH_PIC;          //抓取图片按钮的宽
	int m_height__BUTTON_CATCH_PIC;        //抓取图片按钮的高

	CRect m_rect_EDIT_URL;               //编辑框初始化的位置
	int m_width_EDIT_URL;                //编辑框的宽
	int m_height__EDIT_URL;              //编辑框的高

	CRect m_rect_STATIC_URL;             //静态文本框初始化的位置
	int m_width_STATIC_URL;              //静态文本框的宽
	int m_height__STATIC_URL;            //静态文本框的高

	CRect m_rect_PICURE_CONTROL;            //播放区域初始化的位置
	int m_width_PICURE_CONTROL;             //播放区域的宽
	int m_height_PICURE_CONTROL;            //播放区域的高

	CRect m_rect_BUTTON_CANCEL;             //退出按钮初始化的位置
	int m_width_BUTTON_CANCEL;              //退出按钮的宽
	int m_height__BUTTON_CANCEL;            //退出按钮的高


	int m_width_MAINFRAME;                  //主窗口的宽
	int m_height_BMAINFRAME;                //主窗口的高

	CRect m_Picture_rect;                    //显示图像的区域大小
	CStatic m_picture_static;                //播放区域指针

	//获取控件初始化位置 //只调用一次
	int Access_control_position();   
	//程序启动做的初始化 打开编解码库 SDL初始化等,创建默认窗口  //只调用一次
	int InitProgram(); 
	//程序退出做的析构关闭库销毁窗口等  //只调用一次
	int UinitProgram();
	////程序所需要的变量分配等
	//int InitVariable();
	////程序所需要的变量析构等
	//int UinitVariable();
	////释放内存资源
	//void freeBuf();


	//SDL_Overlay     *catch_bmp;
	
	int m_is_full_screen;                    //是否全屏播放
	int m_screen_width ;                     //改变大小后的宽和高
	int m_screen_height;                     //改变大小后的宽和高
	int m_client_exist;                        //0表示RTSPClien不存在 1表示存在

	CButton m_bitmap_open;
	CButton m_bitmap_catch;
	CButton m_bitmap_exit;
	CButton m_bitmap_play;

	//按钮图标
	HICON hIcon_open;    
	HICON hIcon_open_b;
	HICON hIcon_play;
	HICON hIcon_pause;
	HICON hIcon_play_b;
	HICON hIcon_catch ;
	HICON hIcon_catch_b;
	HICON hIcon_exit;


	CRTSPSession RTSPSession;
	CffmpegProcess ffmpegProcess;
//	afx_msg void OnEnChangeediturl();
};

