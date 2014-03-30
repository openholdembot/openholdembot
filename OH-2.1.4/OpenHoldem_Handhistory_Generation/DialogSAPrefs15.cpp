// CDlgSAPrefs15.cpp : implementation file
//

#include "stdafx.h"

#include "SAPrefsSubDlg.h"
#include "DialogSAPrefs15.h"
#include "CPreferences.h"


// CDlgSAPrefs15 dialog

IMPLEMENT_DYNAMIC(CDlgSAPrefs15, CDialog)

CDlgSAPrefs15::CDlgSAPrefs15(CWnd* pParent /*=NULL*/)
	: CSAPrefsSubDlg(CDlgSAPrefs15::IDD, pParent)
{

}

CDlgSAPrefs15::~CDlgSAPrefs15()
{
}

void CDlgSAPrefs15::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GUI_START_MINIMIZED, _gui_start_minimized_Button);
	DDX_Control(pDX, IDC_GUI_Disable_Progress_Dialog, _gui_disable_progress_dialog_Button);
}


BEGIN_MESSAGE_MAP(CDlgSAPrefs15, CDialog)
END_MESSAGE_MAP()


// CDlgSAPrefs15 message handlers
BOOL CDlgSAPrefs15::OnInitDialog()
{
	CSAPrefsSubDlg::OnInitDialog();

	_gui_start_minimized_Button.SetCheck(prefs.gui_start_minimized());
	_gui_disable_progress_dialog_Button.SetCheck(prefs.gui_disable_progress_dialog());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSAPrefs15::OnOK()
{
	prefs.set_gui_start_minimized(_gui_start_minimized_Button.GetCheck() == true);
	prefs.set_gui_disable_progress_dialog(_gui_disable_progress_dialog_Button.GetCheck() == true);

	CSAPrefsSubDlg::OnOK();
}