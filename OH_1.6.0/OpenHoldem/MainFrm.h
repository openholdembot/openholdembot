#ifndef INC_MAINFRM_H
#define INC_MAINFRM_H

#include "MyCToolBar.h"
#include "structs_defines.h"

// Used for holding table list by callback function
extern CArray <STableList, STableList>		g_tlist;

// Startup path
extern char		startup_path[MAX_PATH];

#define		HWND_CHECK_TIMER				1
#define		ENABLE_BUTTONS_TIMER			2
#define		UPDATE_STATUS_BAR_TIMER			3
#define		ATTACH_WINDOW_TIMER				4

class CMainFrame : public CFrameWnd {
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
	CStatusBar		m_wndStatusBar;
	CMyToolBar		m_FlagsToolBar;
	CString			status_plcards, status_comcards, status_pokerhand, status_prwin, status_nopp, status_nit, status_action,
					status_handrank;
	RECT			table_view_size;

	int create_main_toolbar(void);
	int create_flags_toolbar(void);
	void align_toolbars(void);
	int create_status_bar(void);
	afx_msg void OnEditFormula();
	afx_msg void OnScraperOutput();
	afx_msg void OnViewShootreplayframe();
	afx_msg void OnEditPreferences();
	afx_msg void OnFileLoadTableMap();
	afx_msg void OnDllLoad();
	afx_msg void OnBnClickedRedCircle();
	afx_msg void OnBnClickedGreenCircle();
	afx_msg void OnClickedFlags();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateStatus(CCmdUI *pCmdUI);
	afx_msg void OnAutoplayer();
	afx_msg void OnMinMax();
	afx_msg void OnAttachTop();
	afx_msg void OnAttachBottom();
	afx_msg void OnLockBlinds();
	afx_msg void OnPokerproConnect();
	afx_msg void OnUpdateMenuFileNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuFileLoadProfile(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileConnect(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuDllLoad(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDllLoadspecificfile(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewMainToolbar(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewFlagsToolbar(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewStatusbar(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePokerproConnect(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewShootreplayframe(CCmdUI *pCmdUI);
	afx_msg void OnFormulaViewMainToolbar();
	afx_msg void OnFormulaViewFlagsToolbar();
	afx_msg void OnFormulaViewStatusbar();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDllLoadspecificfile();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CMainFrame();
	virtual BOOL DestroyWindow();
	CMyToolBar		m_MainToolBar;
};

#endif /* INC_MAINFRM_H */