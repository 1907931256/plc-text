#pragma once
#include "afxwin.h"


// m_ZoomCfgDlg �Ի���

class ZoomCfgDlg : public CDialog
{
	DECLARE_DYNAMIC(ZoomCfgDlg)

public:
	ZoomCfgDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~ZoomCfgDlg();

// �Ի�������
	enum { IDD = IDD_DLG_ZOOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

public:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedOk();
	void UnInitData();

public:
	CString m_csValueTemp;
	CString m_csIpcZoomCfg;
	CString m_csIp;
	CString m_csPuid;
	CString m_csDefaultZoom;

	CEdit m_csEditIp;
	CEdit m_csEditPuid;
	CEdit m_csEditDefaultZoom;

	CComboBox m_nComFeet;
	CEdit m_EditHeightMax;
	CEdit m_EditHeightMin;
	CEdit m_EditZoomValue;
};
