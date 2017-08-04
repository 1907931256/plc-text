#pragma once
#include "afxwin.h"


// CstreamSetDlg �Ի���

class CstreamSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CstreamSetDlg)

public:
	CstreamSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CstreamSetDlg();
	virtual BOOL OnInitDialog();

// �Ի�������
	enum { IDD = IDD_DLG_STREAM};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
