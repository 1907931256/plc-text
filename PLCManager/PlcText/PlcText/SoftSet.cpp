// SoftSet.cpp : 实现文件
//

#include "stdafx.h"
#include "PlcText.h"
#include "SoftSet.h"
#include "Utility.h"
#include "IniFile.h"
#include "Language.h"
#include "Utility.h"

// CSoftSet 对话框

IMPLEMENT_DYNAMIC(CSoftSet, CDialog)

CSoftSet::CSoftSet(CWnd* pParent /*=NULL*/)
	: CDialog(CSoftSet::IDD, pParent)
{
	m_nEnableWatchdog  = 0;
	m_nPlCHeartbeatTimeout = 0;

}

CSoftSet::~CSoftSet()
{
}

void CSoftSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CLEAR, m_cLogClear);
}


BEGIN_MESSAGE_MAP(CSoftSet, CDialog)
	ON_BN_CLICKED(IDOK, &CSoftSet::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSoftSet::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_ENABLEWATCHDOG, &CSoftSet::OnBnClickedCheckEnablewatchdog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE, &CSoftSet::OnBnClickedCheckEnable)
	ON_BN_CLICKED(IDC_BUTTON_GROUP_REFRESH, &CSoftSet::OnBnClickedButtonGroupRefresh)
	ON_BN_CLICKED(IDC_BUTTON_DATAVIEWER_M, &CSoftSet::OnBnClickedButtonDataviewerM)
	ON_BN_CLICKED(IDC_BUTTON_PTZTEST, &CSoftSet::OnBnClickedButtonPtztest)
END_MESSAGE_MAP()


// CSoftSet 消息处理程序

BOOL CSoftSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(_CS("Software_Set"));
	SendDlgItemMessage(IDC_COMBO_LANGUAGE, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(_CS("Language_Chs")));
	SendDlgItemMessage(IDC_COMBO_LANGUAGE, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(_CS("Language_Eng")));
	if (CLanguage::Instance()->GetLanguageType().CompareNoCase(_T("CHS")) == 0)
		SendDlgItemMessage(IDC_COMBO_LANGUAGE,CB_SETCURSEL,0,0);
	else
		SendDlgItemMessage(IDC_COMBO_LANGUAGE,CB_SETCURSEL,1,0);

	//SetDlgItemText(IDC_STATIC_LANGUAGE,_CS("LanguageTitle"));

	m_cLogClear.InsertString(0, _CS("DontClear"));
	m_cLogClear.InsertString(1, _CS("Clear1Week"));
	m_cLogClear.InsertString(2, _CS("Clear2Week"));
	m_cLogClear.InsertString(3, _CS("Clear3Week"));
	m_cLogClear.InsertString(4, _CS("Clear4Week"));
	const TCHAR *szArray[]= {
		_T("LanguageTitle"),
		_T("RestartOnTime"),
		_T("EanbleRestartTime"),
		_T("RestartTime"),
		_T("log"),
		_T("AutoClear"),
//		_T("DontClear"),
// 		_T("Clear1Week"),
// 		_T("Clear2Week"),
// 		_T("Clear3Week"),
// 		_T("Clear4Week"),
		_T("LogType"),
		_T("SwitchGroup"),
		_T("Watchdog"),
		_T("EnableWatchdog"),
		_T("PLCHeartTimeout"),
		_T("OK"),
		_T("Cancel")};
	CWnd *pWnd = GetWindow(GW_CHILD);
	int nIndex = 0;
	int nArrayCount =  sizeof(szArray)/sizeof(TCHAR*);
	TCHAR szClassName[128] = {0};
	TCHAR szText[256] = {0};
	while(pWnd)
	{
		INT nID = pWnd->GetDlgCtrlID();
		if (nID == IDC_CHECK_PTZ || nID == IDC_CHECK_ZOOM|| nID ==IDC_CHECK_TVWALL)
		{
			pWnd = pWnd->GetNextWindow(GW_HWNDNEXT);
			continue;
		}
		pWnd->GetWindowText(szText,256);
		if (nIndex < nArrayCount)
		{
			GetClassName(pWnd->m_hWnd,szClassName,128);
			if (_tcscmp(szClassName,_T("Button")) == 0 || _tcscmp(szClassName,_T("Static")) == 0)
			{
				pWnd->SetWindowText(_CS2(szArray[nIndex]));
				TraceMsgW(_T("%s	%s.\n"),szText,_CS2(szArray[nIndex]));
				nIndex ++;
			}
		}
				
		pWnd = pWnd->GetNextWindow(GW_HWNDNEXT);
	}


	m_cLogClear.SetCurSel(m_iAutoClear);
	SetDlgItemText(IDC_EDIT_TIME, m_csTime);
	((CButton *)GetDlgItem(IDC_CHECK_ENABLE))->SetCheck(m_iTimeEnable);
	((CButton *)GetDlgItem(IDC_CHECK_PTZ))->SetCheck(m_iPtz);
	((CButton *)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(m_iZoom);
	((CButton *)GetDlgItem(IDC_CHECK_TVWALL))->SetCheck(m_iTvwall);
	((CButton *)GetDlgItem(IDC_CHECK_SWITCH))->SetCheck(m_iSwitchGroup);
	SetDlgItemText(IDC_BUTTON_GROUP_REFRESH,_CS("ConfigR"));
	SetDlgItemText(IDC_BUTTON_DATAVIEWER_M,_CS("DataViwer"));
	
	CheckDlgButton(IDC_CHECK_ENABLEWATCHDOG,m_nEnableWatchdog);

	GetDlgItem(IDC_EDIT_PLCTIMEOUT)->EnableWindow(m_nEnableWatchdog == 1);
	GetDlgItem(IDC_EDIT_TIME)->EnableWindow(m_iTimeEnable == 1);


	SetDlgItemInt(IDC_EDIT_PLCTIMEOUT,m_nPlCHeartbeatTimeout);

	SetDlgItemText(IDC_BUTTON_PTZTEST,_CS("PtzTest"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSoftSet::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemText(IDC_EDIT_TIME, m_csTime);
	m_iAutoClear = m_cLogClear.GetCurSel();
	m_iTimeEnable = ((CButton *)GetDlgItem(IDC_CHECK_ENABLE))->GetCheck();
	m_iPtz = ((CButton *)GetDlgItem(IDC_CHECK_PTZ))->GetCheck();
	m_iTvwall = ((CButton *)GetDlgItem(IDC_CHECK_TVWALL))->GetCheck();
	m_iZoom = ((CButton *)GetDlgItem(IDC_CHECK_ZOOM))->GetCheck();
	m_iSwitchGroup = ((CButton *)GetDlgItem(IDC_CHECK_SWITCH))->GetCheck();
	m_nPlCHeartbeatTimeout = GetDlgItemInt(IDC_EDIT_PLCTIMEOUT);
	m_nEnableWatchdog = IsDlgButtonChecked(IDC_CHECK_ENABLEWATCHDOG);
	TCHAR szAppPath[1024] = {0};
	GetAppPath(szAppPath,1024);
	_tcscat(szAppPath,_T("\\Language.ini"));
	CIniFile LanguageIni(szAppPath);
	int nLanguaeSel = SendDlgItemMessage(IDC_COMBO_LANGUAGE,CB_GETCURSEL);
	if (nLanguaeSel == 0)
		LanguageIni.WriteString(_T("LANGUAGE"),_T("TYPE"),_T("CHS"));
	else
		LanguageIni.WriteString(_T("LANGUAGE"),_T("TYPE"),_T("ENG"));

	OnOK();
}

void CSoftSet::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

void CSoftSet::OnBnClickedCheckEnablewatchdog()
{
	GetDlgItem(IDC_EDIT_PLCTIMEOUT)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_ENABLEWATCHDOG) == BST_CHECKED);
}

void CSoftSet::OnBnClickedCheckEnable()
{
	GetDlgItem(IDC_EDIT_TIME)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_ENABLE) == BST_CHECKED);
}

void CSoftSet::OnBnClickedButtonGroupRefresh()
{
	// TODO: 在此添加控件通知处理程序代码
	::SendMessage(fHandle,WM_CONFIGURE_REFRESH,NULL,NULL);
}


void CSoftSet::OnBnClickedButtonDataviewerM()
{
	// TODO: 在此添加控件通知处理程序代码
	::SendMessage(fHandle,WM_DATAVIEWER_CLICK,NULL,NULL);
}

void CSoftSet::OnBnClickedButtonPtztest()
{
	// TODO: 在此添加控件通知处理程序代码
	::SendMessage(fHandle,WM_PTZTESTPANEL_CLICK,NULL,NULL);
}
