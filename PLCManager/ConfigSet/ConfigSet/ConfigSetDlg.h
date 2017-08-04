
// ConfigSetDlg.h : ͷ�ļ�
//
#pragma once
#include "afxcmn.h"
#include "GroupCfgSet.h"
#include "GeneralCfgSet.h"
#include "IpcGroupCfg.h"

// CConfigSetDlg �Ի���
class CConfigSetDlg : public CDialog
{
// ����
public:
	CConfigSetDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CONFIGSET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	afx_msg void OnTcnSelchangeMaintab(NMHDR *pNMHDR, LRESULT *pResult);


	CString GetCurrentPath();

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	GeneralCfgSet m_GeneralCfg;
	GroupCfgSet m_Groupcfg;
	IpcGroupCfg m_IpcGroupSet;

	//typedef std::map<int, IpcList> IpcGroupMap;
	IpcGroupMap m_IpcGroupMap;
};
