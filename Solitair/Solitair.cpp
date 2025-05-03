
// Solitair.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Solitair.h"
#include "MainFrm.h"
#include "ChildView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSolitairApp

BEGIN_MESSAGE_MAP(CSolitairApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CSolitairApp::OnAppAbout)
END_MESSAGE_MAP()


// CSolitairApp construction

CSolitairApp::CSolitairApp() noexcept
{

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Solitair.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CSolitairApp object

CSolitairApp theApp;


// CSolitairApp initialization

BOOL CSolitairApp::InitInstance()
{
	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("MSSolitair"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr,
		nullptr);





	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int CSolitairApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	return CWinApp::ExitInstance();
}

// CSolitairApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	CEdit	prevhighscr;
	CString		m_highscore;
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, prevhighscr);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CAboutDlg::OnPaint()
{
	prevhighscr.SetWindowTextW(m_highscore);
}

// App command to run the dialog
void CSolitairApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	if(((CMainFrame*)m_pMainWnd)->m_wndView.m_highscore>0)
		aboutDlg.m_highscore.Format(_T("%d:%d"), ((CMainFrame *)m_pMainWnd)->m_wndView.m_highscore/60, ((CMainFrame*)m_pMainWnd)->m_wndView.m_highscore %60);
	
	aboutDlg.DoModal();
}

// CSolitairApp message handlers



