#pragma once

#include "GetJjhAlarmInfo.h"

//#include "GetAxisJoyStickCom.h"

 

//#ifndef _JjhJogStick
//#define _JjhJogStick
GetJjhAlarmInfo* GetJjhAlarmInfo::pInstance = 0; 

//#endif


GetJjhAlarmInfo* GetJjhAlarmInfo::Instantialize()  
{ 
	if(pInstance == NULL)  
	{   //double check  
		//Lock lock(cs);           //用lock实现线程安全，用资源管理类，实现异常安全  
		//使用资源管理类，在抛出异常的时候，资源管理类对象会被析构，析构总是发生的无论是因为异常抛出还是语句块结束。  
		if(pInstance == NULL)  
		{  
			pInstance = new GetJjhAlarmInfo();  
		}  
	}  

	return pInstance;  
}  

void GetJjhAlarmInfo::Unstantialize()
{
	if (pInstance)
	{
		delete pInstance;
	}
}

GetJjhAlarmInfo::GetJjhAlarmInfo()
{
	m_sJjhServerIp = "0.0.0.0";
	m_iJjhServerPort = 502;
	m_iLogLevel = 0;
	m_iKeyId = 1;
	m_ReConnectThread = NULL;
	m_GetJjhInfoThread = NULL;
	m_bReConnect = false;
	m_bGetInfo = false;
	m_nRefresh = 50;
	m_iCurAlarmState = 0;
	InitializeCriticalSection(&g_cs); //初始化锁_2017/1/4
	m_pGetAlarmClient = new GetJjhAlarmClient(this);
}
GetJjhAlarmInfo::~GetJjhAlarmInfo()
{
    StopReConnect();
	StopGetJjhInfo();
	if (m_plog)
	{
		delete m_plog;
		m_plog = NULL;
	}
}

void GetJjhAlarmInfo::InitData()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");

	m_MapJjhScreen.empty();
	std::vector<std::string> JjhScreenAddArray;
	iniFile.ReadSectionString("JJHSCREEN", JjhScreenAddArray);
	for(std::vector<std::string>::iterator iteScreen = JjhScreenAddArray.begin(); 
		iteScreen != JjhScreenAddArray.end(); iteScreen++)
	{
		std::vector<std::string> vectorJjhScreen;
		StringSplit(*iteScreen, ",", vectorJjhScreen);

		std::vector<std::string> vectorJjhIpc;
		StringSplit(vectorJjhScreen[6], "-", vectorJjhIpc);

		VecIpc vectorJjhIpcEx;
		for (int i = 0; i < vectorJjhIpc.size(); i++)
		{
			IpcId2Monitorid ipcItem;
			ipcItem.sIpcId = vectorJjhIpc[i];
			ipcItem.iMonitorId = i + 1;
			vectorJjhIpcEx.push_back(ipcItem);
		}
		
        JjhInputItem Item;
		Item._VecIpc = vectorJjhIpcEx;
		Item._sIp = vectorJjhScreen[1];
		Item._iPort = atoi(vectorJjhScreen[2].c_str());
		Item._iScreen =  atoi(vectorJjhScreen[3].c_str());
		Item._iSplit = atoi(vectorJjhScreen[4].c_str());
		Item._iMode = atoi(vectorJjhScreen[5].c_str());
		m_MapJjhScreen.insert(std::make_pair(atoi(vectorJjhScreen[0].c_str()), Item));
	}

	iniFile.ReadString("JJHADDR", "ServerIp", m_sJjhServerIp);
	iniFile.ReadInt("JJHADDR", "ServerPort", m_iJjhServerPort);

	iniFile.ReadInt("ADDR", "ServerKeyId", m_iKeyId);
	iniFile.ReadInt("LOGLEVEl", "level", m_iLogLevel);

	//iniFile.ReadString("ADDR", "TvWallPromat", m_TvWallPromat);
	m_pGetAlarmClient->SetIP(m_sJjhServerIp.c_str(), m_iJjhServerPort);

	m_plog = new Logger(GetCurrentPath().c_str(), (EnumLogLevel)m_iLogLevel);//修改日志等级
}

void GetJjhAlarmInfo::Showlog2Dlg(const char *pStr, int itype)
{
	m_CallLoginfo(itype, pStr, m_dwDataUser);
}

long GetJjhAlarmInfo::LogInJjhServer(const char *Ip, int nPort)
{
    m_sJjhServerIp = Ip;
	m_iJjhServerPort = nPort;
    m_pGetAlarmClient->SetIP(m_sJjhServerIp.c_str(), m_iJjhServerPort);
    return m_pGetAlarmClient->Connect();
}

void GetJjhAlarmInfo::StartReConnect()
{
	DWORD dwThreadID;
	m_bReConnect = true;
	m_ReConnectThread = CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)WorkThreadReConnect , this , 0 , &dwThreadID);
}

void GetJjhAlarmInfo::StartGetJjhInfo()
{
	DWORD dwThreadID;
	m_bGetInfo = true;
	m_GetJjhInfoThread = CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)WorkThreadGetJjhInfo , this , 0 , &dwThreadID);
}

void GetJjhAlarmInfo::WorkThreadGetJjhInfo(LPVOID lpParam)
{
	GetJjhAlarmInfo* pThis = (GetJjhAlarmInfo*)lpParam;
	pThis->RunGetJjhInfo();
}

void GetJjhAlarmInfo::WorkThreadReConnect(LPVOID lpParam)
{
	GetJjhAlarmInfo* pThis = (GetJjhAlarmInfo*)lpParam;
	pThis->RunReConnect();
}

void GetJjhAlarmInfo::RunReConnect()
{
	while(m_bReConnect)
	{
		m_Lock.acquire();
		m_pGetAlarmClient->Connect();
		m_Lock.release();
		Sleep(2000);
	}
}

void GetJjhAlarmInfo::StopReConnect()
{
	if(m_ReConnectThread)
	{
		m_bReConnect = false;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_ReConnectThread,1000))
		{
			TerminateThread(m_ReConnectThread,-1);
			m_ReConnectThread = NULL;
		}
	}
}

void GetJjhAlarmInfo::StopGetJjhInfo()
{
	if(m_GetJjhInfoThread)
	{
		m_bGetInfo = false;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_GetJjhInfoThread,1000))
		{
			TerminateThread(m_GetJjhInfoThread,-1);
			m_GetJjhInfoThread = NULL;
		}
	}
}
void GetJjhAlarmInfo::RunGetJjhInfo()
{
	while(m_bGetInfo)
	{
		m_Lock.acquire();
		m_pGetAlarmClient->SendRequest(RequestBuf, REQUEST_LEN);
		m_Lock.release();
		Sleep(m_nRefresh);
	}
}

void GetJjhAlarmInfo::ParseRevData(const char *pData, int iLen)
{
	if (iLen == RESPONSE_LEN) //不是获取的input1,2,3,4端子的数据不处理
	{
		int iValue = pData[RESPONSE_LEN - 1] & 0xff;//(0-15代表不同的状态)
//============打印端口号======================================
		//char valueOut[50];
		//sprintf_s(valueOut,"EIO-4 value %d",iValue);
		//Showlog2Dlg(valueOut, CONNECT_PLC_SUC);
		//=====================================================================

		MapJjhScreen::iterator ite = m_MapJjhScreen.find(iValue);
		if (ite != m_MapJjhScreen.end() && iValue != m_iCurAlarmState)
		{
			//==========传给摇杆使用 2017/1/4========
			EnterCriticalSection(&g_cs);
			puidfEIO.clear();
			for (VecIpc::iterator ite_win = ite->second._VecIpc.begin(); 
				ite_win != ite->second._VecIpc.end(); ite_win++)
			{
				puidfEIO.push_back(ite_win->sIpcId);
			}
			LeaveCriticalSection(&g_cs);
			//=======================================

			m_iCurAlarmState = iValue;
			std::string sRet;
			sRet.append("{");
			SetValue(sRet, "key_id", m_iKeyId);
			SetValue(sRet, "command", "SCREEN_CUT", FALSE);
			SetValue(sRet, "reset", 1, FALSE);
			SetValue(sRet, "screen_id", ite->second._iScreen);
			SetValue(sRet, "number", ite->second._iSplit);
			SetValue(sRet, "mode", ite->second._iMode);
			SetjsonEnd(sRet);
			//SendInfo2TVWALLServer(sRet,ip,port);
			SendInfo2TVWALLServer(sRet, ite->second._sIp, ite->second._iPort);//分割

			std::string sRetVec;
			sRetVec.append("{");
			SetValue(sRetVec, "key_id", m_iKeyId);
			SetValue(sRetVec, "command", "MAP_IPC", FALSE);
			sRetVec.append("\"maps\":[");			
			for (VecIpc::iterator ite_win = ite->second._VecIpc.begin(); 
				ite_win != ite->second._VecIpc.end(); ite_win++)
			{
				sRetVec.append("{");
				SetValue(sRetVec, "ipc_id", ite_win->sIpcId.c_str(), FALSE);
				SetValue(sRetVec, "monitor_id", ite_win->iMonitorId + 64*(ite->second._iScreen-1), TRUE);//edit by jeckean,64 nocheck
				sRetVec.append("},");
			}
			sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
			sRetVec.append("]}");
		
			SendInfo2TVWALLServer(sRetVec, ite->second._sIp, ite->second._iPort);	
		}
	}
}

bool GetJjhAlarmInfo::SendInfo2TVWALLServer(std::string sRet, std::string ip, int nPort)
{
	//通过回调函数实现,链接不存在时容易卡死
	//char szlog[MAX_STR_LEN] = {0};
	//sprintf_s(szlog, MAX_STR_LEN, "TvwallIp:%s，TvwallIp:%d, info:%s",ip.c_str(), nPort, sRet.c_str());
	//CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
	m_InfoCallFun(sRet.c_str(),ip.c_str(), nPort, m_dwDataUser);
	return true;
}
