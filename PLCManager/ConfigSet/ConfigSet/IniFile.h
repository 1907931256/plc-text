
#pragma once
#include <vector>

class CIniFile
{
public:
	CIniFile() : m_strFile(_T("")) {}
	CIniFile(const CString& strFile) : m_strFile(strFile) 
	{
		SetFileAttributes( m_strFile, FILE_ATTRIBUTE_NORMAL );
	}
	~CIniFile() {}

	void SetFileName(const CString& strFile) 
	{ 
		m_strFile = strFile; 
		SetFileAttributes( m_strFile, FILE_ATTRIBUTE_NORMAL );
	}
	const CString& GetFileName() const { return m_strFile; }

	void ReadSection(CStringArray& setArray);
	void ReadSectionEntry (const TCHAR* pSection,  CStringArray& setArray);
    void ReadSectionEntry(const TCHAR* pSection, std::vector<CString>& setArray);
	void ReadSectionString(const TCHAR* lpSection, CStringArray& setArray);
	void ReadString	 (const TCHAR* pSection,  const TCHAR* pEntry, CString& strItem);
	void ReadInt	 (const TCHAR* pSection,  const TCHAR* pEntry, int& nValue);
	void ReadInt	 (const TCHAR* pSection,  const TCHAR* pEntry, short& nValue);
	void WriteString (const TCHAR* pSection,  const TCHAR* pEntry, const TCHAR* pItem);
	void WriteInt	 (const TCHAR* pSection,  const TCHAR* pEntry, int nValue);
	void EraseSection(const TCHAR* pSection);

protected:
	CString m_strFile;
};
