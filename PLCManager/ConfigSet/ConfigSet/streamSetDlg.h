#pragma once
#include "afxwin.h"


// CstreamSetDlg 对话框

class CstreamSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CstreamSetDlg)

public:
	CstreamSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CstreamSetDlg();
	virtual BOOL OnInitDialog();

// 对话框数据
	enum { IDD = IDD_DLG_STREAM};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_csStreamType;
	CComboBox m_StrramType;
	CString m_csDeviceID;
	CString m_csDeviceName;
	CString m_csIp;
	CString m_csPort;
	CString m_csCompany;
	CString m_csUser;
	CString m_csPassWord;
	CString m_csDeviceType;
	afx_msg void OnBnClickedButton1();
};
