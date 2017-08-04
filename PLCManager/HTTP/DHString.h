/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�DHString.h
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

#if !defined(AFX_STRING_H__FE5FC6FD_DD45_45D7_903D_B9B9D881F8C6__INCLUDED_)
#define AFX_STRING_H__FE5FC6FD_DD45_45D7_903D_B9B9D881F8C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef  WIN32
#pragma warning(disable:4996) 
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define isSpaces(ch) (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\b' || ch == '\f' || ch == '\n') 

//�ַ����������ࡣ
class String  
{
public:
	//ȡ��ch��source�е�һ������
	static int indexOf(const char* source, char ch);

	//ȡ��ch�ַ���source�е�num������
	static int indexOf(int num, const char* source, char ch);

	//�ַ���str��source�е�һ�γ���ʱ������
	static int indexOf(const char* source, const char* str);

	//�ַ���str��source�е�num�γ���ʱ������
	static int indexOf(int num, const char* source, const char* str);

	//��str�ַ���ת���ɴ�д
	static void toUpperCase(char* str);

	//��str�ַ���ת����Сд
	static void toLowerCase(char* str);

	//ȡ��ָ��λ�õ��Ӵ�
	static char* subString(const char* source, int start, int end, char* buf,size_t bufLen);

	//�Ƚ��������Ƿ���ȣ���Сд����
	static int equals(const char* str1, const char* str2);

	//�Ƚ��������Ƿ���ȣ����Դ�Сд
	static int equalsIgnoreCase(const char* str1, const char* str2);

	//���ַ�����ʽ����ֵת����������
	static int str2int(const char* str);
	
	//��ȡseperate�ָ��ĵ�һ���Ӵ�
	static char* readWord(const char* src, char seperate);
	static int readWord(const char* src, char seperate, char* des,size_t desLen);	//���߳���������������ӿ�

	//��ȡseperate�ָ��ĵ�num+1���Ӵ�
	static char* readWord(const char* src, char seperate, int num);
	static int readWord(const char* src, char seperate, int num, char* des,size_t desLen);	//���߳���������������ӿ�

	//��ȡsrc��seperate����Ӵ�
	static char* readValue(const char* src, char seperate);
	static int readValue(const char* src, char seperate, char* des,size_t desLen);	//���߳���������������ӿ�

	//��ȡsrc��seperateǰ���Ӵ�
	static char* readName(const char* src, char seperate);
	static int readName(const char* src, char seperate, char* des,size_t desLen);		//���߳���������������ӿ�

	//ȥ��ǰ��ո�
	static char* trim(char* pszSource);
	
	//��ȡdata�е�Httpͷ���Ӵ�
	static int getHttpHead(const char* data, char* buf,size_t bufLen);

	//��ȡData�е�Http�����Ӵ�
	/*
	  ���HTTP������Я�����������ݣ�����Ҫָ�����ݳ��ȣ������ذ��峤�ȡ�
	  len�����������ܳ�
	  return���������ݵĳ���
	*/
	static int getHttpBody(const char* data, int len, char* buf);

	static int wholeLen(const char* src);
};

#endif // !defined(AFX_STRING_H__FE5FC6FD_DD45_45D7_903D_B9B9D881F8C6__INCLUDED_)
