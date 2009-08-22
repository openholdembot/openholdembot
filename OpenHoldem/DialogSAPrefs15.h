#ifndef INC_DIALOGSAPREFS15_H
#define INC_DIALOGSAPREFS15_H

#include "resource.h"
#include "afxwin.h"


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

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};

#endif