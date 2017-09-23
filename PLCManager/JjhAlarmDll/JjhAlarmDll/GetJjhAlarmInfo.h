#pragma once

#include <string>
#include <Winsock2.h>
#include "JjhAlarmDll.h"
#include "AX_Mutex.h"
#include "GetJjhAlarmClient.h"
#include "IniFile.h"
#include "Global.h"
#include "logger.h"

#define REQUEST_LEN 12
#define RESPONSE_LEN 10

static char RequestBuf[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x01, 0x00, 0x0A, 0x00, 0x01};



typedef struct IpcId2Monitorid
{
	std::string sIpcId;
	int iMonitorId;
	IpcId2Monitorid()
	{
		sIpcId = "";
		iMonitorId = 0;
	}
}IpcId2Monitorid;

typedef std::vector<IpcId2Monitorid> VecIpc;

typedef struct JjhInputItem
{
	std::string _sIp;
	int _iPort;
	int _iScreen;
	int _iSplit;
	int _iMode;
    VecIpc _VecIpc;
	JjhInputItem()
	{
		_iPort = 0;
		_iScreen = 0;
		_iSplit = 0;
		_iMode = 0;
		_sIp = "";
	}
}JjhInputItem;

typedef std::map<int, JjhInputItem> MapJjhScreen;

class GetJjhAlarmInfo
{  
private:  
	GetJjhAlarmInfo();
	~GetJjhAlarmInfo();
	GetJjhAlarmInfo(const GetJjhAlarmInfo &){}  
	GetJjhAlarmInfo& operator = (const GetJjhAlarmInfo &){} 

public:
	//Data
	void Showlog2Dlg(const char *pStr, int itype);
	void InitData();
	long LogInJjhServer(const char *Ip, int nPort);
	void StartGetJjhInfo();
	void StopGetJjhInfo();
	void StartReConnect();
	void StopReConnect();
	void ParseRevData(const char *pData, int iLen);

private:
	void RunGetJjhInfo();
	void RunReConnect();
	bool SendInfo2TVWALLServer(std::string sRet, std::string ip, int nPort);
	static void WorkThreadGetJjhInfo(LPVOID lpParam);
	static void WorkThreadReConnect(LPVOID lpParam);

public:  
	static GetJjhAlarmInfo *Instantialize();  
	static void Unstantialize();
	static GetJjhAlarmInfo *pInstance;  

    int m_nRefresh;
	unsigned long m_dwDataUser; 
	CallbackFuncRealData m_InfoCallFun;
	CallbackFuncLog m_CallLoginfo;
	Logger *m_plog;
	//======2017/1/5===============
	std::vector<std::string>puidfEIO;
	CRITICAL_SECTION g_cs;  //锁

private:
	AX_Mutex m_Lock;
	int m_iCurAlarmState;
	GetJjhAlarmClient *m_pGetAlarmClient;//data 京金华设备端口值和屏幕预案
	HANDLE m_ReConnectThread;
	HANDLE m_GetJjhInfoThread;
	bool m_bReConnect;
	bool m_bGetInfo;
	MapJjhScreen m_MapJjhScreen;
	std::string m_sJjhServerIp;
	int m_iJjhServerPort;
	int m_iKeyId;
	int m_iLogLevel;

};  
