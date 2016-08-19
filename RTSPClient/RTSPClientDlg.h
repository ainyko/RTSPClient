
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

	int m_IsAccess_control_position;        //�Ƿ��Ѿ���ȡ�˿ؼ��ĳ�ʼ��λ��

	CRect m_rect_BUTTON_OPEN;               //�򿪰�ť��ʼ����λ��
	int m_width_BUTTON_OPEN;                //�򿪰�ť�Ŀ�
	int m_height__BUTTON_OPEN;              //�򿪰�ť�ĸ�

	CRect m_rect_BUTTON_CATCH_PIC;         //ץȡͼƬ��ť��ʼ����λ��
	int m_width_BUTTON_CATCH_PIC;          //ץȡͼƬ��ť�Ŀ�
	int m_height__BUTTON_CATCH_PIC;        //ץȡͼƬ��ť�ĸ�

	CRect m_rect_EDIT_URL;               //�༭���ʼ����λ��
	int m_width_EDIT_URL;                //�༭��Ŀ�
	int m_height__EDIT_URL;              //�༭��ĸ�

	CRect m_rect_STATIC_URL;             //��̬�ı����ʼ����λ��
	int m_width_STATIC_URL;              //��̬�ı���Ŀ�
	int m_height__STATIC_URL;            //��̬�ı���ĸ�

	CRect m_rect_PICURE_CONTROL;            //���������ʼ����λ��
	int m_width_PICURE_CONTROL;             //��������Ŀ�
	int m_height_PICURE_CONTROL;            //��������ĸ�

	CRect m_rect_BUTTON_CANCEL;             //�˳���ť��ʼ����λ��
	int m_width_BUTTON_CANCEL;              //�˳���ť�Ŀ�
	int m_height__BUTTON_CANCEL;            //�˳���ť�ĸ�


	int m_width_MAINFRAME;                  //�����ڵĿ�
	int m_height_BMAINFRAME;                //�����ڵĸ�

	CRect m_Picture_rect;                    //��ʾͼ��������С
	CStatic m_picture_static;                //��������ָ��

	//��ȡ�ؼ���ʼ��λ�� //ֻ����һ��
	int Access_control_position();   
	//�����������ĳ�ʼ�� �򿪱����� SDL��ʼ����,����Ĭ�ϴ���  //ֻ����һ��
	int InitProgram(); 
	//�����˳����������رտ����ٴ��ڵ�  //ֻ����һ��
	int UinitProgram();
	////��������Ҫ�ı��������
	//int InitVariable();
	////��������Ҫ�ı���������
	//int UinitVariable();
	////�ͷ��ڴ���Դ
	//void freeBuf();


	//SDL_Overlay     *catch_bmp;
	
	int m_is_full_screen;                    //�Ƿ�ȫ������
	int m_screen_width ;                     //�ı��С��Ŀ�͸�
	int m_screen_height;                     //�ı��С��Ŀ�͸�
	int m_client_exist;                        //0��ʾRTSPClien������ 1��ʾ����

	CButton m_bitmap_open;
	CButton m_bitmap_catch;
	CButton m_bitmap_exit;
	CButton m_bitmap_play;

	//��ťͼ��
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

