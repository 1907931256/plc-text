//#include "stdafx.h"
#include "logger.h"
#include <imagehlp.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#pragma comment(lib, "DbgHelp.lib")
 
//默认构造函数
Logger::Logger()
{
    //初始化
    memset(m_strLogPath, 0, MAX_STR_LEN);
    memset(m_strCurLogName, 0, MAX_STR_LEN);
    m_pFileStream = NULL;
	m_pFile = NULL;
	m_nLogMaxNum = 0;
	m_nPTZ = 0;
	m_nZOOM = 0;
	m_nTVWALL = 0;
	m_bWrite = false;

    //设置默认的写日志级别
    m_nLogLevel = EnumLogLevel::LogLevelNormal;
    //初始化临界区变量
    InitializeCriticalSection(&m_cs);
    //创建日志文件名
    GenerateLogName();
}
 
//构造函数
Logger::Logger(const char * strLogPath, EnumLogLevel nLogLevel, int nPTZ, int nZOOM, int nTVWALL, int nSwitchGroup):m_nLogLevel(nLogLevel)
{
    //初始化
	m_pFile = NULL;
    m_pFileStream = NULL;
	m_nLogMaxNum = 0;

	m_nPTZ = nPTZ;
	m_nTVWALL = nTVWALL;
	m_nZOOM = nZOOM;
	m_nSwitchGroup = nSwitchGroup;
	m_bWrite = false;

    strcpy(m_strLogPath, strLogPath);
    InitializeCriticalSection(&m_cs);
    CreateLogPath();
    GenerateLogName();
}
 
 
//析构函数
Logger::~Logger()
{
    //释放临界区
    DeleteCriticalSection(&m_cs);
    //关闭文件流
    if(m_pFileStream)
        fclose(m_pFileStream);
	if (m_pFile)
	{
		fclose(m_pFile);
	}
}
 
//写关键信息接口
void Logger::TraceKeyInfo(const char * strInfo, ...)
{
    if(!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, KEYINFOPREFIX);
    //获取可变形参
    va_list arg_ptr = NULL;
    va_start(arg_ptr, strInfo);
    vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1, strInfo, arg_ptr);
    va_end(arg_ptr);
    //写日志文件
    Trace(pTemp);
    arg_ptr = NULL;
 
}
 
//写错误信息
void Logger::TraceError(const char* strInfo, ...)
{
    //判断当前的写日志级别，若设置为不写日志则函数返回
    if(m_nLogLevel >= EnumLogLevel::LogLevelStop)
        return;
    if(!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, ERRORPREFIX);
    va_list arg_ptr = NULL;
    va_start(arg_ptr, strInfo);
    vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1, strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr = NULL;
}
 
//写警告信息
void Logger::TraceWarning(const char * strInfo, ...)
{
    //判断当前的写日志级别，若设置为只写错误信息则函数返回
    if(m_nLogLevel >= EnumLogLevel::LogLevelNormal)
        return;
    if(!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, WARNINGPREFIX);
    va_list arg_ptr = NULL;
    va_start(arg_ptr, strInfo);
    vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr = NULL;
}
 
 
//写一般信息
void Logger::TraceInfo(const char * strInfo, ...)
{
    //判断当前的写日志级别，若设置只写错误和警告信息则函数返回
    if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
        return;
    if(!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp,INFOPREFIX);
    va_list arg_ptr = NULL;
    va_start(arg_ptr, strInfo);
   vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr = NULL;
}
//写PTZ信息
void Logger::TracePTZInfo(const char * strInfo, ...)
{
	if (1 != m_nPTZ)
	{
		return;
	}
	//判断当前的写日志级别，若设置只写错误和警告信息则函数返回
	if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp,INFOPTZ);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}
//写ZOOM信息
void Logger::TraceZOOMInfo(const char * strInfo, ...)
{
	if (1 != m_nZOOM)
	{
		return;
	}
	//判断当前的写日志级别，若设置只写错误和警告信息则函数返回
	if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp,INFOZOOM);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}
//写TVWALL信息
void Logger::TraceTVWALLInfo(const char * strInfo, ...)
{
	if (1 != m_nTVWALL)
	{
		return;
	}
	//判断当前的写日志级别，若设置只写错误和警告信息则函数返回
	if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp,INFOTVWALL);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}
//写整组切换信息
void Logger::TraceSWITCHInfo(const char * strInfo, ...)
{
	if (1 != m_nSwitchGroup)
	{
		return;
	}
	//判断当前的写日志级别，若设置只写错误和警告信息则函数返回
	if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp,INFOSWITCH);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsnprintf(pTemp + strlen(pTemp),MAX_STR_LEN-strlen(pTemp)-1,  strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}
//获取系统当前时间
char * Logger::GetCurrentTime()
{
    time_t curTime;
    struct tm * pTimeInfo = NULL;
    time(&curTime);
    pTimeInfo = localtime(&curTime);
    char temp[MAX_STR_LEN] = {0};
    _snprintf(temp, MAX_STR_LEN-1, "%02d:%02d:%02d", pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec);
    char * pTemp = temp;
    return pTemp;
}
 
//设置写日志级别
void Logger::SetLogLevel(EnumLogLevel nLevel)
{
    m_nLogLevel = nLevel;
}
 
//写文件操作
void Logger::Trace(const char * strInfo)
{
    if(!strInfo)
        return;
    try
    {
		char buf[100] = {0};
		strcpy(buf, GetCurrentTime());
		if (strcmp(buf, "12:00:00") <= 0)
		{
			m_bWrite = false;
		}
		if (strcmp(buf, "12:00:00") >= 0 && !m_bWrite)
		{
			m_bWrite = true;
			if(m_pFileStream)
				fclose(m_pFileStream);
			if (m_pFile)
			{
				fclose(m_pFile);
			}
			m_pFile = NULL;
			m_pFileStream = NULL;
			m_nLogMaxNum = 0;
			GenerateLogName();
		}

        //进入临界区
        EnterCriticalSection(&m_cs);
		
		char temp[1024] = {0};
		strcat(temp, m_strLogPath);
		strcat(temp, m_strCurLogName);
        //若文件流没有打开，则重新打开
        if(!m_pFileStream)
        {
            m_pFileStream = fopen(temp, "w+");
            if(!m_pFileStream)
            {
                return;
            }
        }
	
		if (m_nLogMaxNum >= MAX_NUM_LOG)
		{
			long nSize = ftell(m_pFileStream);
			rewind(m_pFileStream);
			char *sBuf = new char [nSize];
			memset(sBuf, 0, nSize);
			fwrite(sBuf, nSize, 1, m_pFileStream);
			rewind(m_pFileStream);

			delete [] sBuf;
			m_nLogMaxNum = 0;
		}
		strcat((char *)strInfo, "\n");
        //写日志信息到文件流
        fwrite(strInfo, strlen(strInfo), 1, m_pFileStream);
        fflush(m_pFileStream);
		m_nLogMaxNum++;
        //离开临界区
        LeaveCriticalSection(&m_cs);
    }
    //若发生异常，则先离开临界区，防止死锁
    catch(...)
    {
        LeaveCriticalSection(&m_cs);
    }
}
 
//创建日志文件的名称
void Logger::GenerateLogName()
{
    time_t curTime;
    struct tm * pTimeInfo = NULL;
    time(&curTime);
    pTimeInfo = localtime(&curTime);
    char temp[1024] = {0};
    //日志的名称如：2013-01-01.log
    _snprintf(temp, MAX_STR_LEN-1, "%04d-%02d-%02d-%02d-%02d-%02d.txt", pTimeInfo->tm_year+1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday,
		                                               pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec);
    if(0 != strcmp(m_strCurLogName, temp))
    {
        strcpy(m_strCurLogName,temp);
        if(m_pFileStream)
            fclose(m_pFileStream);
        char temp[1024] = {0};
        strcat(temp, m_strLogPath);
        strcat(temp, m_strCurLogName);
        //以追加的方式打开文件流
        m_pFileStream = fopen(temp, "w+");
    }
}
 
//创建日志文件的路径
void Logger::CreateLogPath()
{
    if(0 != strlen(m_strLogPath))
    {
        strcat(m_strLogPath, "\\log\\");
    }
    MakeSureDirectoryPathExists(m_strLogPath);
}

void Logger::WriteFile(const unsigned char *pData, const char *pstrOperate, int nLength)
{
	if(m_nLogLevel >= EnumLogLevel::LogLevelStop)
		return;
	time_t curTime;
	struct tm * pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);

	char temp[1024] = {0};
	//日志的名称如：path2013-01-01.log
	_snprintf(temp, MAX_STR_LEN-1, "%s%04d-%02d-%02d-%02d-%02d-%02d_%s.txt", m_strLogPath, pTimeInfo->tm_year+1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday,
		pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec, pstrOperate);

	FILE * pFile = fopen(temp, "w+");
	if(pFile != NULL)
	{
		fwrite(pData, 1, nLength, pFile);
		fclose(pFile);
	}
	else
	{
		char tempTmp[1024] = {0};
		_snprintf(temp,MAX_STR_LEN,  "open file %s", temp);
		TraceInfo(temp);
	}
}