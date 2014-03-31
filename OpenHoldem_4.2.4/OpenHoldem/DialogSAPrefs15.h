//*****************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//*****************************************************************************
//
// Purpose:
//
//*****************************************************************************

#ifndef INC_DIALOGSAPREFS15_H
#define INC_DIALOGSAPREFS15_H

#include "resource.h"
#include "afxwin.h"
#include "SAPrefsDialog.h"


// CDlgSAPrefs15 dialog

class CDlgSAPrefs15 : public CSAPrefsSubDlg
{
	DECLARE_DYNAMIC(CDlgSAPrefs15)

public:
	CDlgSAPrefs15(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSAPrefs15();

// Dialog Data
	enum { IDD = IDD_SAPREFS15 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	OnInitDialog();
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

public:
	
	CButton			_gui_start_minimized_Button;
	CButton			_gui_disable_progress_dialog_Button;
};

#endif