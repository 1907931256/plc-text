#pragma once
#include "IniFile.h"

// GeneralCfgSet �Ի���

class GeneralCfgSet : public CDialog
{
	DECLARE_DYNAMIC(GeneralCfgSet)

public:
	GeneralCfgSet(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~GeneralCfgSet();

// �Ի�������
	enum { IDD = IDD_DLG_GENERAL };

public:
	void SetIniFilePath(CString csPath){m_csIniPath = csPath + _T("Config\\");}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	CString m_csIniPath;
};
