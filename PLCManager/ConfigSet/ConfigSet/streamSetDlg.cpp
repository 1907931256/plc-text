// streamSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "streamSetDlg.h"


// CstreamSetDlg 对话框

IMPLEMENT_DYNAMIC(CstreamSetDlg, CDialog)

CstreamSetDlg::CstreamSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CstreamSetDlg::IDD, pParent)
	,m_csStreamType(_T("1"))
{

}

CstreamSetDlg::~CstreamSetDlg()
{
}

void CstreamSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_StrramType);
}


BEGIN_MESSAGE_MAP(CstreamSetDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CstreamSetDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

BOOL CstreamSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetDlgItemText(IDC_DEVICEID, m_csDeviceID);
	SetDlgItemText(IDC_DEVICENAME, m_csDeviceName);
	SetDlgItemText(IDC_IP, m_csIp);
	SetDlgItemText(IDC_PORT, m_csPort);
	if (_T("2") == m_csCompany)
	{
		SetDlgItemText(IDC_COMPANY, _T("Axis"));
	}
	else if (_T("4") == m_csCompany)
	{
		SetDlgItemText(IDC_COMPANY, _T("HIK"));
	}
	else
	{
		SetDlgItemText(IDC_COMPANY, _T("DEV 2.0"));
	}
	SetDlgItemText(IDC_USER, m_csUser);
	SetDlgItemText(IDC_PASSWORD, m_csPassWord);
	SetDlgItemText(IDC_DEVICETYPE, m_csDeviceType);
	
	m_StrramType.AddString(_T("1"));
	m_StrramType.AddString(_T("2"));
	m_StrramType.SetCurSel(0);

	return TRUE;

}

void CstreamSetDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemText(IDC_DEVICEID, m_csDeviceID);
	GetDlgItemText(IDC_DEVICENAME, m_csDeviceName);
	GetDlgItemText(IDC_IP, m_csIp);
	GetDlgItemText(IDC_PORT, m_csPort);
	GetDlgItemText(IDC_COMPANY, m_csCompany);
	GetDlgItemText(IDC_USER, m_csUser);
	GetDlgItemText(IDC_PASSWORD, m_csPassWord);
	GetDlgItemText(IDC_DEVICETYPE, m_csDeviceType);

	CString csStreamType;
	m_StrramType.GetWindowText(csStreamType);
	m_csStreamType = csStreamType;

	CDialog::OnOK();
}
