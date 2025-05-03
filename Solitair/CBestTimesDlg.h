#pragma once
#include "afxdialogex.h"
#include "ChildView.h"


// CBestTimesDlg dialog

class CBestTimesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBestTimesDlg)

public:
	CBestTimesDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBestTimesDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_besttimeslist;
	BESTTIMERECORD	m_besttimes[10];  // The last 10 best times
};
