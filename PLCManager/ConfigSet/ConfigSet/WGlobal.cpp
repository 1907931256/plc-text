#include "Stdafx.h"
#include "WGlobal.h"

unsigned char CStrCode::HChar2UChar(unsigned char c1, unsigned char c2)
{
	unsigned char c1c;
	unsigned char c2c;
	unsigned char cRet = 0;
	if (c1 >= '0' && c1 <= '9')
		c1c = c1 -'0';
	else if (c1 >= 'a' && c1 <= 'f')
		c1c = c1 -'a' + 10;
	else if (c1 >= 'A' && c1 <= 'F')
		c1c = c1 -'A' + 10;
	else
		c1c=0;
	if (c2 >= '0' && c2 <= '9')
		c2c = c2 -'0';
	else if (c2 >= 'a' && c2 <= 'f')
		c2c = c2 -'a' + 10;
	else if (c2 >= 'A' && c2 <= 'F')
		c2c = c2 -'A' + 10;
	else
		c2c =0;
	cRet = (c1c * 16 + c2c);
	return cRet;
}

// 语言转换有关
#define M2W(mStrNull, pWBuf, nWSize) MultiByteToWideChar(CP_ACP, 0, mStrNull, -1, pWBuf, nWSize)//string to wstring
#define W2M(wStrNull, pMBuf, nMSize) WideCharToMultiByte(CP_ACP, 0, wStrNull, -1, pMBuf, nMSize, NULL, NULL)//wstring to string

BOOL CStrCode::WStrToMStr(CStringW & strCS, std::string & sStr)
{
	std::string sRet;
	CStringW strW;
	strW = strCS;
	wchar_t wcTmp[MAX_PATH] = {0};
	wchar_t * wcpTmp = strW.GetBuffer(strW.GetLength() + 1);
	wcscpy_s(&wcTmp[0], sizeof(wcTmp) / sizeof(wcTmp[0]), wcpTmp);
	strW.ReleaseBuffer();
	wcTmp[sizeof(wcTmp) / sizeof(wcTmp[0]) - 1] = 0;
	char cTmp[MAX_PATH * 2] = {0};
	W2M(&wcTmp[0], &cTmp[0], sizeof(cTmp) / sizeof(cTmp[0]));
	cTmp[sizeof(cTmp) / sizeof(cTmp[0]) - 1] = 0;
	sRet = &cTmp[0];
	sStr = sRet;
	return TRUE;
}

BOOL CStrCode::MStrToWStr(CStringW & strCS, std::string & sStr)
{
	std::string sRet;
	CStringW strW;
	strW = strCS;
	wchar_t wcTmp[1024] = {0};
	M2W(sStr.c_str(), &wcTmp[0], sizeof(wcTmp) / sizeof(wcTmp[0]));
	wcTmp[sizeof(wcTmp) / sizeof(wcTmp[0]) - 1] = TCHAR('\0');
	strCS = &wcTmp[0];
	return TRUE;
}
//类实现
void CStrCode::UTF8ToUnicode(wchar_t* pOut,const char *pText)
{
	char* uchar = (char *)pOut;

	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

	return;
}

void CStrCode::UnicodeToUTF8(char* pOut,const wchar_t* pText)
{
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后
	char* pchar = (char *)pText;

	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));

	return;
}

void CStrCode::UnicodeToGB2312(char* pOut,wchar_t uData)
{
	WideCharToMultiByte(CP_ACP,NULL,&uData,1,pOut,sizeof(wchar_t),NULL,NULL);
	return;
}     

void CStrCode::Gb2312ToUnicode(wchar_t* pOut,const char *gbBuffer)
{
	::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1);
	return ;
}

void CStrCode::GB2312ToUTF8(std::string& pOut,const char *pText, int pLen)
{
	char buf[4];
	int nLength = pLen* 3;
	char* rst = new char[nLength + 1];

	memset(buf,0,4);
	memset(rst,0,nLength);

	int i = 0;
	int j = 0;      
	while(i < pLen)
	{
		//如果是英文直接复制就可以
		if( *(pText + i) >= 0)
		{
			rst[j++] = pText[i++];
		}
		else
		{
			wchar_t pbuffer;
			Gb2312ToUnicode(&pbuffer,pText+i);

			UnicodeToUTF8(buf,&pbuffer);

			unsigned short int tmp = 0;
			tmp = rst[j] = buf[0];
			tmp = rst[j+1] = buf[1];
			tmp = rst[j+2] = buf[2];    

			j += 3;
			i += 2;
		}
	}
	rst[j] = '\0';

	//返回结果
	pOut = rst;             
	delete []rst;   

	return;
}

void CStrCode::UTF8ToGB2312(std::string &pOut, const char *pText, int pLen)//pLen不包括'\0'
{
	char * newBuf = new char[pLen + 1];
	char Ctemp[4];
	memset(Ctemp,0,4);

	int i =0;
	int j = 0;

	while(i < pLen)
	{
		if(pText[i] > 0)
		{
			newBuf[j++] = pText[i++];                       
		}
		else                 
		{
			WCHAR Wtemp;
			UTF8ToUnicode(&Wtemp,pText + i);

			UnicodeToGB2312(Ctemp,Wtemp);

			newBuf[j] = Ctemp[0];
			newBuf[j + 1] = Ctemp[1];

			i += 3;    
			j += 2;   
		}
	}
	newBuf[j] = '\0';

	pOut = newBuf;
	delete []newBuf;

	return; 
}  

wchar_t * CStrCode::UTF8ToUnicode( const char* str )
{
	int textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_UTF8, 0, str,-1, NULL,0 ); 
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
	memset(result,0,(textlen+1)*sizeof(wchar_t)); 
	MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen ); 
	return result; 
}