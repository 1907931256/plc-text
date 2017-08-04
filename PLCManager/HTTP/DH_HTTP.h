#ifndef __ZTE_HTTP_H__
#define __ZTE_HTTP_H__
/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：DH_HTTP.h
* 文件标识：参见配置管理计划书
* 摘　　要：HTTP协议栈。本协议栈的设计原则是扩展性好、通用性好。对于绝大多数意义不是很明确的头域使用字符串处理
			由具体的上层应用进行应用级解析和使用。HTTP数据分为请求和应答两类。对于标准的头域包括HTTP、RTSP、SIP
			都进行了处理。因为HTTP协议的特点是扩展性好，所以上层应用很容易进行扩展，为了支持扩展定义了parseLine
			和packetLine进行扩展。新的应用协议从基类中继承出去，实现这两个方法，对扩展头域进行正反向序列化处理
			就可以支持扩展头域。
			很多时候HTTP PDU会被放入队列，经过几道保存和传输，所以引入了引用计数机制。
			请求类和应用类仅是HTTP第一行数据不同，所以公共处理代码和处理框架在HTTPCommon类中，HTTPRequest和
			HTTPResponse仅仅是处理第一行数据的不同而已。
*
* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年5月28日
* 修订记录：增加完整的RTSP和SIP支持。

*
* 取代版本：0.1
* 原作者　：李明江
* 完成日期：2007年5月19日
* 修订记录：创建
*/

//#include "StdAfx.h"
#include "HttpProtocol.h"
#include "DHString.h"

//#include <IAddRefAble.h>
#include <AX_IAddRefAble.h>
#include <string>
#ifdef  WIN32
#pragma warning(disable:4996) 
#endif

//class HTTPCommon:public IAddRefAble
class HTTPCommon:public AX_IAddRefAble
{
public:
	HTTPCommon(void);

	virtual ~HTTPCommon(void);

	int fromStream(char * data);
	char* toStream(void);

	int fromStream(char* data, int len);
	char* toStream(int &len);

	char* getBody(void);
	int	  getBodyLen(void);
	char* getBody(int &len);

	int setBody(const char* body, int len);

	int SetStreamContent(char *pData, int nLen);
	int appendContent(char* data, int len);

	char* getString(void);
	int getLength();

	char* getStream(int &len);

	int getType(void);	//Request or Response
	int getProtocol(void);	//SIP or HTTP
	void setProtocol(PROTOCOL_TYPE type);

	void reset(void);

	int parseCommon(const char* data);
	int packetCommon(void);

public:
	int method;
	char from[64];
	char to[64];
	char cseq[64];
	char callID[64];
	int maxForwards;
	char contact[64];
	int contentType;
	int contentLength;
	char url[256];
	char via[128];
	char contentTypeString[64];

	char accept[128];
	char userAgent[64];
	char host[64];
	char xClientAddress[64];
	char xTransactionID[64];
	char setCookie[64];
	char date[64];
	char server[64];
	char cookie[64];
	char acceptEncoding[64];
	char acceptLanguage[64];
	char allow[64];
	int bandwidth;
	int blockSize;
	char scale[64];
	char speed[64];
	char conference[64];
	char connection[64];
	char contentBase[64];
	char contentEncoding[64];
	char contentLanguage[64];
	char range[64];
	char rtpInfo[64];
	char session[64];
	char timestamp[64];
	char transport[128];
	char wwwAuthenticate[256];
	char Authorization[300];
	char unsupport[64];
	char vary[64];
	char expires[64];
	char lastModified[64];

	/*Begin: Added by yehao 2007-09-06*/
	char _x_Cache_Control[32];
	char _x_Accept_Retransmit[32];
	char _x_Accept_Dynamic_Rate[4];
	char _x_Dynamic_Rate[16];
	/*End: yehao 2007-09-06*/

	//rtsp head fields
	char rtsp_public[64];
	int rtsp_token;
	string data_param2;

protected:
	char *_body;
	unsigned _maxBodySize;
	char * _buffer;
	unsigned _maxBufferSize;
	int _bodyLen;
	int _buffLen;
	int _reqOrRep;
	int _protocol;
protected:
	virtual int parseHead(const char* data) = 0;
	virtual int packetHead(void) = 0;
	
	virtual int parseLine(char* data);
	virtual int packetLine(void);
};

/////////////////////////////////////////////////////////////
class HTTPRequest : public HTTPCommon
{
public:
	HTTPRequest(void);
	virtual ~HTTPRequest(void);

private:
	int parseHead(const char* data);
	int packetHead(void);
};

/////////////////////////////////////////////////////////////
class HTTPResponse : public HTTPCommon
{
public:
	HTTPResponse(void);
	virtual ~HTTPResponse(void);
public:
	int result;
	char message[64];
private:
	int parseHead(const char* data);
	int packetHead(void);
};

////////////////////////////////////////////////////////////////////
class HTTPFactory
{
public:
	static HTTPCommon* createPDUFromStream(char* data);
	static HTTPCommon* createPDUFromStream(char* data, int len, int& readlen, int wholeBody = 1);	//wholeBody-是否需要返回完整的包体
};

#endif //head
