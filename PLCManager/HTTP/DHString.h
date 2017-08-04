/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：DHString.h
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

//字符串处理方法类。
class String  
{
public:
	//取出ch在source中第一个索引
	static int indexOf(const char* source, char ch);

	//取出ch字符在source中第num次索引
	static int indexOf(int num, const char* source, char ch);

	//字符串str在source中第一次出现时的索引
	static int indexOf(const char* source, const char* str);

	//字符串str在source中第num次出现时的索引
	static int indexOf(int num, const char* source, const char* str);

	//将str字符串转化成大写
	static void toUpperCase(char* str);

	//将str字符串转化成小写
	static void toLowerCase(char* str);

	//取出指定位置的子串
	static char* subString(const char* source, int start, int end, char* buf,size_t bufLen);

	//比较两个串是否相等，大小写敏感
	static int equals(const char* str1, const char* str2);

	//比较两个串是否相等，忽略大小写
	static int equalsIgnoreCase(const char* str1, const char* str2);

	//将字符串形式的数值转换成数据型
	static int str2int(const char* str);
	
	//读取seperate分隔的第一个子串
	static char* readWord(const char* src, char seperate);
	static int readWord(const char* src, char seperate, char* des,size_t desLen);	//多线程情况下请调用这个接口

	//读取seperate分隔的第num+1个子串
	static char* readWord(const char* src, char seperate, int num);
	static int readWord(const char* src, char seperate, int num, char* des,size_t desLen);	//多线程情况下请调用这个接口

	//读取src中seperate后的子串
	static char* readValue(const char* src, char seperate);
	static int readValue(const char* src, char seperate, char* des,size_t desLen);	//多线程情况下请调用这个接口

	//读取src中seperate前的子串
	static char* readName(const char* src, char seperate);
	static int readName(const char* src, char seperate, char* des,size_t desLen);		//多线程情况下请调用这个接口

	//去除前后空格
	static char* trim(char* pszSource);
	
	//读取data中的Http头部子串
	static int getHttpHead(const char* data, char* buf,size_t bufLen);

	//读取Data中的Http包体子串
	/*
	  如果HTTP包体中携带二进制数据，就需要指定数据长度，并返回包体长度。
	  len：请求数据总长
	  return：包体数据的长度
	*/
	static int getHttpBody(const char* data, int len, char* buf);

	static int wholeLen(const char* src);
};

#endif // !defined(AFX_STRING_H__FE5FC6FD_DD45_45D7_903D_B9B9D881F8C6__INCLUDED_)
