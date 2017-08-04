#include "StdAfx.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////////////
void CIniFile::ReadSection(CStringArray& setArray)
{
	TCHAR szBuf[1024*64];
	memset(szBuf, 0, sizeof(szBuf));
	TCHAR* p = szBuf;
	int nLen = 0;

	if (GetPrivateProfileString(NULL, NULL, _T(""), 
		szBuf, sizeof(szBuf)/sizeof(TCHAR), 
		m_strFile) > 0)
	{
		while (*p != _T('\0'))
		{
			setArray.Add(p);
			nLen = (int)_tcslen(p) + 1;
			p += nLen;
		}  
	}
}

void CIniFile::ReadSectionEntry(const TCHAR* pSection, CStringArray& setArray)
{
    TCHAR szBuf[1024*64] = {0};
	TCHAR* p = szBuf;
	int nLen = 0;
	if (GetPrivateProfileString(pSection, NULL, _T(""), 
								szBuf, sizeof(szBuf)/sizeof(TCHAR), 
								m_strFile) > 0)
	{
		while (*p != _T('\0'))
		{
			setArray.Add(p);
			nLen = (int)_tcslen(p) + 1;
			p += nLen;
		}  
	}
}

void CIniFile::ReadSectionEntry(const TCHAR* pSection, std::vector<CString>& setArray)
{
    TCHAR szBuf[1024*64] = {0};
    TCHAR* p = szBuf;
    int nLen = 0;
    if (GetPrivateProfileString(pSection, NULL, _T(""), 
        szBuf, sizeof(szBuf)/sizeof(TCHAR), 
        m_strFile) > 0)
    {
        while (*p != _T('\0'))
        {
            setArray.push_back(CString(p));
            nLen = (int)_tcslen(p) + 1;
            p += nLen;
        }  
    }
}

void CIniFile::ReadSectionString(const TCHAR* pSection, CStringArray& setArray)
{
	CStringArray ayKey;
	CString strItem;
	ReadSectionEntry(pSection, ayKey);
	for (int i=0; i<ayKey.GetSize(); ++i)
	{
		ReadString(pSection, ayKey[i], strItem);
		if (!strItem.IsEmpty())
			setArray.Add(strItem);
	}
}

void CIniFile::ReadString(const TCHAR* pSection, const TCHAR* pEntry, CString& strItem)
{

	TCHAR szReturn[1024*4];
	memset(szReturn, 0, sizeof(szReturn));

	strItem.Empty();
	if (GetPrivateProfileString(pSection, pEntry, _T(""), 
								szReturn, _countof(szReturn), 
								m_strFile) > 0)
	{
		strItem = szReturn;
	}
}

void CIniFile::ReadInt(const TCHAR* pSection, const TCHAR* pEntry, int& nValue)
{
	TCHAR szReturn[32];
	memset(szReturn, 0, sizeof(szReturn));
	if (GetPrivateProfileString(pSection, pEntry, _T(""), 
								szReturn, _countof(szReturn), 
								m_strFile) > 0)
		nValue = _tstoi(szReturn);
	else
		nValue = 0;
}

void CIniFile::ReadInt(const TCHAR* pSection, const TCHAR* pEntry, short& nValue)
{
	int nIntValue = 0;
	ReadInt(pSection, pEntry, nIntValue);
	nValue = (short)nIntValue;
}

void CIniFile::WriteString(const TCHAR* pSection, const TCHAR* pEntry, const TCHAR* pItem)
{
	WritePrivateProfileString(pSection, pEntry, pItem, m_strFile);
}

void CIniFile::WriteInt(const TCHAR* pSection, const TCHAR* pEntry, int nValue)
{
	TCHAR szValue[32];
	_stprintf_s(szValue, _T("%d"), nValue);
	WriteString(pSection, pEntry, szValue);
}

void CIniFile::EraseSection(const TCHAR* pSection)
{
	WritePrivateProfileStruct(pSection, NULL, NULL, 0, m_strFile);
}
////////////////////////////////////////////////////////////////////////////////////////
