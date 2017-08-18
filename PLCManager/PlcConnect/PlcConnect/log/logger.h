#ifndef LOGGER_H_
#define LOGGER_H_
#include <stdafx.h>
#include <Windows.h>
#include <stdio.h>
#include "CommandDefine.h"
/*
    * ������Logger
    * ���ã��ṩд��־���ܣ�֧�ֶ��̣߳�֧�ֿɱ��β���������֧��д��־���������
    * �ӿڣ�SetLogLevel������д��־����
            TraceKeyInfo��������־����д�ؼ���Ϣ
            TraceError��д������Ϣ
            TraceWarning��д������Ϣ
            TraceInfo��дһ����Ϣ
*/

class Logger
{
public:
    //Ĭ�Ϲ��캯��
    Logger();
    //���캯��
    Logger(const char * strLogPath, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal, int nPTZ = 0, int nZOOM = 0, int nTVWALL = 0, int nSwitchGroup = 0);
    //��������
    virtual ~Logger();
public:
    //д�ؼ���Ϣ
    void TraceKeyInfo(const char * strInfo, ...);
    //д������Ϣ
    void TraceError(const char* strInfo, ...);
    //д������Ϣ
    void TraceWarning(const char * strInfo, ...);
    //дһ����Ϣ
    void TraceInfo(const char * strInfo, ...);
	//дPTZ��Ϣ
	void TracePTZInfo(const char * strInfo, ...);
	//дZOOM��Ϣ
	void TraceZOOMInfo(const char * strInfo, ...);
	//дTVWALL��Ϣ
	void TraceTVWALLInfo(const char * strInfo, ...);
	//д�����л���Ϣ
	void TraceSWITCHInfo(const char * strInfo, ...);
    //����д��־����
    void SetLogLevel(EnumLogLevel nLevel);

	void WriteFile(const unsigned char *pData, const char *pstrOperate, int nLength = 190);
private:
    //д�ļ�����
    void Trace(const char * strInfo);
    //��ȡ��ǰϵͳʱ��
    char * GetCurrentTime();
    //������־�ļ�����
    void GenerateLogName();
    //������־·��
    void CreateLogPath();
    
private:
    //д��־�ļ���
    FILE * m_pFileStream;
	FILE * m_pFile;//����������ļ�
    //д��־����
    EnumLogLevel m_nLogLevel;
    //��־��·��
    char m_strLogPath[MAX_STR_LEN];
    //��־������
    char m_strCurLogName[MAX_STR_LEN];
    //�߳�ͬ�����ٽ�������
    CRITICAL_SECTION m_cs;
	//��־��󻺴�����
	int m_nLogMaxNum;
	int m_nPTZ;
	int m_nTVWALL;
	int m_nZOOM;
	int m_nSwitchGroup;
	bool m_bWrite;
};
 
#endif