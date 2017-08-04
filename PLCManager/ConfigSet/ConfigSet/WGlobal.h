#pragma once
//using std;

class CStrCode
{
public:
	static BOOL MStrToWStr(CStringW & strCS, std::string & sStr);
	static BOOL WStrToMStr(CStringW & strCS, std::string & sStr);
	static void UTF8ToUnicode(wchar_t* pOut, const char *pText);			// 把UTF-8转换成Unicode
	static void UnicodeToUTF8(char* pOut,const wchar_t* pText);				// Unicode 转换成UTF-8
	static void UnicodeToGB2312(char* pOut,wchar_t uData);					// 把Unicode 转换成 GB2312 
	static void Gb2312ToUnicode(wchar_t* pOut,const char *gbBuffer);		// GB2312 转换成　Unicode
	static void GB2312ToUTF8(std::string& pOut,const char *pText, int pLen);		// GB2312 转为 UTF-8
	static void UTF8ToGB2312(std::string &pOut, const char *pText, int pLen);	// UTF-8 转为 GB2312
	static wchar_t * UTF8ToUnicode(const char* str);
	static unsigned char HChar2UChar(unsigned char c1, unsigned char c2);
};