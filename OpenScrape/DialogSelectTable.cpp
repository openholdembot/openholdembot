// SelectTableDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OpenScrape.h"
#include "DialogSelectTable.h"

// CDlgSelectTable dialog
IMPLEMENT_DYNAMIC(CDlgSelectTable, CDialog)

CDlgSelectTable::CDlgSelectTable(CWnd* pParent /*=NULL*/) : CDialog(CDlgSelectTable::IDD, pParent) 
{
    __SEH_SET_EXCEPTION_HANDLER	
}

CDlgSelectTable::~CDlgSelectTable() 
{
}

void CDlgSelectTable::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_LIST_SELECT_TABLE, m_List_Select_Table);
	DDX_Control(pDX, IDC_LIST_SELECT_TABLE, m_HSLB);
	DDX_Control(pDX, IDOK, m_OK);
}


BEGIN_MESSAGE_MAP(CDlgSelectTable, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgSelectTable::OnBnClickedOk)
	ON_LBN_DBLCLK(IDC_LIST_SELECT_TABLE, &CDlgSelectTable::OnLbnDblclkListSelectTable)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT_TABLE, &CDlgSelectTable::OnLbnSelchangeListSelectTable)
END_MESSAGE_MAP()


// CDlgSelectTable message handlers

void CDlgSelectTable::OnBnClickedOk() 
{
	__SEH_HEADER

	selected_item = m_HSLB.GetCurSel();
	OnOK();

	__SEH_LOGFATAL("CDlgSelectTable::OnBnClickedOk : \n");
}

BOOL CDlgSelectTable::OnInitDialog() 
{
	__SEH_HEADER

	int		i, N;

	CDialog::OnInitDialog();

	selected_item = -1;

	// Populate listbox
	N = (int) tlist.GetSize();
	for (i=0; i<N; i++) {
		m_HSLB.AddString(tlist[i].title.GetString());
	}

	m_OK.EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE

	__SEH_LOGFATAL("CDlgSelectTable::OnInitDialog : \n");
}

void CDlgSelectTable::OnLbnDblclkListSelectTable() 
{
	__SEH_HEADER

	OnBnClickedOk();

	__SEH_LOGFATAL("CDlgSelectTable::OnLbnDblclkListSelectTable : \n");
}

void CDlgSelectTable::OnLbnSelchangeListSelectTable() 
{
	__SEH_HEADER

	m_OK.EnableWindow(true);

	__SEH_LOGFATAL("CDlgSelectTable::OnLbnSelchangeListSelectTable : \n");
}
