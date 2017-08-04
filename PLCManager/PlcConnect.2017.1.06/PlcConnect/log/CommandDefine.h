#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
//��־�������ʾ��Ϣ
static const char * KEYINFOPREFIX   = " Key: ";
static const char * ERRORPREFIX = " Error: ";
static const char * WARNINGPREFIX   = " Warning: ";
static const char * INFOPREFIX      = " INFO: ";
static const char * INFOPTZ			= " PTZ: ";
static const char * INFOZOOM		= " ZOOM: ";
static const char * INFOTVWALL      = " TVWALL: ";
static const char * INFOSWITCH      = " SWITCHGROUP: ";
 
static const int MAX_STR_LEN = 2048;
static const int MAX_NUM_LOG = 40960;	//����־���������
//��־����ö��
typedef enum EnumLogLevel
{
    LogLevelAll = 0,    //������Ϣ��д��־
    LogLevelMid,        //д���󡢾�����Ϣ
    LogLevelNormal,     //ֻд������Ϣ
    LogLevelStop        //��д��־
};
 
#endif