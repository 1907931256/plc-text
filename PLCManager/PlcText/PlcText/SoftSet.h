#pragma once
#include "afxwin.h"


// CSoftSet 对话框
#define WM_CONFIGURE_REFRESH (WM_USER+200)
#define WM_DATAVIEWER_CLICK (WM_USER+201)

#define WM_PTZTESTPANEL_CLICK (WM_USER+202)
class CSoftSet : public CDialog
{
	DECLARE_DYNAMIC(CSoftSet)

public:
	CSoftSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSoftSet();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cLogClear;
	CString m_csTime;
	int m_iTimeEnable;
	int m_iAutoClear;
	int m_iPtz;
	int m_iZoom;
	int m_iTvwall;
	int m_iSwitchGroup;
	int m_nEnableWatchdog ;
	int m_nPlCHeartbeatTimeout;
	HWND fHandle;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedCheckEnablewatchdog();
	afx_msg void OnBnClickedCheckEnable();
	afx_msg void OnBnClickedButtonGroupRefresh();
	afx_msg void OnBnClickedButtonDataviewerM();
	afx_msg void OnBnClickedButtonPtztest();
};
