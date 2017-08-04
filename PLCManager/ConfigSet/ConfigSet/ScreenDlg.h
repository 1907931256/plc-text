#pragma once
#include <vector>
class GroupCfgSet;
// CScreenDlg 对话框

class CScreenDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenDlg)

public:
	CScreenDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScreenDlg();

// 对话框数据
	enum { IDD = IDD_DLG_SCREEN};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	void SetParent(GroupCfgSet *pGroupCfg){m_pGroupCfgSetDlg = pGroupCfg;}
	void SetWndIndex(int iIndex){m_iIndex = iIndex;}
	void SetOutText(CString csText){m_csText = csText;}
	void SetIpcPuid(CString csPuid){m_csIpcPuid = csPuid;}
	std::vector<CString> Split(CString string, CString separator = _T(","));

	DECLARE_MESSAGE_MAP()

public:
	CString m_csIpcPuid;
	CString m_csText;
	CBrush m_brushVideo;
	int m_iIndex;
	GroupCfgSet *m_pGroupCfgSetDlg;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	void OnContextMenu(CWnd* , CPoint point);
	afx_msg void OnClean();
	afx_msg void On32774();
};
