
// Solitair.h : main header file for the Solitair application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSolitairApp:
// See Solitair.cpp for the implementation of this class
//

class CSolitairApp : public CWinApp
{
public:
	CSolitairApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSolitairApp theApp;
