#ifndef __ZTE_HTTP_H__
#define __ZTE_HTTP_H__
/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�DH_HTTP.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ��HTTPЭ��ջ����Э��ջ�����ԭ������չ�Ժá�ͨ���Ժá����ھ���������岻�Ǻ���ȷ��ͷ��ʹ���ַ�������
			�ɾ�����ϲ�Ӧ�ý���Ӧ�ü�������ʹ�á�HTTP���ݷ�Ϊ�����Ӧ�����ࡣ���ڱ�׼��ͷ�����HTTP��RTSP��SIP
			�������˴�����ΪHTTPЭ����ص�����չ�Ժã������ϲ�Ӧ�ú����׽�����չ��Ϊ��֧����չ������parseLine
			��packetLine������չ���µ�Ӧ��Э��ӻ����м̳г�ȥ��ʵ������������������չͷ��������������л�����
			�Ϳ���֧����չͷ��
			�ܶ�ʱ��HTTP PDU�ᱻ������У�������������ʹ��䣬�������������ü������ơ�
			�������Ӧ�������HTTP��һ�����ݲ�ͬ�����Թ����������ʹ�������HTTPCommon���У�HTTPRequest��
			HTTPResponse�����Ǵ����һ�����ݵĲ�ͬ���ѡ�
*
* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��5��28��
* �޶���¼������������RTSP��SIP֧�֡�

*
* ȡ���汾��0.1
* ԭ���ߡ���������
* ������ڣ�2007��5��19��
* �޶���¼������
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
	static HTTPCommon* createPDUFromStream(char* data, int len, int& readlen, int wholeBody = 1);	//wholeBody-�Ƿ���Ҫ���������İ���
};

#endif //head
