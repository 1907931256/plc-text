#pragma once
#include "afxwin.h"
#include "IniFile.h"
#include "TypeDef.h"
#include "afxcmn.h"
#include "ScreenDlg.h"
#include "ZoomCfgDlg.h"

#define N_SIDE_WND_ID		WM_USER + 1000

// GroupCfgSet 对话框
class CConfigSetDlg;

class GroupCfgSet : public CDialog
{
	DECLARE_DYNAMIC(GroupCfgSet)

public:
	GroupCfgSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~GroupCfgSet();

// 对话框数据
	enum { IDD = IDD_DLG_GROUP };

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnTvwallCroup();
	afx_msg void OnCbnSelchangeGroupNum();
	afx_msg void OnCbnSelchangeTvwallScreenNum();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMDblclkListIpcGroup(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListIpcZoom(NMHDR *pNMHDR, LRESULT *pResult);

	void SetIniFilePath(CString csPath){m_csIniPath = csPath + _T("Config\\");}
	void SetMainDlg(CConfigSetDlg *pMainDlg){m_pMainCfgDlg = pMainDlg;}
	void RefrushIpc();
	void CleanIpcZoom();
	void CleanDeviceList();
	void ArrayWindow(WORD iNumber, CRect Rect);
	void InitWind();
	void CleanIpcScreenlist();
	BOOL CheckTvWallScreenIndex(int iTvWallIndex, int iScreenIndex, int & iTotleIndex);

	void DrawActivePage(BOOL bActive);

	void RenewSplitScreen(int nTvwallScreen, int nMode);
	std::vector<CString> Split(CString string, CString separator = _T(","));
	CString FindIPC(CString strPUID);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CConfigSetDlg *m_pMainCfgDlg;
	CString m_csIniPath;
	CComboBox m_ComBoxGroup;        //组号
	CComboBox m_ComBoxMode;         //模式
	CComboBox m_ComBoxScreenIndex;  //屏幕序号
	CComboBox m_ScreenSplitNum;     //屏幕分割数
	CEdit m_TvWallPort;             //tv_wall 端口
	CIPAddressCtrl m_TvWallIp;      //tv_wall IP
	CListCtrl m_Ipclist;
	CListCtrl m_IpcListZoom;
	
	int m_nTvwallScreen;
	int m_nMode;

	int m_nCurIndex;
	CBrush m_brushSel;
	CWnd *m_wndSelect[4];
	CScreenDlg m_videoWnd[MAXWNDNUM];
	BOOL m_bCheckBol;

    TvWallIndex2Screen m_MapTvWallIndex2ScreenIndex;
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnSetZoom();
	afx_msg void OnCbnSelchangeTvwallTotleScreen();
	afx_msg void OnCbnSelchangeTvwallMode();
	afx_msg void OnBnClickedButton1();
	CButton m_cCheckFreeCut;
	afx_msg void OnBnClickedCheckFreecut();
	afx_msg void OnBnClickedButton2();
};
