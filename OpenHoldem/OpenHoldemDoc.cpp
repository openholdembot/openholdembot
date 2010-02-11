// OpenHoldemDoc.cpp : implementation of the COpenHoldemDoc class
//

#include "stdafx.h"
#include "CAutoplayer.h"
#include "CDllExtension.h"
#include "CPreferences.h"
#include "DialogFormulaScintilla.h"
#include "MainFrm.h"
#include "OH_MessageBox.h"
#include "OpenHoldem.h"
#include "OpenHoldemDoc.h"


// COpenHoldemDoc
IMPLEMENT_DYNCREATE(COpenHoldemDoc, CDocument)

BEGIN_MESSAGE_MAP(COpenHoldemDoc, CDocument)
END_MESSAGE_MAP()

// COpenHoldemDoc construction/destruction
COpenHoldemDoc::COpenHoldemDoc() 
{
	p_formula->ClearFormula();
}

COpenHoldemDoc::~COpenHoldemDoc() 
{
}

BOOL COpenHoldemDoc::OnNewDocument() 
{
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	if (!CDocument::OnNewDocument())
	return FALSE;

	// Kill the formula dialog, if it is open
	if(m_formulaScintillaDlg) 
	{
		delete m_formulaScintillaDlg;
		m_formulaScintillaDlg	=	NULL;
		pMyMainWnd->m_MainToolBar.GetToolBarCtrl().CheckButton(ID_MAIN_TOOLBAR_FORMULA, false);
	}

	// Default bot
	p_formula->SetDefaultBot();

	// Try to unload dll
	p_dll_extension->UnloadDll();

	// Create hand list matrices
	p_formula->CreateHandListMatrices();

	// Create parse trees for default formula
	p_formula->ParseAllFormula(pMyMainWnd->GetSafeHwnd(), prefs.disable_msgbox());

	SetTitle("Default");
	//SetModifiedFlag(true);

	// Load dll, if set in preferences
	if (prefs.dll_load_on_startup())
		p_dll_extension->LoadDll("");

	if (prefs.simple_window_title() && theApp.m_pMainWnd)
		theApp.m_pMainWnd->PostMessage(WMA_SETWINDOWTEXT, 0, (LPARAM)NULL);

	return true;
}


// COpenHoldemDoc serialization
void COpenHoldemDoc::Serialize(CArchive& ar) 
{
	// Extra caution, in case we want to load a formula,
	// while the autoplayer is engaged.
	// This currently can only happen via the MRU-list.
	// The alternative way would be to disable that list...
	//
	// MainFrame.cpp:
	// ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, &CMainFrame::OnUpdateLRUList)
	//
	// void CMainFrame::OnUpdateLRUList(CCmdUI *pCmdUI)
	// {
	//	pCmdUI->Enable(!p_autoplayer->autoplayer_engaged());
	// }
	//
	// Unfortunatelly this does not work, 
	// but removes the list and replaces it once with the default: "Recent file".
	//
	// And there's very little information about both
	// ON_UPDATE_COMMAND_UI_RANGE and temporary disabling of MRU-lists.
	//
	// So we decided to go that route.
	//
	if (p_autoplayer->autoplayer_engaged())
	{
		// This error can happen only in interactive mode,
		// so there's no need to turn that Messagebox off
		// depending on prefs.disable_msgbox()
		OH_MessageBox("Can't load a formula while autoplayer engaged.", "ERROR", 0);
		return;
	}
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	// Writing a file
	if (ar.IsStoring()) 
	{
		// Store archive in the new OpenHoldem format
		bool use_new_OHF_format = !IsWinHoldemFormat(ar.GetFile()->GetFileName());
		p_formula->WriteFormula(ar);
		// Do not close this archive here.
		// It's expected to stay open at this point!
		if (IsWinHoldemFormat(ar.GetFile()->GetFileName())) 
		{	
			// If the file was in the old WHF format,
			// store it also in the new OHF format.
			CString the_new_FileName = GetPathName();
			the_new_FileName.Replace("whf", "ohf");		
			// Notification
			OH_MessageBox_Interactive("Converting file formats\n{whf, whx} -> {ohf}",
				"File Conversion", MB_OK | MB_ICONINFORMATION);	
			// Open new style formula (OHF)	
			CFile OHF_File;
			OHF_File.Open(the_new_FileName, CFile::modeCreate | CFile::modeWrite);
			CArchive OHF_Archive(&OHF_File, CArchive::store);
			// Write new style formula (OHF) in any case
			p_formula->WriteFormula(OHF_Archive);
			// Close archive and file
			OHF_Archive.Close();
			OHF_File.Close();		
		}
	}
	// Reading a file
	else 
	{
		// Kill the formula dialog, if it is open
		if(m_formulaScintillaDlg) 
		{
			delete m_formulaScintillaDlg;
			m_formulaScintillaDlg	=	NULL;
			pMyMainWnd->m_MainToolBar.GetToolBarCtrl().CheckButton(ID_MAIN_TOOLBAR_FORMULA, false);
		}

		// Read ohf / whf file
		ReadFormula(ar);
		SetModifiedFlag(false);

		p_formula->set_formula_name(ar.GetFile()->GetFileName());

		// Try to unload dll
		p_dll_extension->UnloadDll();

		// Create hand list matrices
		p_formula->CreateHandListMatrices();

		// Create parse trees for newly loaded formula
		p_formula->ParseAllFormula(pMyMainWnd->GetSafeHwnd(), prefs.disable_msgbox());

		// Load dll, if set in preferences
		if (prefs.dll_load_on_startup())
			p_dll_extension->LoadDll("");
		if (prefs.simple_window_title() && theApp.m_pMainWnd)
			theApp.m_pMainWnd->PostMessage(WMA_SETWINDOWTEXT, 0, (LPARAM)NULL);
	}
}

void COpenHoldemDoc::ReadFormula(CArchive& ar) 
{
	// Clear everything
	p_formula->ClearFormula();

	// There are two types of formulas
	//   * ohf 
	//   * whf and optional whx
	// In the latter case we have to read both files. 
	p_formula->ReadFormulaFile(ar, true);

	CFile *cf_whf = ar.GetFile();  
	CString CSpath = cf_whf->GetFilePath(); 

	if (IsWinHoldemFormat(CSpath))
		{
			CFile *cf_whf = ar.GetFile();
			CFile cf_whx; 
			CString CSpath = cf_whf->GetFilePath();
			CSpath.Replace(".whf", ".whx");

			if (cf_whx.Open(CSpath, CFile::modeNoTruncate | CFile::modeRead| CFile::shareDenyWrite)) 
			{ 
				CArchive ar_whx(&cf_whx, CArchive::load);   
				// Read whx file, too. //???	
				p_formula->ReadFormulaFile(ar_whx, false);	
			}
	}

	// Check and add missing...
	p_formula->CheckForDefaultFormulaEntries();
}

BOOL COpenHoldemDoc::IsWinHoldemFormat(CString the_FileName)
{	
	unsigned int Length = the_FileName.GetLength();

	// Path maybe undefined at startup...
	if (Length < 3)
		return false;

	// Checking the first character of the file extension
	// Assuming an extension of size 3: "ohf" or "whf".
	char critical_Character = the_FileName.GetString()[Length - 3];

	return (critical_Character == 'w');
}

COpenHoldemDoc * COpenHoldemDoc::GetDocument() 
{
	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	return (COpenHoldemDoc *) pFrame->GetActiveDocument();
}

