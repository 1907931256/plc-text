#pragma once
//using std;

class CStrCode
{
public:
	static BOOL MStrToWStr(CStringW & strCS, std::string & sStr);
	static BOOL WStrToMStr(CStringW & strCS, std::string & sStr);
	static void UTF8ToUnicode(wchar_t* pOut, const char *pText);			// ��UTF-8ת����Unicode
	static void UnicodeToUTF8(char* pOut,const wchar_t* pText);				// Unicode ת����UTF-8
	static void UnicodeToGB2312(char* pOut,wchar_t uData);					// ��Unicode ת���� GB2312 
	static void Gb2312ToUnicode(wchar_t* pOut,const char *gbBuffer);		// GB2312 ת���ɡ�Unicode
	static void GB2312ToUTF8(std::string& pOut,const char *pText, int pLen);		// GB2312 תΪ UTF-8
	static void UTF8ToGB2312(std::string &pOut, const char *pText, int pLen);	// UTF-8 תΪ GB2312
	static wchar_t * UTF8ToUnicode(const char* str);
	static unsigned char HChar2UChar(unsigned char c1, unsigned char c2);
};