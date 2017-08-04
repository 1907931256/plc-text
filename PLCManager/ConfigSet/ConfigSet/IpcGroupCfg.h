#pragma once
#include "IniFile.h"
#include "afxcmn.h"
#include "BasicExcel.h"
#include "afxwin.h"
#include "streamSetDlg.h"
#include "WGlobal.h"

class CConfigSetDlg;

// IpcGroupCfg 对话框
using namespace YExcel;

typedef struct DEVICEINFO
{
	CString csDeviceId;
	CString csDeviceName;
}DEVICEINFO;
class IpcGroupCfg : public CDialog
{
	DECLARE_DYNAMIC(IpcGroupCfg)

public:
	IpcGroupCfg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~IpcGroupCfg();
	virtual BOOL OnInitDialog();

// 对话框数据
	enum { IDD = IDD_DLG_IPCGROUPSET };

public:
	afx_msg void OnBnClickedBtnSelect();
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnNMDblclkListIpc(NMHDR *pNMHDR, LRESULT *pResult);

	void SetIniFilePath(CString csPath){m_csIniPath = csPath + _T("Config\\");m_csOpenPath = csPath;}
	void AddDeviceListFromExls();
	void CleanDeviceList();
	void SetMainDlg(CConfigSetDlg *pMianCfgDlg){m_pMainCfgDlg = pMianCfgDlg;}
	void CleanGroupTree();
	void ReadGroupIPC();
	void SaveToExcel(int iIndex);
	char *WtoM(CString csStr);
	int IsSameDevice(CString csDeviceId, CString DeviceName, int num = 0);
	std::vector<CString> Split(CString string, CString separator = _T(","));

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CTreeCtrl m_GroupTree;
	CConfigSetDlg *m_pMainCfgDlg;
	CString m_csDeviceFile;
	CString m_csIniPath;
	CString m_csOpenPath;
	CListCtrl m_Devlist;
	CComboBox m_GroupNum;
	CImageList	m_cImageList;
	CHeaderCtrl *m_pHeadCtrl;

	std::map<int , DEVICEINFO> m_mDeviceMap;
	std::map<int , CString> m_mSameDeviceID;
	std::map<int , CString> m_mSameDeviceName;


	CButton m_cCheckBox;
	afx_msg UINT OnNotifyFormat(CWnd *pWnd, UINT nCommand);
	afx_msg void OnHdnItemclickListIpc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListIpc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnCustomdrawMyList(NMHDR* pNMHDR, LRESULT* pResult);  
	CButton m_btnset;
	afx_msg void OnBnClickedBtnDel();
	CButton m_cBtnDel;
};
