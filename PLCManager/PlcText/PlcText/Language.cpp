#include "StdAfx.h"
#include "Language.h"

CLanguage* CLanguage::s_pLanguage = NULL;
CLanguage::CLanguage(void)
{
}

CLanguage::~CLanguage(void)
{
}

CLanguage* CLanguage::Instance()
{
	if (NULL == s_pLanguage)
	{
		s_pLanguage = new CLanguage();
	}

	return s_pLanguage;
}

void CLanguage::UnInstace()
{
	if (s_pLanguage)
	{
		delete s_pLanguage;
		s_pLanguage = NULL;
	}
}

BOOL CLanguage::SetLangPath(CString strPath, CString strType)
{
	m_strPath = strPath;
	m_strType = strType;
	return TRUE;
}

CString CLanguage::GetStr(CString strKey)
{
	TCHAR val[256] = {0};
	memset(val, 0, 256);
	GetPrivateProfileString(m_strType, strKey, strKey, val, 256, m_strPath);
	return val;
}

CString CLanguage::GetStrEx(CString strKey)
{
	CString strEx = GetStr(strKey);
	strEx.Replace(L"\\r", L"\r");
	strEx.Replace(L"\\n", L"\n");
	strEx.Replace(L"\\t", L"\t");
	return strEx;
}

void CLanguage::SetWndText(CWnd * pWnd)
{
	CString strCaption,strText;

	//设置主窗口的标题
	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText = GetStr(strCaption);
		pWnd->SetWindowText(strText);
	}

	//设置子窗口的标题
	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")	//下一个子窗口
		{
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}

		//设置子窗口的当前语言文本
		pChild->GetWindowText(strCaption);
		if (_T("reboot_effect") == strCaption)
		{
			int mm = 12;
		}
		strText = GetStr(strCaption);
		pChild->SetWindowText(strText);

		

		//下一个子窗口
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}
