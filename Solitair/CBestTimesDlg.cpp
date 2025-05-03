// CBestTimesDlg.cpp : implementation file
//

#include "pch.h"
#include "Solitair.h"
#include "afxdialogex.h"
#include "CBestTimesDlg.h"


// CBestTimesDlg dialog

IMPLEMENT_DYNAMIC(CBestTimesDlg, CDialogEx)

CBestTimesDlg::CBestTimesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CBestTimesDlg::~CBestTimesDlg()
{
}

void CBestTimesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_besttimeslist);
}


BEGIN_MESSAGE_MAP(CBestTimesDlg, CDialogEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CBestTimesDlg message handlers
BOOL CBestTimesDlg::OnInitDialog()
{
	int			i;
	CString		s;
	
	if (!CDialogEx::OnInitDialog())
		return FALSE;
	for (i = 0; i < 10; i++) {
		if (m_besttimes[i].m_datetime > 0) {
			// one time to show
			CTime		t(m_besttimes[i].m_datetime);
			s.Format(_T("Time: %d:%02d %s"), m_besttimes[i].m_besttime/60, m_besttimes[i].m_besttime%60,t.Format(_T("%H:%M %A, %B %d, %Y")));
			m_besttimeslist.AddString(s);
		}
	}
	

	return TRUE;
}
