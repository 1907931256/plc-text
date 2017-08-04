
#pragma once
#include <Windows.h>
#include <vector>

class CIniFile
{
public:
	CIniFile() : m_strFile("") {}
	CIniFile(const std::string& strFile) : m_strFile(strFile) 
	{
		SetFileAttributes( m_strFile.c_str(), FILE_ATTRIBUTE_NORMAL );
	}
	~CIniFile() {}

	void SetFileName(const std::string& strFile) 
	{ 
		m_strFile = strFile; 
		SetFileAttributes( m_strFile.c_str(), FILE_ATTRIBUTE_NORMAL );
	}
	const std::string& GetFileName() const { return m_strFile; }

	void ReadSection(std::vector<std::string>& setArray);
	void ReadSectionEntry (const TCHAR* pSection,  std::vector<std::string>& setArray);
	//void ReadSectionEntry(const TCHAR* pSection, std::vector<std::string>& setArray);
	void ReadSectionString(const TCHAR* lpSection, std::vector<std::string>& setArray);
	void ReadString	 (const TCHAR* pSection,  const TCHAR* pEntry, std::string& strItem);
	void ReadInt	 (const TCHAR* pSection,  const TCHAR* pEntry, int& nValue);
	void ReadInt	 (const TCHAR* pSection,  const TCHAR* pEntry, short& nValue);
	void WriteString (const TCHAR* pSection,  const TCHAR* pEntry, const TCHAR* pItem);
	void WriteInt	 (const TCHAR* pSection,  const TCHAR* pEntry, int nValue);
	void EraseSection(const TCHAR* pSection);

protected:
	std::string m_strFile;
};
