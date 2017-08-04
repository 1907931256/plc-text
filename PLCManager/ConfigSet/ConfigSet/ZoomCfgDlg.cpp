// m_ZoomCfgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "ZoomCfgDlg.h"


// m_ZoomCfgDlg 对话框

IMPLEMENT_DYNAMIC(ZoomCfgDlg, CDialog)

ZoomCfgDlg::ZoomCfgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ZoomCfgDlg::IDD, pParent)
	,m_csIpcZoomCfg(_T(""))
	,m_csIp(_T(""))
	,m_csPuid(_T(""))
	,m_csDefaultZoom(_T(""))
	,m_csValueTemp(_T(""))
{

}

ZoomCfgDlg::~ZoomCfgDlg()
{
}

void ZoomCfgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IP, m_csEditIp);
	DDX_Control(pDX, IDC_EDIT_PUID, m_csEditPuid);
	DDX_Control(pDX, IDC_EDIT_DEFULT_ZOOM, m_csEditDefaultZoom);
	DDX_Control(pDX, IDC_TVWALL_GROUP_INDEX, m_nComFeet);
	DDX_Control(pDX, IDC_EDIT_HEIGHT_MAX, m_EditHeightMax);
	DDX_Control(pDX, IDC_EDIT_ZOOM_VALUE, m_EditZoomValue);
	DDX_Control(pDX, IDC_EDIT_HEIGHT_MIN, m_EditHeightMin);
}


BEGIN_MESSAGE_MAP(ZoomCfgDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD, &ZoomCfgDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDOK, &ZoomCfgDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL  ZoomCfgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString csFeet[] = {_T("20"), _T("30"), _T("40"), _T("45")};

	for (int i = 0; i < 4; i++)
	{
		m_nComFeet.AddString(csFeet[i]);
	}
	m_nComFeet.SetCurSel(0);

	m_csEditIp.SetWindowText(m_csIp);
	m_csEditPuid.SetWindowText(m_csPuid);
	m_csEditDefaultZoom.SetWindowText(m_csDefaultZoom);

	m_EditHeightMin.SetWindowText(_T("10"));
	m_EditHeightMax.SetWindowText(_T("20"));
	m_EditZoomValue.SetWindowText(_T("1000"));

	return TRUE;
}

//// m_ZoomCfgDlg 消息处理程序


void ZoomCfgDlg::OnBnClickedBtnAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csFeet, csHeightMax, csHeightMin, csZoomValue, csValueTemp;
	m_nComFeet.GetWindowText(csFeet);
	m_EditHeightMin.GetWindowText(csHeightMin);
	m_EditHeightMax.GetWindowText(csHeightMax);
	m_EditZoomValue.GetWindowText(csZoomValue);

	csValueTemp.Format(_T("%s-%s-%s-%s,"), csFeet, csHeightMin, csHeightMax, csZoomValue);
	m_csValueTemp += csValueTemp;
}

void ZoomCfgDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csPuid, csDefaultZoom;

	m_csEditPuid.GetWindowText(csPuid);
	m_csEditDefaultZoom.GetWindowText(csDefaultZoom);

	m_csValueTemp = m_csValueTemp.Left(m_csValueTemp.GetLength() - 1);

	m_csIpcZoomCfg = csPuid + _T(",") + csDefaultZoom + _T(",") + m_csValueTemp;

	OnOK();
}

void ZoomCfgDlg::UnInitData()
{
    m_csIpcZoomCfg = m_csValueTemp = _T("");
}