
// ConfigSetDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "ConfigSetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CConfigSetDlg �Ի���

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


// CConfigSetDlg ��Ϣ�������

BOOL CConfigSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	CWnd *pTabWnd = this->GetDlgItem(IDC_MAINTAB);
//	m_tab.InsertItem(0, _T("    ��������    "));
	m_tab.InsertItem(0, _T("    �豸����    "));
	m_tab.InsertItem(1, _T("    ��������    "));

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

	m_Groupcfg.RenewSplitScreen(1,0);	//��ʼ��Ĭ����Ļ�ָ�״̬


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CConfigSetDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CConfigSetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CConfigSetDlg::OnTcnSelchangeMaintab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
