#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
//��־�������ʾ��Ϣ
static const char * KEYINFOPREFIX   = " Key: \n";
static const char * ERRORPREFIX = " Error: \n";
static const char * WARNINGPREFIX   = " Warning: \n";
static const char * INFOPREFIX      = " Info: \n";
 
static const int MAX_STR_LEN = 1024;
//��־����ö��
typedef enum EnumLogLevel
{
    LogLevelAll = 0,    //������Ϣ��д��־
    LogLevelMid,        //д���󡢾�����Ϣ
    LogLevelNormal,     //ֻд������Ϣ
    LogLevelStop        //��д��־
};
 
#endif