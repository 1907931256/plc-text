#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
//日志级别的提示信息
static const char * KEYINFOPREFIX   = " Key: ";
static const char * ERRORPREFIX = " Error: ";
static const char * WARNINGPREFIX   = " Warning: ";
static const char * INFOPREFIX      = " INFO: ";
static const char * INFOPTZ			= " PTZ: ";
static const char * INFOZOOM		= " ZOOM: ";
static const char * INFOTVWALL      = " TVWALL: ";
static const char * INFOSWITCH      = " SWITCHGROUP: ";
 
static const int MAX_STR_LEN = 2048;
static const int MAX_NUM_LOG = 40960;	//存日志的最大条数
//日志级别枚举
typedef enum EnumLogLevel
{
    LogLevelAll = 0,    //所有信息都写日志
    LogLevelMid,        //写错误、警告信息
    LogLevelNormal,     //只写错误信息
    LogLevelStop        //不写日志
};
 
#endif