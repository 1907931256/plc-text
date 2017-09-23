#pragma once
#include "Global.h"
#include <Windows.h>

void SetStrCat(std::string &strOut, std::string strIn, int nValue)
{
	char szTemp[128] = {0};
	sprintf_s(szTemp, 128, "%s_%d", strIn.c_str(), nValue);
	strOut.append(szTemp);
}

void SetValue(std::string &sRet, const char *szKey, const char *szValue, bool bEnd)
{
	char szTemp[128] = {0};
	sprintf_s(szTemp, 128, "\"%s\":\"%s\"", szKey, szValue);
	sRet.append(szTemp);
	if (!bEnd)
	{
		sRet.append(",");
	}
}

void SetValue(std::string &sRet, const char *szKey, int nValue, bool bEnd)
{
	char szTemp[128] = {0};
	sprintf_s(szTemp, 128, "\"%s\":%d", szKey, nValue);
	sRet.append(szTemp);
	if (!bEnd)
	{
		sRet.append(",");
	}
}

void SetjsonEnd(std::string &sRet)
{
	sRet = sRet.substr(0, sRet.length()-1);
	sRet.append("}}");
}

std::string GetCurrentPath()
{
		HMODULE module = GetModuleHandle(0); 
		char pFileName[MAX_PATH]; 
		GetModuleFileNameA(module, pFileName, MAX_PATH); 

		std::string csFullPath = pFileName; 
		int nPos = csFullPath.rfind( '\\' );
		if( nPos < 0 ) 
		{
			return ""; 
		}
		else 
		{
			return csFullPath.substr(0, nPos); 
		}
		return "";
}

void StringSplit(const std::string& src, const std::string& separator, std::vector<std::string>& dest)
{
	dest.clear();
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator,start);
		if (index != std::string::npos)
		{    
			substring = str.substr(start,index-start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator,index);
			if (start == std::string::npos) return;
		}
	}while(index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);  
LPFN_ISWOW64PROCESS fnIsWow64Process;  
bool IsWow64()  
{  
	BOOL bIsWow64 = FALSE;  

	//IsWow64Process is not available on all supported versions of Windows.  
	//Use GetModuleHandle to get a handle to the DLL that contains the function  
	//and GetProcAddress to get a pointer to the function if available.  

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(  
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");  

	if(NULL != fnIsWow64Process)  
	{  
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))  
		{  
			//handle error  
		}  
	}  
	return bIsWow64;  
}  
