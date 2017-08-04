/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：DH_HTTP.cpp
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

#include "DH_HTTP.h"
#include "StringEX.h"
#include "string_util.h"

////Macro Sector
#define PARSE_HEAD_FIELD(title, value, var, keyWord)			\
		else if (strcmp(title, keyWord) == 0) \
		{												\
		strncpy(var, value, sizeof(var));				\
		}

#define PARSE_HEAD_FIELD_IGNORE_CASE(title, value, var, keyWord)			\
        else if (base::strcasecmp(title, keyWord) == 0) \
		{												\
		strncpy(var, value, sizeof(var));				\
		}	

#define PARSE_HEAD_FIELD_INT(title, value, var, keyWord)		\
		else if (strcmp(title, keyWord) == 0) \
		{												\
		var = String::str2int(value);\
		}

#define PACKET_HEAD_FIELD_STR(var, title)		\
	if (strlen(var) > 0)						\
	{											\
	memset(str, 0, sizeof(str));			\
    base::snprintf(str,sizeof(str),"%s: %s\r\n", title, var);	\
	strncat(_buffer, str,LEN_HTTP_BUFFER);					\
	}

#define PACKET_HEAD_FIELD_INT(var, title)		\
	if (var > 0)						\
	{											\
	memset(str, 0, sizeof(str));			\
    base::snprintf(str,sizeof(str),"%s: %d\r\n", title, var);	\
	strncat(_buffer, str,LEN_HTTP_BUFFER);					\
	}

HTTPCommon::HTTPCommon(const std::string& objectName)
{
	_body = NULL;
	_buffer = NULL;
	name = objectName;
	reset();

	//printf("%s:create!\n",name.c_str());
}

HTTPCommon::~HTTPCommon(void)
{
	//printf("%s:released!\n",name.c_str());
	if (_body)
	{
		delete[] _body;
		_body = NULL;
	}
	if (_buffer)
	{
		delete[] _buffer;
		_buffer = NULL;
	}
}

void HTTPCommon::reset(void)
{
	method = -1;
	memset(from, 0, sizeof(from));
	memset(to, 0, sizeof(to));
	memset(callID, 0, sizeof(callID));
	memset(contact, 0, sizeof(contact));
	memset(url, 0, sizeof(url));
	memset(via, 0, sizeof(via));
	memset(cseq, 0, sizeof(cseq));
	memset(accept, 0, sizeof(accept));
	memset(userAgent, 0, sizeof(userAgent));
	memset(host, 0, sizeof(host));
	memset(xClientAddress, 0, sizeof(xClientAddress));
	memset(xTransactionID, 0, sizeof(xTransactionID));
	memset(setCookie, 0, sizeof(setCookie));
	memset(cookie, 0, sizeof(cookie));
	memset(date, 0, sizeof(date));
	memset(server, 0, sizeof(server));
	memset(acceptEncoding, 0, sizeof(acceptEncoding));
	memset(acceptLanguage, 0, sizeof(acceptLanguage));
	memset(allow, 0, sizeof(allow));
	memset(conference, 0, sizeof(conference));
	memset(connection, 0, sizeof(connection));
	memset(contentBase, 0, sizeof(contentBase));
	memset(contentEncoding, 0, sizeof(contentEncoding));
	memset(contentLanguage, 0, sizeof(contentLanguage));
	memset(range, 0, sizeof(range));
	memset(rtpInfo, 0, sizeof(rtpInfo));
	memset(session, 0, sizeof(session));
	memset(timestamp, 0, sizeof(timestamp));
	memset(transport, 0, sizeof(transport));
	memset(wwwAuthenticate, 0, sizeof(wwwAuthenticate));
	memset(Authorization, 0, sizeof(Authorization));
	memset(scale, 0, sizeof(scale));
	memset(speed, 0, sizeof(speed));
	memset(unsupport, 0, sizeof(unsupport));
	memset(vary, 0, sizeof(vary));
	memset(lastModified, 0, sizeof(lastModified));
	memset(expires, 0, sizeof(expires));
	memset(contentTypeString, 0, sizeof(contentTypeString));
	/*Begin: Add by yehao 2007-09-06*/
	memset(_x_Cache_Control, 0, sizeof(_x_Cache_Control));
	memset(_x_Accept_Retransmit, 0, sizeof(_x_Accept_Retransmit));
	memset(_x_Accept_Dynamic_Rate, 0, sizeof(_x_Accept_Dynamic_Rate));
	memset(_x_Dynamic_Rate, 0, sizeof(_x_Dynamic_Rate));
	/*End: yehao 2007-09-06*/
	//rtsp head fields
	memset(rtsp_public, 0, sizeof(rtsp_public));
	rtsp_token = 0;
	data_param2 = "";

	contentLength = 0;
	contentType = -1;
	maxForwards = -1;
	blockSize = -1;
	bandwidth = -1;

	//memset(_body, 0, _bodyLen);
	//memset(_buffer, 0, sizeof(_buffer));

	_protocol = PROTOCOL_HTTP;
	_bodyLen = 0;
	_buffLen = 0;

	if (_body)
	{
		delete[]_body;
		_body = NULL;
	}

	if (_buffer)
	{
		delete[] _buffer;
		_buffer = NULL;
	}
}

int HTTPCommon::setBody(const char* body, int len)
{
	assert(body != NULL);
	if (body == NULL)
		return -2;
	if (_body == NULL)
	{
		_body = new char[LEN_HTTP_BODY];
	}
	if(len > LEN_HTTP_BODY)
		return -1;

	memset(_body, 0, LEN_HTTP_BODY);
	memcpy(_body, body, len);
	_bodyLen = len;
	return 0;
}

int HTTPCommon::appendContent(char* data, int len)
{
	//防止操作越界，gaowei 08-04-15
	if ( _bodyLen + len > LEN_HTTP_BODY )
	{
		return -1;
	}

	if (_body == NULL)
	{
		_body = new char[LEN_HTTP_BODY];
	}

	memcpy(_body + _bodyLen, data, len);
	_bodyLen += len;
	return _bodyLen;
}

int HTTPCommon::SetStreamContent(char *pData, int nLen)
{
	contentLength = nLen;
	contentType = CONTENT_TYPE_OCTET;
	return setBody(pData, nLen);
}

char* HTTPCommon::getBody(void)
{
	return _body;
}

int HTTPCommon::getBodyLen(void)
{
	return _bodyLen;
}

char* HTTPCommon::getBody(int &len)
{
	len = _bodyLen;
	return _body;
}

char* HTTPCommon::getString(void)
{
	return _buffer;
}

char* HTTPCommon::getStream(int &len)
{
	len = _buffLen;
	return _buffer;
}
int HTTPCommon::getLength()
{
	return _buffLen;
}

int HTTPCommon::getType(void)
{
	return _reqOrRep;
}

int HTTPCommon::getProtocol(void)
{
	return _protocol;
}

void HTTPCommon::setProtocol(PROTOCOL_TYPE type)
{
	_protocol = type;
}

int HTTPCommon::parseLine(char* data)
{
	return 0;
}

int HTTPCommon::packetLine(void)
{
	return 0;
}

int HTTPCommon::fromStream(char* data)
{
	//先取出包头和包体，包头调用parseHead解析，包体直接放在_body中了。
	assert(data != NULL);
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	//头部不全，应该认为解析失败，gaowei 08-04-15
	int ret = String::getHttpHead(data, buf,sizeof(buf));
	if ( ret < -1 )
	{
		return HTTP_ERROR_BASE;
	}

	//忽略大小写，所以，先把接收到的头全部转换为大写
//	String::toUpperCase(buf);
	int error = parseHead(buf);
	
	//构造HTTP包体
	if (error == HTTP_SUCCESS)
	{
		if (contentLength > 0)
		{

			//防止拷贝越界
			//if ( _bodyLen > LEN_HTTP_BODY )
			//{
			//	_bodyLen = LEN_HTTP_BODY;
			//}
			if (_body == NULL)
			{
				_body = new char[contentLength+1];
				memset(_body,0,contentLength+1);
			}
			else if (contentLength>_bodyLen)
			{
				delete[] _body;
				_body = new char[contentLength+1];
				memset(_body,0,contentLength+1);
			}
			_bodyLen = contentLength;


			memcpy(_body, data + String::indexOf(data, "\r\n\r\n") + 4, _bodyLen);
			_body[_bodyLen]=0;
		}
		//String::getHttpBody(data, _body);
	}

	return error;
}

int HTTPCommon::fromStream(char* data, int len)
{
	assert(data != NULL);
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	int ret = String::getHttpHead(data, buf,sizeof(buf));
	if ( ret < 0 )
	{
		return HTTP_ERROR_BASE;
	}

	int error = parseHead(buf);

	//构造HTTP包体
	if (error == HTTP_SUCCESS)
	{
		int bodyIndex = String::indexOf(data, "\r\n\r\n") + 4;
		ret = bodyIndex;
		if (contentLength > 0)
		{
			//确定可以拷贝的数据长度 
			int bodyLen = len - bodyIndex > contentLength ? contentLength : len - bodyIndex;       

			//if ( _bodyLen > LEN_HTTP_BODY )
			//{
			//	_bodyLen = LEN_HTTP_BODY;
			//}
			if (_body == NULL)
			{
				_body = new char[bodyLen+1];
				memset(_body,0,bodyLen+1);
			}
			else if (bodyLen>_bodyLen)
			{
				delete[] _body;
				_body = new char[bodyLen+1];
				memset(_body,0,bodyLen+1);
			}

			_bodyLen = bodyLen;
			 
			memcpy(_body, data + bodyIndex, _bodyLen);

			ret += _bodyLen;
			/*
			_bodyLen = len - (String::indexOf(data, "\r\n\r\n") + 4);
			memcpy(_body, data + String::indexOf(data, "\r\n\r\n") + 4, _bodyLen);
			_body[_bodyLen]=0;
			*/
		}
	}
	else
	{
		ret = -1;
	}

	return ret; //返回解析的长度
}

int HTTPCommon::parseCommon(const char* data)
{
	int index = 0;
	int dataLen = String::indexOf(data, "\r\n\r\n");
	if (dataLen == -1)
		dataLen = strlen(data);
	
	//取其它HTTP头域
	while (1)
	{		
		//将指针指向下一个头域
		int iNext = String::indexOf(data + index, "\r\n");
		if (iNext == -1)
			break;
		else
			index += iNext;

		if (index == dataLen)
			break;
		//将指针向后移动，减小处理范围。移动后，取第一个段就行，这样效率最高
		//因为indexOf包括\r\n，所以，需要加2以去除
		char pTemp[256] = {0};
		char pTitle[128] = {0};	//':'之前的子串
		char pValue[128] = {0};	//':'之后的子串

		String::readWord(data + index + 2, '\r', 1, pTemp,sizeof(pTemp));
		String::trim(pTemp);

		String::readName(pTemp, ':', pTitle,sizeof(pTitle));
		String::trim(pTitle);

		String::readValue(pTemp, ':', pValue,sizeof(pValue));
		String::trim(pValue);

		if (strcmp(pTitle, STR_ACCEPT) ==0)
		{
			strncpy(accept, pValue, sizeof(accept));
		}
		else if (strcmp(pTitle, STR_CONTENT_TYPE) ==0)
		{
			if (strcmp(pValue, STR_CONTENT_TYPE_XML) == 0)
			{
				contentType = CONTENT_TYPE_XML;
			}
			else if(strcmp(pValue, STR_CONTENT_TYPE_SDP) == 0)
			{
				contentType = CONTENT_TYPE_SDP;
			}
			else if(strcmp(pValue, STR_CONTENT_TYPE_HTTP) == 0)
			{
				contentType = CONTENT_TYPE_HTTP;
			}
			else if(strcmp(pValue, STR_CONTENT_TYPE_HTML) == 0)
			{
				contentType = CONTENT_TYPE_HTML;
			}
			else if(strcmp(pValue, STR_CONTENT_TYPE_OCTET) == 0)
			{
				contentType = CONTENT_TYPE_OCTET;
			}			
			else 
			{
				contentType = -1;
			}
			strncpy(contentTypeString, pValue, sizeof(contentTypeString));
		}
		PARSE_HEAD_FIELD(pTitle, pValue, via, STR_VIA)

		PARSE_HEAD_FIELD_INT(pTitle, pValue, contentLength, STR_CONTENT_LENGTH)

		PARSE_HEAD_FIELD(pTitle, pValue, from, STR_FROM)
		PARSE_HEAD_FIELD(pTitle, pValue, to, STR_TO)
		PARSE_HEAD_FIELD(pTitle, pValue, callID, STR_CALLID)

		//modified by zsj, xm的设备的这个字段是全小写的！
		//PARSE_HEAD_FIELD(pTitle, pValue, cseq, STR_CSEQ)
		PARSE_HEAD_FIELD_IGNORE_CASE(pTitle, pValue, cseq, STR_CSEQ)

		PARSE_HEAD_FIELD_INT(pTitle, pValue, maxForwards, STR_MAX_FORWARDS)

		PARSE_HEAD_FIELD(pTitle, pValue, userAgent, STR_USER_AGENT)
		PARSE_HEAD_FIELD(pTitle, pValue, host, STR_HOST)
		PARSE_HEAD_FIELD(pTitle, pValue, xClientAddress, STR_X_CLIENT_ADDRESS)
		PARSE_HEAD_FIELD(pTitle, pValue, xTransactionID, STR_X_TRANSACTION_ID)
		PARSE_HEAD_FIELD(pTitle, pValue, setCookie, STR_SET_COOKIE)
		PARSE_HEAD_FIELD(pTitle, pValue, cookie, STR_COOKIE)
		PARSE_HEAD_FIELD(pTitle, pValue, date, STR_DATE)
		PARSE_HEAD_FIELD(pTitle, pValue, server, STR_SERVER)
		PARSE_HEAD_FIELD(pTitle, pValue, acceptEncoding, STR_ACCEPT_ENCODING)
		PARSE_HEAD_FIELD(pTitle, pValue, acceptLanguage, STR_ACCEPT_LANGUAGE)
		PARSE_HEAD_FIELD(pTitle, pValue, allow, STR_ALLOW)
		PARSE_HEAD_FIELD(pTitle, pValue, conference, STR_CONFERENCE)
		PARSE_HEAD_FIELD(pTitle, pValue, connection, STR_CONNECTION)
		/*Begin: Add by yehao 2007-09-06*/
		PARSE_HEAD_FIELD(pTitle, pValue, _x_Cache_Control, STR_CACHE_CONTROL_X)
		PARSE_HEAD_FIELD(pTitle, pValue, _x_Accept_Retransmit, STR_ACCEPT_RETRANSMIT_X)
		PARSE_HEAD_FIELD(pTitle, pValue, _x_Accept_Dynamic_Rate, STR_ACCEPT_DYNAMIC_RATE_X)
		PARSE_HEAD_FIELD(pTitle, pValue, _x_Dynamic_Rate, STR_DYNAMIC_RATE_X)
		/*End: yehao 2007-09-06*/
		PARSE_HEAD_FIELD(pTitle, pValue, contentBase, STR_CONTENT_BASE)
		PARSE_HEAD_FIELD(pTitle, pValue, contentEncoding, STR_CONTENT_ENCODING)
		PARSE_HEAD_FIELD(pTitle, pValue, contentLanguage, STR_CONTENT_LANGUAGE)
		PARSE_HEAD_FIELD(pTitle, pValue, range, STR_RANGE)
		PARSE_HEAD_FIELD(pTitle, pValue, rtpInfo, STR_RTP_INFO)
		PARSE_HEAD_FIELD(pTitle, pValue, session, STR_SESSION)
		PARSE_HEAD_FIELD(pTitle, pValue, timestamp, STR_TIMESTAMP)
		PARSE_HEAD_FIELD(pTitle, pValue, transport, STR_TRANSPORT)
		PARSE_HEAD_FIELD(pTitle, pValue, wwwAuthenticate, STR_WWW_AUTHENTICATE)
		PARSE_HEAD_FIELD(pTitle, pValue, Authorization, STR_AUTHOR)
		PARSE_HEAD_FIELD(pTitle, pValue, scale, STR_SCALE)
		PARSE_HEAD_FIELD(pTitle, pValue, speed, STR_SPEED)
		PARSE_HEAD_FIELD(pTitle, pValue, unsupport, STR_UNSUPPORT)
		PARSE_HEAD_FIELD(pTitle, pValue, vary, STR_VARY)
		PARSE_HEAD_FIELD(pTitle, pValue, lastModified, STR_LAST_MODIFIED)
		PARSE_HEAD_FIELD(pTitle, pValue, expires, STR_EXPIRES)
		PARSE_HEAD_FIELD(pTitle, pValue, contact, STR_CONTACT)
		PARSE_HEAD_FIELD(pTitle, pValue, expires, STR_EXPIRES)

		PARSE_HEAD_FIELD_INT(pTitle, pValue, bandwidth, STR_BANDWIDTH)
		PARSE_HEAD_FIELD_INT(pTitle, pValue, blockSize, STR_BLOCKSIZE)

		//rtsp head fields
		PARSE_HEAD_FIELD(pTitle, pValue, rtsp_public, STR_RTSP_PUBLIC)
		PARSE_HEAD_FIELD_INT(pTitle, pValue, rtsp_token, STR_RTSP_TOKEN)
		else if (strcmp(pTitle, "Data_Param2") == 0)
		{
			data_param2 = pValue;
		}
		else
		{
			parseLine(pTemp);
		}
		
		//越过\r\n，不然index不会增长。
		index += 2;
	}

	return HTTP_SUCCESS;
}

char* HTTPCommon::toStream(void)
{
	//打包时需要先打包体，再打包头。因为Content-Length需要在打包体时由用户生成。
	//调用用户打包包体方法，实现用户级打包功能
	if (_body == NULL)
	{
		_body = new char[LEN_HTTP_BODY];
		memset(_body,0,LEN_HTTP_BODY);
	}
	if (contentLength == 0)
		contentLength = strlen(_body);

	if (contentLength >= LEN_HTTP_BODY)
	{
		if (_buffer)
		{
			delete[] _buffer;
		}

		_buffer = new char[1024+contentLength];
		memset(_buffer, 0, 1024+contentLength);
	}
	else
	{
		if(_buffer == NULL)
			_buffer = new char [LEN_HTTP_BUFFER];
		memset(_buffer, 0, LEN_HTTP_BUFFER);
	}

	//打包包头并放进缓冲区（打包好的包头流在m_data里，下面打包体会覆盖，所以要暂存起来，并
	//为打包包体清空数据Buffer）	
	packetHead();

	//清理数据区
	//memset(m_body, 0, sizeof(m_body));
	strncat(_buffer, "\r\n", LEN_HTTP_BUFFER);

	_buffLen = strlen(_buffer);
	if (contentLength > 0)
	{
		memcpy(_buffer + _buffLen, _body, contentLength );
		_buffLen += contentLength;
	}

	/*
	if (strlen(_body) > 0)
	{
		strcat(_buffer, _body);
	}
	*/

	//打包好的字符流放在m_data里，返回m_data避免野指针Bug存在。
	//包头后和包体后的空行在此处添加，空行为“\r\n\r\n”

//	_buffLen = strlen(_buffer);
	return _buffer;
}

char* HTTPCommon::toStream(int &len)
{	
	if (_buffer == NULL)
	{
		_buffer = new char[LEN_HTTP_BUFFER];
	}
	memset(_buffer, 0, LEN_HTTP_BUFFER);
	if (contentLength == 0)
		contentLength = strlen(_body);

	packetHead();

	strncat(_buffer, "\r\n",LEN_HTTP_BUFFER);
	_buffLen = strlen(_buffer);

	if (_bodyLen > 0)
	{
		memcpy(_buffer + _buffLen, _body, _bodyLen );
		_buffLen += _bodyLen;
	}

	len = _buffLen;
	return _buffer;
}

int HTTPCommon::packetCommon(void)
{
	char str[512];

	//context-length域
	memset(str, 0, sizeof(str));	
	if (_body == NULL)
	{
        base::snprintf(str, sizeof(str), "%s: 0\r\n", STR_CONTENT_LENGTH);
	}
	else
	{
		//sprintf(str, "%s:%d\r\n", STR_CONTENT_LENGTH, strlen(_body));
        base::snprintf(str, sizeof(str), "%s: %d\r\n", STR_CONTENT_LENGTH, contentLength);
	}
	strncat(_buffer, str,LEN_HTTP_BUFFER);

	//context-type域
	if (contentType >= 0)
	{
		memset(str, 0, sizeof(str));
		switch (contentType)
		{
		case CONTENT_TYPE_XML:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_XML);
			break;
		case CONTENT_TYPE_SDP:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_SDP);
			break;
		case CONTENT_TYPE_JSON:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_JSON);
			break;
		case CONTENT_TYPE_HTTP:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_HTTP);
			break;
		case CONTENT_TYPE_HTML:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_HTML);
			break;
		case CONTENT_TYPE_OCTET:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, STR_CONTENT_TYPE_OCTET);
			break;
		default:
            base::snprintf(str, sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, contentTypeString);
			break;
		}
		strncat(_buffer, str,LEN_HTTP_BUFFER);
	}	
	else if(strlen(contentTypeString) > 0)
	{
        base::snprintf(str,sizeof(str), "%s: %s\r\n", STR_CONTENT_TYPE, contentTypeString);
		strncat(_buffer, str,LEN_HTTP_BUFFER);
	}

	PACKET_HEAD_FIELD_STR(from, STR_FROM)
	
	PACKET_HEAD_FIELD_STR(to, STR_TO)

	PACKET_HEAD_FIELD_STR(callID, STR_CALLID)

	PACKET_HEAD_FIELD_STR(contact, STR_CONTACT)

	PACKET_HEAD_FIELD_STR(cseq, STR_CSEQ)

	PACKET_HEAD_FIELD_INT(maxForwards, STR_MAX_FORWARDS)

	//date域
	PACKET_HEAD_FIELD_STR(date, STR_DATE)

	//X-Client-Address域	
	PACKET_HEAD_FIELD_STR(xClientAddress, STR_X_CLIENT_ADDRESS)
	
	//X-Transaction-ID域
	PACKET_HEAD_FIELD_STR(xTransactionID, STR_X_TRANSACTION_ID)
	
	//via域
	PACKET_HEAD_FIELD_STR(via, STR_VIA)
		
	//server域	
	PACKET_HEAD_FIELD_STR(server, STR_SERVER)
	
	//set-cookie域	
	PACKET_HEAD_FIELD_STR(setCookie, STR_SET_COOKIE)

	//cookie域
	PACKET_HEAD_FIELD_STR(cookie, STR_COOKIE)

	//host域
	PACKET_HEAD_FIELD_STR(host, STR_HOST)

	//accept域
	PACKET_HEAD_FIELD_STR(accept, STR_ACCEPT)

	//userAgent域
	PACKET_HEAD_FIELD_STR(userAgent, STR_USER_AGENT)

	PACKET_HEAD_FIELD_STR(acceptEncoding, STR_ACCEPT_ENCODING)
	PACKET_HEAD_FIELD_STR(acceptLanguage, STR_ACCEPT_LANGUAGE)
	PACKET_HEAD_FIELD_STR(allow, STR_ALLOW)
	PACKET_HEAD_FIELD_STR(conference, STR_CONFERENCE)
	PACKET_HEAD_FIELD_STR(connection, STR_CONNECTION)
	PACKET_HEAD_FIELD_STR(contentBase, STR_CONTENT_BASE)
	PACKET_HEAD_FIELD_STR(contentEncoding, STR_CONTENT_ENCODING)
	PACKET_HEAD_FIELD_STR(contentLanguage, STR_CONTENT_LANGUAGE)
	PACKET_HEAD_FIELD_STR(range, STR_RANGE)
	PACKET_HEAD_FIELD_STR(rtpInfo, STR_RTP_INFO)
	PACKET_HEAD_FIELD_STR(session, STR_SESSION)
	PACKET_HEAD_FIELD_STR(timestamp, STR_TIMESTAMP)
	PACKET_HEAD_FIELD_STR(transport, STR_TRANSPORT)
	PACKET_HEAD_FIELD_STR(wwwAuthenticate, STR_WWW_AUTHENTICATE)
	PACKET_HEAD_FIELD_STR(Authorization, STR_AUTHOR)
	PACKET_HEAD_FIELD_STR(scale, STR_SCALE)
	PACKET_HEAD_FIELD_STR(speed, STR_SPEED)
	PACKET_HEAD_FIELD_STR(unsupport, STR_UNSUPPORT)
	PACKET_HEAD_FIELD_STR(lastModified, STR_LAST_MODIFIED)
	PACKET_HEAD_FIELD_STR(vary, STR_VARY)
	PACKET_HEAD_FIELD_STR(expires, STR_EXPIRES)

	PACKET_HEAD_FIELD_INT(bandwidth, STR_BANDWIDTH)
	PACKET_HEAD_FIELD_INT(blockSize, STR_BLOCKSIZE)
	/*Begin: Add by yehao 2007-09-06*/
	PACKET_HEAD_FIELD_STR(_x_Cache_Control, STR_CACHE_CONTROL_X)
	PACKET_HEAD_FIELD_STR(_x_Accept_Retransmit, STR_ACCEPT_RETRANSMIT_X)
	PACKET_HEAD_FIELD_STR(_x_Accept_Dynamic_Rate, STR_ACCEPT_DYNAMIC_RATE_X)
	PACKET_HEAD_FIELD_STR(_x_Dynamic_Rate, STR_DYNAMIC_RATE_X)
	/*End: yehao 2007-09-06*/

	//rtsp head fields
	PACKET_HEAD_FIELD_STR(rtsp_public, STR_RTSP_PUBLIC)
	PACKET_HEAD_FIELD_INT(rtsp_token, STR_RTSP_TOKEN)

	//头和体的空行不在头中加，包括包体最后的空行都不在包体中加，在上层PDU类中加。
	packetLine();

	return HTTP_SUCCESS;
}
/////////////////////////////////////////////////////////////
HTTPRequest::HTTPRequest() : HTTPCommon ()
{
	_reqOrRep = MODEL_REQUEST;
}

HTTPRequest::~HTTPRequest()
{

}

int HTTPRequest::parseHead(const char* data)
{	

	char pTemp[256] = {0};
	String::readWord(data, '\r', pTemp,sizeof(pTemp));	//读出第一行，格式如："POST /cgi-bin/test.do HTTP1.1"
	String::trim(pTemp);
	 
	//读operation
	if (String::indexOf(pTemp, "POST") == 0)
		method = M_POST;
	else if (String::indexOf(pTemp, "PUT") == 0)
		method = M_PUT;
	else if (String::indexOf(pTemp, "DELETE") == 0 ||
				String::indexOf(pTemp, "CONNECT") == 0 ||
			//	String::indexOf(pStr, "OPTIONS") >=0 ||
				String::indexOf(pTemp, "TRACE") == 0
				)
		method = M_UNSUPPORT;
	else if (String::indexOf(pTemp, "INVITE") == 0)
	{
		method = M_INVITE;
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "ACK") == 0)
	{
		method = M_ACK;
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "BYE") == 0)
	{
		method = M_BYE;
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "INFO") == 0)
	{
		method = M_INFO;
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "REGISTER") >= 0)
	{
		method = M_REGISTER;
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "OPTIONS") == 0)
	{
		method = M_OPTIONS;
		if (String::indexOf(pTemp, "SIP") >=0)
			_protocol = PROTOCOL_RTSP;
		else
			_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "DESCRIBE") == 0)
	{
		method = M_DESCRIBE;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "MESSAGE") == 0)
	{
		method = M_MESSAGE;
		_protocol= PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "ANNOUNCE") == 0)
	{
		method = M_ANNOUNCE;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "SETUP") == 0)
	{
		method = M_SETUP;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "PLAY") == 0)
	{
		method = M_PLAY;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "PAUSE") == 0)
	{
		method = M_PAUSE;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "TEARDOWN") == 0)
	{
		method = M_TEARDOWN;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "GET_PARAMETER") == 0)
	{
		method = M_GET_PARAMETER;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "SET_PARAMETER") == 0)
	{
		method = M_SET_PARAMETER;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "REDIRECT") == 0)
	{
		method = M_REDIRECT;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "RECORD") == 0)
	{
		method = M_RECORD;
		_protocol= PROTOCOL_RTSP;
	}
	else if (String::indexOf(pTemp, "GET") == 0)
	{
		method = M_GET;
	}
	else
	{
		method = M_INVALID;
		return HTTP_ERROR_UNSUPPORT;
	}
	
	//取URL字符
	String::readWord(data, (char)0x20, 2, url,sizeof(url));	
	//取版本信息
//	String::readWord(data, (char)0x20, 3);	

	return HTTPCommon::parseCommon(data);
}

int HTTPRequest::packetHead(void)
{
	char str[1024];

	memset(str, 0, sizeof(str));
	switch (method)
	{
	case M_POST:	
        base::snprintf(str, sizeof(str), "POST %s %s\r\n", url, STR_HTTP_VERSION);
		break;
	case M_GET:	
        base::snprintf(str, sizeof(str), "GET %s %s\r\n", url, STR_HTTP_VERSION);
		break;
	case M_PUT:	
        base::snprintf(str, sizeof(str), "PUT %s %s\r\n", url, STR_HTTP_VERSION);
		break;
	case M_INVITE:
        base::snprintf(str, sizeof(str), "INVITE %s %s\r\n", url, STR_SIP_VERSION);
		break;
	case M_BYE:
        base::snprintf(str, sizeof(str), "BYE %s %s\r\n", url , STR_SIP_VERSION);
		break;
	case M_ACK:
        base::snprintf(str, sizeof(str), "ACK %s %s\r\n", url , STR_SIP_VERSION);
		break;
	case M_INFO:
        base::snprintf(str, sizeof(str), "INFO %s %s\r\n", url , STR_SIP_VERSION);
		break;
	case M_REGISTER:
        base::snprintf(str, sizeof(str), "REGISTER %s %s\r\n", url , STR_SIP_VERSION);
		break;
	case M_OPTIONS:
        base::snprintf(str, sizeof(str), "OPTIONS %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_DESCRIBE:
        base::snprintf(str, sizeof(str), "DESCRIBE %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_MESSAGE:
        base::snprintf(str, sizeof(str), "MESSAGE %s %s\r\n", url , STR_SIP_VERSION);
		break;
	case M_ANNOUNCE:
        base::snprintf(str, sizeof(str), "ANNOUNCE %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_SETUP:
        base::snprintf(str, sizeof(str), "SETUP %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_PLAY:
        base::snprintf(str, sizeof(str), "PLAY %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_PAUSE:
        base::snprintf(str, sizeof(str), "PAUSE %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_TEARDOWN:
        base::snprintf(str, sizeof(str), "TEARDOWN %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_GET_PARAMETER:
        base::snprintf(str, sizeof(str), "GET_PARAMETER %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_SET_PARAMETER:
        base::snprintf(str, sizeof(str), "SET_PARAMETER %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_REDIRECT:
        base::snprintf(str, sizeof(str), "REDIRECT %s %s\r\n", url , STR_RTSP_VERSION);
		break;
	case M_RECORD:
        base::snprintf(str, sizeof(str), "RECORD %s %s\r\n", url , STR_RTSP_VERSION);
		break;

	default:
		break;
	}
	
	//int index = strlen(str);
	strncat(_buffer, str,LEN_HTTP_BUFFER);

	//依次打包进其它的包头域	
	return HTTPCommon::packetCommon();	
}
/////////////////////////////////////////////////////////////

HTTPResponse::HTTPResponse(void): HTTPCommon ()
{
	_reqOrRep = MODEL_RESPONSE;
	result = 200;
	memset(message, 0, sizeof(message));
    base::snprintf(message, sizeof(message), "%s", "OK");
}

HTTPResponse::~HTTPResponse()
{

}

int HTTPResponse::parseHead(const char* data)
{
	char pTemp[256] = {0};
	String::readWord(data, '\r', pTemp,sizeof(pTemp));	//读出第一行，格式如："POST /cgi-bin/test.do HTTP1.1"
	String::trim(pTemp);
	
	//读operation
	if (String::indexOf(pTemp, "HTTP") == 0)
	{
		_protocol = PROTOCOL_HTTP;
	}
	else if (String::indexOf(pTemp, "SIP") == 0)
	{
		_protocol = PROTOCOL_SIP;
	}
	else if (String::indexOf(pTemp, "RTSP") == 0)
	{
		_protocol = PROTOCOL_RTSP;
	}

	char pResult[64] = {0};
	String::readWord(data, (char)0x20, 2, pResult,sizeof(pResult));
	result = String::str2int(pResult);

	int messageStart = String::indexOf(2, data, (char)0x20) +1 ;
	int messageEnd = String::indexOf(1, data, '\r');

	AX_OS::strncpy(message, sizeof(message),data+messageStart, messageEnd-messageStart);
//	pStr = String::readWord(data, (char)0x20, 3);
//	strncpy(message, pStr, sizeof(message));		
	
	return HTTPCommon::parseCommon(data);
}

int HTTPResponse::packetHead(void)
{
	char str[256];

	memset(str, 0, sizeof(str));
	//打包第一行
	switch (_protocol)
	{
	case PROTOCOL_HTTP:
        base::snprintf(str, sizeof(str), "HTTP/1.1 %d %s\r\n", result, message);
		break;
	case PROTOCOL_SIP:
        base::snprintf(str, sizeof(str), "SIP/2.0 %d %s\r\n", result, message);
		break;
	case PROTOCOL_RTSP:
        base::snprintf(str, sizeof(str), "RTSP/1.0 %d %s\r\n", result, message);
		break;
	default:
		break;
	}

	//int index = strlen(str);
	strncat(_buffer, str,LEN_HTTP_BUFFER);
	//依次打包进其它的包头域	
	
	return HTTPCommon::packetCommon();
}


HTTPCommon* HTTPFactory::createPDUFromStream(char* data)
{
	HTTPCommon* obj = NULL;

	//读出第一行，便于比较，gaowei,08-07-15
	char pTemp[256] = {0};
	String::readWord(data, '\r', pTemp,sizeof(pTemp));
	String::trim(pTemp);

	if (String::indexOf(pTemp, "POST") == 0 ||
		String::indexOf(pTemp, "GET") == 0 ||
		String::indexOf(pTemp, "PUT") == 0	||

		String::indexOf(pTemp, "INVITE") == 0 ||
		String::indexOf(pTemp, "ACK") == 0 ||
		String::indexOf(pTemp, "BYE") == 0 ||
		String::indexOf(pTemp, "INFO") == 0 ||
		String::indexOf(pTemp, "REGISTER") == 0 ||

		String::indexOf(pTemp, "OPTIONS") == 0 ||
		String::indexOf(pTemp, "DESCRIBE") == 0 ||
		String::indexOf(pTemp, "ANNOUNCE") == 0 ||
		String::indexOf(pTemp, "SETUP") == 0 ||
		String::indexOf(pTemp, "PLAY") == 0 ||
		String::indexOf(pTemp, "PAUSE") == 0 ||
		String::indexOf(pTemp, "TEARDOWN") == 0 ||
		String::indexOf(pTemp, "GET_PARAMETER") == 0 ||
		String::indexOf(pTemp, "SET_PARAMETER") == 0 ||
		String::indexOf(pTemp, "REDIRECT") == 0 ||
		String::indexOf(pTemp, "RECORD") == 0
		)
	{
		obj = new HTTPRequest();
	}
	else if (String::indexOf(pTemp, "HTTP") == 0 ||
		String::indexOf(pTemp, "SIP") == 0 ||
		String::indexOf(pTemp, "RTSP") == 0)
	{
		obj = new HTTPResponse();
	}
	else
		return obj;

	if ( obj->fromStream(data) < 0 )
	{
		delete obj;
		obj = NULL;
	}

	return obj;
}

HTTPCommon* HTTPFactory::createPDUFromStream(char *data, int len, int &readlen, int wholeBody)
{
	HTTPCommon* obj = NULL;

	//读出第一行，便于比较，gaowei,08-07-15
	char pTemp[256] = {0};
	String::readWord(data, '\r', pTemp,sizeof(pTemp));
	String::trim(pTemp);

	if (String::indexOf(pTemp, "POST") == 0 ||
		String::indexOf(pTemp, "GET") == 0 ||
		String::indexOf(pTemp, "PUT") == 0	||

		String::indexOf(pTemp, "INVITE") == 0 ||
		String::indexOf(pTemp, "ACK") == 0 ||
		String::indexOf(pTemp, "BYE") == 0 ||
		String::indexOf(pTemp, "INFO") == 0 ||
		String::indexOf(pTemp, "REGISTER") == 0 ||

		String::indexOf(pTemp, "OPTIONS") == 0 ||
		String::indexOf(pTemp, "DESCRIBE") == 0 ||
		String::indexOf(pTemp, "ANNOUNCE") == 0 ||
		String::indexOf(pTemp, "SETUP") == 0 ||
		String::indexOf(pTemp, "PLAY") == 0 ||
		String::indexOf(pTemp, "PAUSE") == 0 ||
		String::indexOf(pTemp, "TEARDOWN") == 0 ||
		String::indexOf(pTemp, "GET_PARAMETER") == 0 ||
		String::indexOf(pTemp, "SET_PARAMETER") == 0 ||
		String::indexOf(pTemp, "REDIRECT") == 0 ||
		String::indexOf(pTemp, "RECORD") == 0
		)
	{
		obj = new HTTPRequest();
	}
	else if (String::indexOf(pTemp, "HTTP") == 0 ||
		String::indexOf(pTemp, "SIP") == 0 ||
		String::indexOf(pTemp, "RTSP") == 0)
	{
		obj = new HTTPResponse();
	}
	else
		return obj;

	readlen = obj->fromStream(data, len);

	//解析失败
	if ( readlen < 0)
	{
		delete obj;
		obj = NULL;
	}

	if (obj != NULL && wholeBody == 1)
	{
		//需要返回完全的包体，要判断一下包体是否不完整
		if (obj->contentLength > obj->getBodyLen())
		{
			delete obj;
			obj = NULL;
		}
	}

	return obj;
}
