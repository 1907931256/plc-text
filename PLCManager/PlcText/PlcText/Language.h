#pragma once

#define  _CS(x)     CLanguage::Instance()->GetStr(_T(x))
#define  _CS2(x)     CLanguage::Instance()->GetStr(x)
#define  _CSEx(x)     CLanguage::Instance()->GetStrEx(_T(x))
#define  _CWndCS(x) CLanguage::Instance()->SetWndText(x)

class CLanguage
{
public:
	CLanguage(void);
	~CLanguage(void);

public:
	static CLanguage* Instance();
	static void UnInstace();

	BOOL SetLangPath(CString strPath, CString strType=_T("CHS"));
	CString GetStr(CString strKey);
	CString GetStrEx(CString strKey);
	void SetWndText(CWnd * pWnd); 
	CString GetLanguageType()
	{
		return m_strType;
	}

private:
	static CLanguage* s_pLanguage;
	CString m_strPath;
	CString m_strType;
};
