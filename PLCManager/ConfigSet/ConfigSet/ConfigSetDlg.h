
// ConfigSetDlg.h : 头文件
//
#pragma once
#include "afxcmn.h"
#include "GroupCfgSet.h"
#include "GeneralCfgSet.h"
#include "IpcGroupCfg.h"

// CConfigSetDlg 对话框
class CConfigSetDlg : public CDialog
{
// 构造
public:
	CConfigSetDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CONFIGSET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	afx_msg void OnTcnSelchangeMaintab(NMHDR *pNMHDR, LRESULT *pResult);


	CString GetCurrentPath();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
