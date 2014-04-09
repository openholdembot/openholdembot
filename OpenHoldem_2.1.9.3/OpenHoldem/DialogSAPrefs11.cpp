// DialogSAPrefs10.cpp : implementation file
//

#include "stdafx.h"

#include <limits.h>

#include "SAPrefsSubDlg.h"
#include "DialogSAPrefs11.h"
#include "CPreferences.h"

#define MAX_MAX_LOG 1000000

// DialogSAPrefs11 dialog


IMPLEMENT_DYNAMIC(CDlgSAPrefs11, CDialog)

CDlgSAPrefs11::CDlgSAPrefs11(CWnd* pParent /*=NULL*/)
		: CSAPrefsSubDlg(CDlgSAPrefs11::IDD, pParent)
{
}

CDlgSAPrefs11::~CDlgSAPrefs11()
{
}

void CDlgSAPrefs11::DoDataExchange(CDataExchange* pDX)
{
	CSAPrefsSubDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENABLE_LOG, m_EnableLog);
	DDX_Control(pDX, IDC_MAXIMUM_LOG, m_MaximumLog);
	DDX_Control(pDX, IDC_MAXIMUM_LOG_SPIN, m_MaximumLog_Spin);
	DDX_Control(pDX, IDC_ENABLE_TRACE, m_EnableTrace);
	DDX_Control(pDX, IDC_DISABLE_MSGBOX, m_disable_msgbox);
	DDX_Control(pDX, IDC_MAXIMUM_LOGSIZE, m_MaximumLogSize);
	DDX_Control(pDX, IDC_MAXIMUM_LOGSIZE_SPIN, m_MaximumLogSize_Spin);
}

BOOL CDlgSAPrefs11::OnInitDialog()
{
	CSAPrefsSubDlg::OnInitDialog();
	CString		text = "";

	m_EnableLog.SetCheck(prefs.log_symbol_enabled() ? BST_CHECKED : BST_UNCHECKED);
	m_EnableTrace.SetCheck(prefs.trace_enabled() ? BST_CHECKED : BST_UNCHECKED);

	text.Format("%d", prefs.log_symbol_max_log());
	m_MaximumLog.SetWindowText(text);
	m_MaximumLog_Spin.SetRange(0, (short) MAX_MAX_LOG);
	m_MaximumLog_Spin.SetPos(prefs.log_symbol_max_log());
	m_MaximumLog_Spin.SetBuddy(&m_MaximumLog);

	m_disable_msgbox.SetCheck(prefs.disable_msgbox() ? BST_CHECKED : BST_UNCHECKED);

	text.Format("%d", prefs.log_max_logsize());
	m_MaximumLogSize.SetWindowText(text);
	m_MaximumLogSize_Spin.SetRange(0, 999);
	m_MaximumLogSize_Spin.SetPos(prefs.log_max_logsize());
	m_MaximumLogSize_Spin.SetBuddy(&m_MaximumLogSize);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CDlgSAPrefs11, CSAPrefsSubDlg)
	ON_BN_CLICKED(IDC_DISABLE_MSGBOX, &CDlgSAPrefs11::OnBnClickedDisableMsgbox)
END_MESSAGE_MAP()

// DialogSAPrefs11 message handlers
void CDlgSAPrefs11::OnOK()
{
	CString			text = "";

	prefs.set_log_symbol_enabled(m_EnableLog.GetCheck()==BST_CHECKED ? true : false);
	prefs.set_trace_enabled(m_EnableTrace.GetCheck()==BST_CHECKED ? true : false);

	m_MaximumLog.GetWindowText(text);
	if (strtoul(text.GetString(), 0, 10)<0 || strtoul(text.GetString(), 0, 10)>MAX_MAX_LOG) {
		MessageBox("Invalid maximum log amount!", "ERROR", MB_OK);
		return;
	}
	prefs.set_log_symbol_max_log(strtoul(text.GetString(), 0, 10));

	prefs.set_disable_msgbox(m_disable_msgbox.GetCheck()==BST_CHECKED ? true : false);

	m_MaximumLogSize.GetWindowText(text);
	prefs.set_log_max_logsize(strtoul(text.GetString(), NULL, 10));
	
	CSAPrefsSubDlg::OnOK();
}


void CDlgSAPrefs11::OnBnClickedDisableMsgbox()
{
	if (m_disable_msgbox.GetCheck()==BST_CHECKED)
		MessageBox("Warning: Selecting this option instructs OpenHoldem to refrain from\n"
				   "displaying ANY runtime informational or error message boxes.  Examples\n"
				   "include parse errors, DLL load errors, etc.  It is strongly advised that\n"
				   "this option only be used in a production environment that has been\n"
				   "completely and thoroughly tested for proper behavior.\n\n"
				   "Note that interactive messages are not disabled, such as when using the\n"
				   "formula editor and the PokerPro dialog.", "WARNING", MB_OK);
}
