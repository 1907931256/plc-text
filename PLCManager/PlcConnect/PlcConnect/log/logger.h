#ifndef LOGGER_H_
#define LOGGER_H_
#include <stdafx.h>
#include <Windows.h>
#include <stdio.h>
#include "CommandDefine.h"
/*
    * 类名：Logger
    * 作用：提供写日志功能，支持多线程，支持可变形参数操作，支持写日志级别的设置
    * 接口：SetLogLevel：设置写日志级别
            TraceKeyInfo：忽略日志级别，写关键信息
            TraceError：写错误信息
            TraceWarning：写警告信息
            TraceInfo：写一般信息
*/

class Logger
{
public:
    //默认构造函数
    Logger();
    //构造函数
    Logger(const char * strLogPath, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal, int nPTZ = 0, int nZOOM = 0, int nTVWALL = 0, int nSwitchGroup = 0);
    //析构函数
    virtual ~Logger();
public:
    //写关键信息
    void TraceKeyInfo(const char * strInfo, ...);
    //写错误信息
    void TraceError(const char* strInfo, ...);
    //写警告信息
    void TraceWarning(const char * strInfo, ...);
    //写一般信息
    void TraceInfo(const char * strInfo, ...);
	//写PTZ信息
	void TracePTZInfo(const char * strInfo, ...);
	//写ZOOM信息
	void TraceZOOMInfo(const char * strInfo, ...);
	//写TVWALL信息
	void TraceTVWALLInfo(const char * strInfo, ...);
	//写整组切换信息
	void TraceSWITCHInfo(const char * strInfo, ...);
    //设置写日志级别
    void SetLogLevel(EnumLogLevel nLevel);

	void WriteFile(const unsigned char *pData, const char *pstrOperate, int nLength = 190);
private:
    //写文件操作
    void Trace(const char * strInfo);
    //获取当前系统时间
    char * GetCurrentTime();
    //创建日志文件名称
    void GenerateLogName();
    //创建日志路径
    void CreateLogPath();
    
private:
    //写日志文件流
    FILE * m_pFileStream;
	FILE * m_pFile;//保存二进制文件
    //写日志级别
    EnumLogLevel m_nLogLevel;
    //日志的路径
    char m_strLogPath[MAX_STR_LEN];
    //日志的名称
    char m_strCurLogName[MAX_STR_LEN];
    //线程同步的临界区变量
    CRITICAL_SECTION m_cs;
	//日志最大缓存条数
	int m_nLogMaxNum;
	int m_nPTZ;
	int m_nTVWALL;
	int m_nZOOM;
	int m_nSwitchGroup;
	bool m_bWrite;
};
 
#endif