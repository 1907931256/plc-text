#pragma once
#include "IniFile.h"

// GeneralCfgSet 对话框

class GeneralCfgSet : public CDialog
{
	DECLARE_DYNAMIC(GeneralCfgSet)

public:
	GeneralCfgSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~GeneralCfgSet();

// 对话框数据
	enum { IDD = IDD_DLG_GENERAL };

public:
	void SetIniFilePath(CString csPath){m_csIniPath = csPath + _T("Config\\");}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CString m_csIniPath;
};
