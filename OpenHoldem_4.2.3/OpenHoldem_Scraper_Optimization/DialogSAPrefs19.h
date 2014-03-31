#ifndef INC_DIALOGSAPREFS19_H
#define INC_DIALOGSAPREFS19_H

#include "resource.h"
#include "afxwin.h"

#include "SAPrefsDialog.h"


// CDlgSAPrefs15 dialog

class CDlgSAPrefs19 : public CSAPrefsSubDlg
{
	DECLARE_DYNAMIC(CDlgSAPrefs19)

public:
	CDlgSAPrefs19(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSAPrefs19();

// Dialog Data
	enum { IDD = IDD_SAPREFS19 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	CButton _handhistory_generator_enable_Button;
};

#endif