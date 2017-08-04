/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�DHString.cpp
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ���ַ��������ࡣJava���
*
* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��5��28��
* �޶���¼���޸Ĳ��ֺ���������ȶ��ԡ�

*
* ȡ���汾��0.1
* ԭ���ߡ���������
* ������ڣ�2006��7��22��
* �޶���¼��������
*/

//////////////////////////////////////////////////////////////////////

#include "StringEX.h"

//��source�в���ch��һ�γ���ʱ������	
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

//��source�в����ַ���str��һ�γ���ʱ������
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

//��source�в���start������str���ֵ�����
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

//��strת���ɴ�д��
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

//��strת����Сд��
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

//ȡ��ָ��λ�õ��Ӵ�
char* String::subString(const char* source, int start, int end, char* buf,size_t bufLen)
{
	assert(source != NULL && buf != NULL);

	//memset(buf, 0, sizeof(buf));

	unsigned n = end - start; 
	AX_OS::strncpy(buf, bufLen, source + start, n); 
	return buf;
}

//�Ƚ��������Ƿ���ȣ���Сд����
int String::equals(const char* str1, const char* str2)
{
	assert(str1 != NULL && str2 !=NULL);
	return strcmp(str1, str2);
}

//���Դ�Сд�����бȽ�
int String::equalsIgnoreCase(const char* str1, const char* str2)
{
	assert(str1 != NULL && str2 !=NULL);
	//return stricmp(str1, str2);
	return 0;
}

//���ַ�����ʽ����ֵת��������������
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

//��ȡseperate�ָ��ĵ�һ���Ӵ�
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


//��ȡseperate�ָ��ĵ�index+1���Ӵ�
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

//��ȡsrc��seperate����Ӵ�
char* String::readValue(const char* src, char seperate)
{
	assert(src != NULL);

	static char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	int index = String::indexOf(src, seperate);
	//�����ܷ��ҵ��ָ����� gaowei 08-04-15
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
	//�����ܷ��ҵ��ָ����� gaowei 08-04-15
	if ( index >= 0 )
	{
		AX_OS::strncpy(des, desLen,src + index + 1, strlen(src) - index);
	}
	return 0;
}

//��ȡsrc��seperateǰ���Ӵ�
char* String::readName(const char* src, char seperate)
{
	assert(src != NULL);

	static char tmp[64];
	memset(tmp, 0, sizeof(tmp));
	//�����ܷ��ҵ��ָ����� gaowei 08-04-15
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

	//�����ܷ��ҵ��ָ����� gaowei 08-04-15
	int index = String::indexOf(src, seperate);
	if ( index >= 0 )
	{
		AX_OS::strncpy(des, desLen,src, index);
	}

	return 0;
}

//��ȡdata�е�Httpͷ���Ӵ�
int String::getHttpHead(const char* data, char* buf,size_t bufLen)
{
	assert(data != NULL && buf != NULL);

	int end = String::indexOf(data, "\r\n\r\n");
	//����ͷ����ȫ��gaowei 08-04-15
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
	//�����ܷ��ҵ��ָ����� gaowei 08-04-15
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
   //�ַ�����ָ�� 
   char *pszHead = pszSource; 
   //���ڱ������ǿո����ַ���λ�õ�ָ�� 
   char *pszLast = &pszSource[strlen(pszSource)-1]; 
   //�������ǿո�ָ���λ�� 
   while (isSpaces(*pszLast)) 
      --pszLast; 
   *(++pszLast) = '\0'; 
   //�����׸��ǿո����ַ���λ�ã��Ա������ַ��� 
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

