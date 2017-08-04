#pragma once
#include "Global.h"

void SetStrCat(std::string &strOut, std::string strIn, int nValue)
{
	char szTemp[128] = {0};
	_snprintf(szTemp, 127, "%s_%d", strIn.c_str(), nValue);
	strOut.append(szTemp);
}

void SetValue(std::string &sRet, const char *szKey, const char *szValue, bool bEnd)
{
	char szTemp[128] = {0};
	_snprintf(szTemp, 127, "\"%s\":\"%s\"", szKey, szValue);
	sRet.append(szTemp);
	if (!bEnd)
	{
		sRet.append(",");
	}
}

void SetValue(std::string &sRet, const char *szKey, int nValue, bool bEnd)
{
	char szTemp[128] = {0};
	_snprintf(szTemp, 127, "\"%s\":%d", szKey, nValue);
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

//void split(std::string& s, std::string& delim,std::vector< std::string >* ret)
//{
//	size_t last = 0;
//	size_t index=s.find_first_of(delim,last);
//	while (index!=std::string::npos)
//	{
//		ret->push_back(s.substr(last,index-last));
//		last=index+1;
//		index=s.find_first_of(delim,last);
//	}
//	if (index-last>0)
//	{
//		ret->push_back(s.substr(last,index-last));
//	}
//}