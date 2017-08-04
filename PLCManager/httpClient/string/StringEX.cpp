/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：DHString.cpp
* 文件标识：参见配置管理计划书
* 摘　　要：字符串处理类。Java风格。
*
* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年5月28日
* 修订记录：修改部分函数，提高稳定性。

*
* 取代版本：0.1
* 原作者　：李明江
* 完成日期：2006年7月22日
* 修订记录：创建。
*/

//////////////////////////////////////////////////////////////////////

#include "StringEX.h"

//在source中查找ch第一次出现时的索引	
int String::indexOf(const char* source, char ch)
{
	//return strchr(source, ch);
	assert(source != NULL);

	for(unsigned int i=0; i<strlen(source); i++)
		if (source[i] == ch)
			return i;
	return -1;
}

int String::indexOf(int num, const char* source, char ch)
{
	assert(source != NULL);

	int count = 1;
	for(unsigned int i=0; i<strlen(source); i++)
	{
		if (source[i] == ch)
		{
			if (count++ == num)
				return i;
		}
	}
	return -1;
}

//在source中查找字符串str第一次出现时的索引
int String::indexOf(const char* source, const char* str)
{
	assert(source != NULL && str != NULL);
//	return String::indexOf(0, source, str);
	char* pos = strstr((char*)source, str);
	if (pos != NULL)
		return pos - source;
	else
		return -1;
	//return strcspn(source, str);
}

//在source中查找start＋１次str出现的索引
int String::indexOf(int start, const char* source, const char* str)
{
	assert(start >= 0);
	assert(source != NULL && str != NULL);

	if(strlen(str) > strlen(source))
		return -3;

	char* newchar = (char*)source;
	int len = 0;
	for (int i=0; i<start; i++)
	{
		char* p = strstr(newchar, str);
		if (p == NULL)
			return -1;

		len += p - newchar + strlen(str);
		newchar = p + strlen(str);	
	}

	return len - strlen(str);
}

//将str转化成大写串
void String::toUpperCase(char* str)
{
	assert(str != NULL);

	for (unsigned int i=0; i<strlen(str); i++)
	{
		if (str[i] > 0x60 && str[i] < 0x7B)
		{
			str[i] = (char)(str[i] - 0x20);
		}
	}
}

//将str转化成小写串
void String::toLowerCase(char* str)
{
	assert(str != NULL);

	for (unsigned int i=0; i<strlen(str); i++)
	{
		if (str[i] > 0x40 && str[i] < 0x5B)
		{
			str[i] = (char)(str[i] + 0x20);
		}
	}
}

//取出指定位置的子串
char* String::subString(const char* source, int start, int end, char* buf,size_t bufLen)
{
	assert(source != NULL && buf != NULL);

	//memset(buf, 0, sizeof(buf));

	unsigned n = end - start; 
	AX_OS::strncpy(buf, bufLen, source + start, n); 
	return buf;
}

//比较两个串是否相等，大小写敏感
int String::equals(const char* str1, const char* str2)
{
	assert(str1 != NULL && str2 !=NULL);
	return strcmp(str1, str2);
}

//忽略大小写而进行比较
int String::equalsIgnoreCase(const char* str1, const char* str2)
{
	assert(str1 != NULL && str2 !=NULL);
	//return stricmp(str1, str2);
	return 0;
}

//将字符串形式的数值转换成数据型数据
int String::str2int(const char* str)
{
	assert(str != NULL);

	int mul=1, value=0;
	for(unsigned int i=1; i<strlen(str); i++)
		mul*=10;

	for(unsigned int j=0; j<strlen(str); j++)
	{
		value += (str[j] - 48) * mul;
		mul/=10;
	}
	return value;
}

//读取seperate分隔的第一个子串
char* String::readWord(const char* src, char seperate)
{
	assert(src != NULL);

	static char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	
	int index = String::indexOf (src, seperate);
	int eIndex = String::indexOf(src, "\r\n");
	if (index > eIndex)
		index = eIndex;
	if(index == -1)
	{
		int end = String::indexOf(src, "\r\n");
		if (end > 0)
			AX_OS::strncpy(tmp, sizeof(tmp),src, end);
		else
			AX_OS::strncpy(tmp, sizeof(tmp),src, strlen(src));
	}
	else
		AX_OS::strncpy(tmp, sizeof(tmp),src, index);
	return tmp;
 }

int String::readWord(const char* src, char seperate, char* des,size_t desLen)
{
	assert(src != NULL && des != NULL);

	int index = String::indexOf (src, seperate);
	int eIndex = String::indexOf(src, "\r\n");
	if (index > eIndex)
		index = eIndex;
	if(index == -1)
	{
		int end = String::indexOf(src, "\r\n");
		if (end > 0)
			AX_OS::strncpy(des, desLen,src, end);
		else
			AX_OS::strncpy(des, desLen,src, strlen(src));
	}
	else
		AX_OS::strncpy(des, desLen,src, index);

	return 0;
}


//读取seperate分隔的第index+1个子串
char* String::readWord(const char* src, char seperate, int num)
{
	assert(src != NULL);
	assert(num >= 0);

	return String::readWord(src + String::indexOf(num - 1, src, seperate) + 1, seperate);
}

int String::readWord(const char* src, char seperate, int num, char* des,size_t desLen)
{
	assert(src != NULL && des != NULL);
	assert(num >= 0);

	return String::readWord(src + String::indexOf(num - 1, src, seperate) + 1, seperate, des,desLen);
}

//读取src中seperate后的子串
char* String::readValue(const char* src, char seperate)
{
	assert(src != NULL);

	static char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	int index = String::indexOf(src, seperate);
	//考虑能否找到分隔符， gaowei 08-04-15
	if ( index >= 0 )
	{
		AX_OS::strncpy(tmp, sizeof(tmp), src + index + 1, strlen(src) - index);
	}
	return tmp;
}

int String::readValue(const char* src, char seperate, char* des,size_t desLen)
{
	assert(src != NULL && des != NULL);

	int index = String::indexOf(src, seperate);
	//考虑能否找到分隔符， gaowei 08-04-15
	if ( index >= 0 )
	{
		AX_OS::strncpy(des, desLen,src + index + 1, strlen(src) - index);
	}
	return 0;
}

//读取src中seperate前的子串
char* String::readName(const char* src, char seperate)
{
	assert(src != NULL);

	static char tmp[64];
	memset(tmp, 0, sizeof(tmp));
	//考虑能否找到分隔符， gaowei 08-04-15
	int index = String::indexOf(src, seperate);
	if ( index >= 0 )
	{
		AX_OS::strncpy(tmp, sizeof(tmp), src, index);
	}
	
	return tmp;
}

int String::readName(const char* src, char seperate, char* des,size_t desLen)
{
	assert(src != NULL && des != NULL);

	//考虑能否找到分隔符， gaowei 08-04-15
	int index = String::indexOf(src, seperate);
	if ( index >= 0 )
	{
		AX_OS::strncpy(des, desLen,src, index);
	}

	return 0;
}

//读取data中的Http头部子串
int String::getHttpHead(const char* data, char* buf,size_t bufLen)
{
	assert(data != NULL && buf != NULL);

	int end = String::indexOf(data, "\r\n\r\n");
	//考虑头部不全，gaowei 08-04-15
	if ( end > 0 )
	{
		subString(data, 0, end+4, buf,bufLen);
	}
	else
	{
		return -1;
	}

	return 0;
}

int String::getHttpBody(const char* data, int len, char* buf)
{
	assert(data != NULL && buf != NULL);

	int begin = String::indexOf(data, "\r\n\r\n");
	//考虑能否找到分隔符， gaowei 08-04-15
	if ( begin < 0 )
	{
		return -1;
	}

	int j=0;

	int nLen = len - begin - 4;

	for(int i=begin+4; i<nLen; i++)
		buf[j++] = data[i];

	buf[j] = 0;
	return nLen;
}

char* String::trim(char* pszSource)
{
   //字符串首指针 
   char *pszHead = pszSource; 
   //用于保存最后非空格类字符的位置的指针 
   char *pszLast = &pszSource[strlen(pszSource)-1]; 
   //查找最后非空格指针的位置 
   while (isSpaces(*pszLast)) 
      --pszLast; 
   *(++pszLast) = '\0'; 
   //查找首个非空格类字符的位置，以便左移字符串 
   for ( ; (*pszHead != '\0'); ++pszHead) 
   { 
      if (! isSpaces(*pszHead)) 
      { 
         break; 
      } 
   } 
   if ( pszSource != pszHead )
      strcpy(pszSource, pszHead); 

   return pszSource; 
}

int String::wholeLen(const char* src)
{
	assert(src != NULL);

	int i = 0;
	while(1)
	{
		if(src[i] == 0)
			return i;
		i++;
	}
}

