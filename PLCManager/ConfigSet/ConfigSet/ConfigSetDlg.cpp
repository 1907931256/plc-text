
// ConfigSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "ConfigSetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CConfigSetDlg 对话框

CConfigSetDlg::CConfigSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigSetDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConfigSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAINTAB, m_tab);
}

BEGIN_MESSAGE_MAP(CConfigSetDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGE, IDC_MAINTAB, &CConfigSetDlg::OnTcnSelchangeMaintab)
END_MESSAGE_MAP()


// CConfigSetDlg 消息处理程序

BOOL CConfigSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CWnd *pTabWnd = this->GetDlgItem(IDC_MAINTAB);
//	m_tab.InsertItem(0, _T("    基本设置    "));
	m_tab.InsertItem(0, _T("    设备分组    "));
	m_tab.InsertItem(1, _T("    分组设置    "));

	CRect rcHead, rect;        
	m_tab.GetItemRect(0, &rcHead);  
	m_tab.GetClientRect(&rect); 
	m_tab.GetClientRect(&rect); 
	rect.top += rcHead.Height() + 5; 

//	m_GeneralCfg.Create(GeneralCfgSet::IDD, pTabWnd);
//	m_GeneralCfg.MoveWindow(&rect);
//	m_GeneralCfg.ShowWindow(SW_SHOW);
	CString sIniPath = GetCurrentPath();
	m_GeneralCfg.SetIniFilePath(sIniPath);

	m_IpcGroupSet.Create(IpcGroupCfg::IDD, pTabWnd);
	m_IpcGroupSet.MoveWindow(&rect);
	m_IpcGroupSet.ShowWindow(SW_SHOW);
	m_IpcGroupSet.SetIniFilePath(sIniPath);
	m_IpcGroupSet.SetMainDlg(this);

	m_IpcGroupSet.ReadGroupIPC();
	m_Groupcfg.Create(GroupCfgSet::IDD, pTabWnd);
	m_Groupcfg.MoveWindow(&rect);
	m_Groupcfg.ShowWindow(SW_HIDE);
	m_Groupcfg.SetIniFilePath(sIniPath);
	m_Groupcfg.SetMainDlg(this);

	m_Groupcfg.RenewSplitScreen(1,0);	//初始化默认屏幕分割状态


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CConfigSetDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CConfigSetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CConfigSetDlg::OnTcnSelchangeMaintab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int iSelect = m_tab.GetCurSel(); 
	/*if (iSelect == 0)
	{
		m_GeneralCfg.ShowWindow(SW_SHOW);
		m_Groupcfg.ShowWindow(SW_HIDE);
		m_IpcGroupSet.ShowWindow(SW_HIDE);
	}
	else if (iSelect == 1)
	{
		m_IpcGroupSet.ShowWindow(SW_SHOW);
		m_GeneralCfg.ShowWindow(SW_HIDE);
		m_Groupcfg.ShowWindow(SW_HIDE);
	}
	else if (iSelect == 2)
	{
		m_GeneralCfg.ShowWindow(SW_HIDE);
		m_Groupcfg.ShowWindow(SW_SHOW);
		m_IpcGroupSet.ShowWindow(SW_HIDE);
		m_Groupcfg.RefrushIpc();
	}*/
	if (iSelect == 0)
	{
		m_IpcGroupSet.ShowWindow(SW_SHOW);
		m_Groupcfg.ShowWindow(SW_HIDE);
	}
	else if (iSelect == 1)
	{
		m_Groupcfg.ShowWindow(SW_SHOW);
		m_IpcGroupSet.ShowWindow(SW_HIDE);
		m_Groupcfg.RefrushIpc();
		m_Groupcfg.m_ComBoxMode.SetCurSel(0);
	}

	*pResult = 0;
}

CString CConfigSetDlg::GetCurrentPath()
{
	HMODULE module = GetModuleHandle(0); 
	WCHAR pFileName[MAX_PATH]; 
	GetModuleFileName(module, pFileName, MAX_PATH); 
	CString csFullPath = pFileName; 
	return csFullPath.Left(csFullPath.ReverseFind('\\')+1);
}
