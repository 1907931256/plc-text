#pragma once
#ifndef _JogStick
#define _JogStick

#include <string>
#include <math.h>
#include <Winsock2.h>
#include "logger.h"
#include "Global.h"
#include "Dump.h"
#include "IniFile.h"
#include "JjhAlarmDll.h"
#include "AxisJoyType.h"


#define _ATL_ATTRIBUTES 1

#define BTN_DOWN 2
#define BTN_UP   3

#define AXIS_ZOOM_MIN -960
#define AXIS_ZOOM_MAX 967

#define STEP_LEN_MIN 1
#define STEP_LEN_MAX 8

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>

//==========2017/1/4摇杆配合EIO使用=========
#include "GetJjhAlarmInfo.h"

//#ifndef _JjhJogStick
//#define _JjhJogStick
//extern JjhInputItem JoyStickUse;
//extern CRITICAL_SECTION g_cs;
//extern JoyStickUse;
//extern  g_cs;
//#endif
//#include "GetJjhAlarmInfo..cpp"
//extern std::vector<std::string>puidfEIO;
//extern CRITICAL_SECTION g_cs;  //锁
//============================================


#ifdef WIN64
#import "..\\..\\..\\Gateway_dll\\PlcText\\Release\\AxisJoystickModuleX64.dll" embedded_idl, no_namespace
#else
#import "..\\..\\PlcText\\Release\\AxisJoystickModule.dll" embedded_idl, no_namespace
#endif 

[ module(name="JoystickTest") ];
[event_receiver(com)]

class JoystickManager
{
private:
	JoystickManager()
	{
		m_bJoyAlive = false;
		m_iLogLevel = 0;
		m_iKeyId = 1;
		m_iBtnCount = 1;
		m_iScreenTotal = 0;
		m_iIpcSelect = 1;
		m_ipcAutoChoose = false;
		memset(m_OldCmdState, 0, sizeof(m_OldCmdState));
		m_StartJoyStickThread = NULL;
		ptzStatus = "stop";
	}

	~JoystickManager()
	{
		myHandler.Release();
		CoUninitialize();
		StopJoyStickThread();
	}

public:
	std::string ptzStatus;
	void InitData()
	{

		std::string strPath = GetCurrentPath();
		CIniFile iniFile(strPath + "\\config\\General.ini");

		std::vector<std::string> BtnArray, ScreenArray;
		iniFile.ReadSectionString("AXISJOYSTICK_J", BtnArray);

		m_Index2IpcMap.empty();
		for(std::vector<std::string>::iterator ite = BtnArray.begin(); ite != BtnArray.end(); ite++)
		{
			std::vector<std::string> BtnItemVec;
			StringSplit(*ite, ",", BtnItemVec);
			IpcItem TmpItem;
			TmpItem.strIpcPuid = BtnItemVec[1];
			TmpItem.strTvWallIp = BtnItemVec[2];
			TmpItem.TvWallPort = _ttoi(BtnItemVec[3].c_str());
			m_Index2IpcMap.insert(std::make_pair(_ttoi(BtnItemVec[0].c_str()), TmpItem));
		}

		m_AxisScreenMap.empty();
		iniFile.ReadSectionString("AXISJOYSTICK_L_R", ScreenArray);
		m_iScreenTotal = ScreenArray.size();
		for(std::vector<std::string>::iterator ite = ScreenArray.begin(); ite != ScreenArray.end(); ite++)
		{
			std::vector<std::string> ScreenItemVec, VecIpc;
			StringSplit(*ite, ",", ScreenItemVec);
			StringSplit(ScreenItemVec[6], "-", VecIpc);
			AxisScreen ScreenItem;

			IpcVec vectorAxisIpcEx;
			for (int i = 0; i < VecIpc.size(); i++)
			{
				Ipc2Index ipcItem;
				ipcItem.strIpcId = VecIpc[i];
				ipcItem.iMonitorId = i + 1;
				vectorAxisIpcEx.push_back(ipcItem);
			}

			ScreenItem._VecIpc = vectorAxisIpcEx;
			ScreenItem._sIp = ScreenItemVec[1];
			ScreenItem._iPort = _ttoi(ScreenItemVec[2].c_str());
			ScreenItem._iScreen = _ttoi(ScreenItemVec[3].c_str());
			ScreenItem._iSplit = _ttoi(ScreenItemVec[4].c_str());
			ScreenItem._iMode = _ttoi(ScreenItemVec[5].c_str());
			m_AxisScreenMap.insert(std::make_pair(_ttoi(ScreenItemVec[0].c_str()), ScreenItem));
		}

		iniFile.ReadInt("ADDR", "ServerKeyId", m_iKeyId);
		iniFile.ReadInt("LOGLEVEl", "level", m_iLogLevel);
		m_plog = new Logger(strPath.c_str(), (EnumLogLevel)m_iLogLevel);//修改日志等级

		//==================2017/1/4==============================
		int ipcChooseSate=0;
		iniFile.ReadInt("JOYSTICKWITHEIO","enable",ipcChooseSate);
		if(ipcChooseSate==1)
			m_ipcAutoChoose = true; 
		//===========================================================

		//注册com组件
		HINSTANCE h;  
		std::string str = strPath + "\\AxisJoystickModule.dll";
		h = LoadLibrary((strPath + "\\AxisJoystickModule.dll").c_str());  
		FARPROC pFunc = ::GetProcAddress((HMODULE)h,"DllRegisterServer");  
		if (pFunc==NULL)  
		{  
			m_plog->TraceInfo("AxisJoystickModule.dll registered failed");
		}  
		else
		{
			m_plog->TraceInfo("AxisJoystickModule.dll registered success");
		    pFunc();
		}
        //打印dump信息
		InitMinDump();
	}
	void StartJoyStickThread()
	{
        m_bJoyAlive = true;
		DWORD dwThreadID;
		m_StartJoyStickThread = CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)WorkThreadJoyStick , this , 0 , &dwThreadID);
	}

	void StopJoyStickThread()
	{
		if(m_StartJoyStickThread)
		{
			m_bJoyAlive = false;
			if(WAIT_OBJECT_0!=WaitForSingleObject(m_StartJoyStickThread,1000))
			{
				TerminateThread(m_StartJoyStickThread,-1);
				m_StartJoyStickThread = NULL;
			}
		}
	}

	void Showlog2Dlg(const char *pStr, int itype = 0)
	{
		m_CallLoginfo(itype, pStr, m_dwDataUser);
	}

	void DealButtonEvent(int iType/*1:dowm 2:up*/, long theJoystickId, long theButtonNbr)
	{
        if(BTN_DOWN == iType)
		{
			DealBtnDown(theJoystickId, theButtonNbr);
			
		}
		else if(BTN_UP == iType)
		{
			DealBtnUp(theJoystickId, theButtonNbr);
		}
	}

	void DealBtnDown(long theJoystickId, long theButtonNbr)
	{
		if (theButtonNbr == 0 || theButtonNbr == 1 || theButtonNbr == 2 || theButtonNbr == 3) //ipc选中
		{
			DealIpcSelect(theJoystickId, theButtonNbr);
		}
		else if (theButtonNbr == 4 || theButtonNbr == 5)
		{
			DealScreenSplit(theJoystickId, theButtonNbr);
		}
		//==== for 沙特
		//if (theButtonNbr == 0 || theButtonNbr == 1 || theButtonNbr == 2 || theButtonNbr == 3 ||theButtonNbr == 4||theButtonNbr == 5) //ipc选中
		//{
		//	DealScreenSplitN(theJoystickId, theButtonNbr);
		//	DealIpcSelect(theJoystickId, theButtonNbr);
		//}


		//else if(theButtonNbr == 3)
		//{
		//	DealScreenSplit(theJoystickId, theButtonNbr);
		//	m_iIpcSelect = -1;
		//}
	}

	void DealIpcSelect(long theJoystickId, long theButtonNbr)
	{
		m_iIpcSelect = theButtonNbr + 1;//暂时不在TV_WALL上显示选中
	}

	void DealScreenSplitN(long theJoystickId, long theButtonNbr)
	{
		if (theButtonNbr == 4)//屏幕往前翻页
		{
			if (m_iBtnCount <= 1)
			{
				m_iBtnCount = m_iScreenTotal;
			}
			else
			{
				m_iBtnCount--;
			}
		}
		else if (theButtonNbr == 5)//屏幕往后翻页
		{
			if (m_iBtnCount >= m_iScreenTotal)
			{
				m_iBtnCount = 1;
			}
			else
			{
				m_iBtnCount++;
			}
		}
		//m_iBtnCount = theButtonNbr+1;

		MapAxisScreen::iterator ite = m_AxisScreenMap.find(m_iBtnCount);
		if (ite != m_AxisScreenMap.end())
		{
			std::string sRet;
			sRet.append("{");
			SetValue(sRet, "key_id", m_iKeyId);
			SetValue(sRet, "command", "SCREEN_CUT", FALSE);
			SetValue(sRet, "reset", 1, FALSE);
			SetValue(sRet, "screen_id", ite->second._iScreen);
			SetValue(sRet, "number", ite->second._iSplit);
			SetValue(sRet, "mode", ite->second._iMode);
			SetjsonEnd(sRet);
			SendJoyStickInf2Tvwall(sRet, ite->second._sIp, ite->second._iPort);//分割

			std::string sRetVec;
			sRetVec.append("{");
			SetValue(sRetVec, "key_id", m_iKeyId);
			SetValue(sRetVec, "command", "MAP_IPC", FALSE);
			sRetVec.append("\"maps\":[");			
			for (IpcVec::iterator ite_win = ite->second._VecIpc.begin(); 
				ite_win != ite->second._VecIpc.end(); ite_win++)
			{
				sRetVec.append("{");
				SetValue(sRetVec, "ipc_id", ite_win->strIpcId.c_str(), FALSE);
				SetValue(sRetVec, "monitor_id", ite_win->iMonitorId + 64*(ite->second._iScreen-1), TRUE);//edit by jeckean,64 nocheck
				sRetVec.append("},");
			}
			sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
			sRetVec.append("]}");
			SendJoyStickInf2Tvwall(sRetVec, ite->second._sIp, ite->second._iPort);	
		}
	}


	void DealScreenSplit(long theJoystickId, long theButtonNbr)
	{
		if (theButtonNbr == 4)//屏幕往前翻页
		{
			if (m_iBtnCount <= 1)
			{
				m_iBtnCount = m_iScreenTotal;
			}
			else
			{
				m_iBtnCount--;
			}
		}
		else if (theButtonNbr == 5)//屏幕往后翻页
		{
			if (m_iBtnCount >= m_iScreenTotal)
			{
				m_iBtnCount = 1;
			}
			else
			{
				m_iBtnCount++;
			}
		}
		//if (theButtonNbr == 5)//屏幕往后翻页
		//{
		//	if (m_iBtnCount >= m_iScreenTotal)
		//	{
		//		m_iBtnCount = 1;
		//	}
		//	else
		//	{
		//		m_iBtnCount++;
		//	}
		//}
		MapAxisScreen::iterator ite = m_AxisScreenMap.find(m_iBtnCount);
		if (ite != m_AxisScreenMap.end())
		{
			std::string sRet;
			sRet.append("{");
			SetValue(sRet, "key_id", m_iKeyId);
			SetValue(sRet, "command", "SCREEN_CUT", FALSE);
			SetValue(sRet, "reset", 1, FALSE);
			SetValue(sRet, "screen_id", ite->second._iScreen);
			SetValue(sRet, "number", ite->second._iSplit);
			SetValue(sRet, "mode", ite->second._iMode);
			SetjsonEnd(sRet);
			SendJoyStickInf2Tvwall(sRet, ite->second._sIp, ite->second._iPort);//分割

			std::string sRetVec;
			sRetVec.append("{");
			SetValue(sRetVec, "key_id", m_iKeyId);
			SetValue(sRetVec, "command", "MAP_IPC", FALSE);
			sRetVec.append("\"maps\":[");			
			for (IpcVec::iterator ite_win = ite->second._VecIpc.begin(); 
				ite_win != ite->second._VecIpc.end(); ite_win++)
			{
				sRetVec.append("{");
				SetValue(sRetVec, "ipc_id", ite_win->strIpcId.c_str(), FALSE);
				SetValue(sRetVec, "monitor_id", ite_win->iMonitorId + 64*(ite->second._iScreen-1), TRUE);//edit by jeckean,64 nocheck
				sRetVec.append("},");
			}
			sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
			sRetVec.append("]}");
			SendJoyStickInf2Tvwall(sRetVec, ite->second._sIp, ite->second._iPort);	
		}

	}

	void DealBtnUp(long theJoystickId, long theButtonNbr)
	{
		//char szlog[MAX_STR_LEN] = {0};
		//sprintf_s(szlog, MAX_STR_LEN, "btnUp %d, %d",theJoystickId, theButtonNbr);
		//m_plog->TraceInfo(szlog);
		//Showlog2Dlg(szlog);
	}

	void DealJoyStickEvent(long theJoystickId,long xValue,long yValue,long zValue)
	{
		DealPtzValue(theJoystickId, xValue, yValue, zValue);
	}

	void DealPtzValue(long theJoystickId,long xValue,long yValue,long zValue)
	{
		if (0 == xValue && 0 == yValue) //zoom
		{
			DealZoomOperate(theJoystickId, zValue);
		}
		else//云台
		{
            DealPtzOperate(theJoystickId, xValue, yValue, zValue);
		}
	}

	void DealZoomOperate(long theJoystickId, long zValue)
	{
		long ivalue = (STEP_LEN_MAX*zValue)/AXIS_ZOOM_MAX;

		Index2Ipc::iterator ite = m_Index2IpcMap.find(m_iIpcSelect);
		if (ite != m_Index2IpcMap.end())
		{
			std::string sRet;
			sRet.append("{");
			SetValue(sRet, "key_id", m_iKeyId);
			SetValue(sRet, "command", "PTZ");
			std::string str = zValue != 0 ? "start" : "stop";
			std::string SubCmd = zValue > 0 ? "ZOOM_ADD" : "ZOOM_REDUCE";
			if(ptzStatus==str)
				return;
			ptzStatus = str;
			SetValue(sRet, "sub_cmd", SubCmd.c_str(), FALSE);
			SetValue(sRet, "state", str.c_str(), false);
			//=====2017/1/5与EIO配合控制=================
			if(m_ipcAutoChoose)
			{
				std::string puid;
				int rect = PuidGet(puid, m_iIpcSelect);
				if(rect)
					puid = ite->second.strIpcPuid;
				SetValue(sRet,"ipc_id",puid.c_str(),FALSE);
			}
			else
			{
			 SetValue(sRet, "ipc_id", ite->second.strIpcPuid.c_str(), FALSE);
			}
			//============================================

			//SetValue(sRet, "monitor_id", ""/*strPUID.c_str()*/, FALSE);
			SetValue(sRet, "stepX", abs(ivalue), FALSE);
			SetValue(sRet, "stepY", 4, FALSE);
			SetjsonEnd(sRet);
			SendJoyStickInf2Tvwall(sRet, ite->second.strTvWallIp, ite->second.TvWallPort);
		}
	}

	void DealPtzOperate(long theJoystickId,long xValue,long yValue,long zValue)
	{
		Index2Ipc::iterator ite = m_Index2IpcMap.find(m_iIpcSelect);
		if (ite != m_Index2IpcMap.end())
		{
			int iPort = ite->second.TvWallPort;
			std::string sIp = ite->second.strTvWallIp;
			//std::string sPuid = ite->second.strIpcPuid;
			std::string sPuid;// = PuidGet(ite,m_iIpcSelect);//获取puid与EIO配合
			int rect = PuidGet(sPuid,m_iIpcSelect);
			if(rect)
				sPuid = ite->second.strIpcPuid;

			long ivalue_x = (STEP_LEN_MAX*xValue)/AXIS_ZOOM_MAX;
			if (ivalue_x == 0)//stop
			{
				if(m_OldCmdState[2].Step != 0 || m_OldCmdState[3].Step != 0)
				{
					std::string strCmd;
					if(0 != m_OldCmdState[2].Step)
					{
						strCmd = "LEFT";
						m_OldCmdState[2].Step = 0;
						m_OldCmdState[2].strState = "stop";
					}
					else
					{
						strCmd = "RIGHT";
						m_OldCmdState[3].Step = 0;
						m_OldCmdState[3].strState = "stop";
					}
					OperatePtz(sIp, iPort, 0, sPuid, "stop", strCmd);
				}
			}
			else//0:LEFT 1:RIGHT
			{
				std::string strCmd;
				int iStep = abs(ivalue_x);
				if(ivalue_x < 0 && m_OldCmdState[2].Step != iStep) //left
				{
					//char szlog[MAX_STR_LEN] = {0};
					//sprintf_s(szlog, MAX_STR_LEN, "ptzSTEP LEFT:%d, %d",iStep, m_OldCmdState[2].Step);
					//m_plog->TraceInfo(szlog);
					strCmd = "LEFT";
					m_OldCmdState[2].Step = iStep;
					m_OldCmdState[2].strState = "start";
					OperatePtz(sIp, iPort, iStep, sPuid, "start", strCmd);
				}
				else if (ivalue_x >0 && m_OldCmdState[3].Step != iStep)
				{
					//char szlog[MAX_STR_LEN] = {0};
					//sprintf_s(szlog, MAX_STR_LEN, "ptzSTEP RIGHT:%d, %d",iStep, m_OldCmdState[3].Step);
					//m_plog->TraceInfo(szlog);
					strCmd = "RIGHT";
					m_OldCmdState[2].Step = iStep;
					m_OldCmdState[2].strState = "start";
					OperatePtz(sIp, iPort, iStep, sPuid, "start", strCmd);
				}
			}

			long ivalue_y = (STEP_LEN_MAX*yValue)/AXIS_ZOOM_MAX;

			if (ivalue_y == 0)//stop
			{
				if(m_OldCmdState[0].Step != 0 || m_OldCmdState[1].Step != 0)
				{
					std::string strCmd;
					if(0 != m_OldCmdState[0].Step)
					{
						strCmd = "UP";
						m_OldCmdState[0].Step = 0;
						m_OldCmdState[0].strState = "stop";
					}
					else
					{
						strCmd = "DOWN";
						m_OldCmdState[1].Step = 0;
						m_OldCmdState[1].strState = "stop";
					}
					OperatePtz(sIp, iPort, 0, sPuid, "stop", strCmd);
				}
			}
			else//0:UP 1:DOWN
			{
				std::string strCmd;
				int iStep = abs(ivalue_y);
				if(ivalue_y < 0 && m_OldCmdState[0].Step != iStep) //UP
				{
					char szlog[MAX_STR_LEN] = {0};
					sprintf_s(szlog, MAX_STR_LEN, "ptzSTEP UP:%d, %d",iStep, m_OldCmdState[0].Step);
					m_plog->TraceInfo(szlog);
					strCmd = "UP";
					m_OldCmdState[0].Step = iStep;
					m_OldCmdState[0].strState = "start";
					OperatePtz(sIp, iPort, iStep, sPuid, "start", strCmd);
				}
				else if (ivalue_y > 0 && m_OldCmdState[1].Step != iStep)
				{
					char szlog[MAX_STR_LEN] = {0};
					sprintf_s(szlog, MAX_STR_LEN, "ptzSTEP RIGHT:%d, %d",iStep, m_OldCmdState[1].Step);
					m_plog->TraceInfo(szlog);
					strCmd = "DOWN";
					m_OldCmdState[1].Step = iStep;
					m_OldCmdState[1].strState = "start";
					OperatePtz(sIp, iPort, iStep, sPuid, "start", strCmd);
				}
			}
		}
	}
	int PuidGet(std::string& sPuid,int select)
	{
		//GetJjhAlarmInfo::Instantialize()->m_nRefresh = 1;
		//std::string str;
		//return str;
		int rect = 1;
		EnterCriticalSection(&GetJjhAlarmInfo::Instantialize()->g_cs);
		if(m_ipcAutoChoose&&GetJjhAlarmInfo::Instantialize()->puidfEIO.size()>=1)
		{
			if(select>GetJjhAlarmInfo::Instantialize()->puidfEIO.size())
				sPuid = GetJjhAlarmInfo::Instantialize()->puidfEIO[0];
			else
			{
				if(m_iIpcSelect>=1)
					sPuid = GetJjhAlarmInfo::Instantialize()->puidfEIO[select-1];
				else
					sPuid = GetJjhAlarmInfo::Instantialize()->puidfEIO[0];
			}
			rect = 0;
		}
		else
			rect = 1;

		LeaveCriticalSection(&GetJjhAlarmInfo::Instantialize()->g_cs);
		return rect;
	}

	void OperatePtz(std::string sIp, int iPort, int iStep, std::string sPuid, std::string sState, std::string sCmd)
	{
		std::string sRet;
		sRet.append("{");
		SetValue(sRet, "key_id", m_iKeyId);
		SetValue(sRet, "command", "PTZ");

		SetValue(sRet, "sub_cmd", sCmd.c_str(), FALSE);
		SetValue(sRet, "state", sState.c_str(), false);
		if(ptzStatus==sState)
			return;
		ptzStatus = sState;
		SetValue(sRet, "ipc_id", sPuid.c_str(), FALSE);
		//SetValue(sRet, "monitor_id", "1000000000004"/*strPUID.c_str()*/, FALSE);
		SetValue(sRet, "stepX", iStep, FALSE);
		SetValue(sRet, "stepY", 0, FALSE);
		SetjsonEnd(sRet);
		SendJoyStickInf2Tvwall(sRet, sIp, iPort);

	}

	void SplitScreen(long theJoystickId, long theButtonNbr)
	{
         //SendJoyStickInf2Tvwall(szlog, "172.16.35.45", 33445);
	}

	bool SendJoyStickInf2Tvwall(std::string sRet, std::string ip, int nPort)
	{
		//通过回调函数实现,链接不存在时容易卡死
		char szlog[MAX_STR_LEN] = {0};
		sprintf_s(szlog, MAX_STR_LEN, "TvwallIp:%s，TvwallIp:%d, info:%s",ip.c_str(), nPort, sRet.c_str());
		Showlog2Dlg(szlog);
		bool bRet = m_InfoCallFun(sRet.c_str(),ip.c_str(), nPort, m_dwDataUser);
		return bRet;	
	}

    static void WorkThreadJoyStick(LPVOID lpParam)
	{
		JoystickManager* pThis = (JoystickManager*)lpParam;
		pThis->Run();
	}

	static JoystickManager *Instantialize()
	{
		if(pInstance == NULL)  
		{   //double check  
			//Lock lock(cs);           //用lock实现线程安全，用资源管理类，实现异常安全  
			//使用资源管理类，在抛出异常的时候，资源管理类对象会被析构，析构总是发生的无论是因为异常抛出还是语句块结束。  
			if(pInstance == NULL)  
			{  
				pInstance = new JoystickManager();  
			}  
		}  
		return pInstance;  
	}
	static void Unstantialize()
	{
		if (pInstance)
		{
			delete pInstance;
		}
	}
	static JoystickManager *pInstance; 

public:
	HRESULT OnStatusChange(long theJoystickId, long theOldStatus, long theNewStatus)
	{
		if (theNewStatus & JOYSTICK_ACTIVE && !(theOldStatus & JOYSTICK_ACTIVE))
		{
			//std::cout<<"Joystick "<<theJoystickId<<" activated.\n";
		}
		return S_OK;
	}
	HRESULT OnButtonUp(long theJoystickId, long theButtonNbr)
	{
		DealButtonEvent(BTN_UP, theJoystickId, theButtonNbr);
		return S_OK;
	}
	HRESULT OnButtonDown(long theJoystickId, long theButtonNbr)
	{
		DealButtonEvent(BTN_DOWN, theJoystickId, theButtonNbr);
		return S_OK;
	}
	HRESULT JoystickMove(long theJoystickId,long xValue,long yValue,long zValue)
	{
		DealJoyStickEvent(theJoystickId, xValue, yValue, zValue);
		return S_OK;
	}

	void Run()
	{

		::CoInitialize(NULL);
		myHandler.CreateInstance(__uuidof(AxisJoystickHandler));

		long aNbrJoysticks = 0;
		myHandler->EnumerateJoysticks(&aNbrJoysticks);
		IAxisJoystick* aJoystick = NULL;
		//char aString[256];
		//std::cout<<"Enter ip address of camera: ";
		//std::cin.getline(aString, 256);
		//std::cout<<"Press <return> to exit\n";
		_bstr_t anURL = _bstr_t("http://") + _bstr_t("0.0.0.0") + bstr_t("/axis-cgi/com/ptz.cgi?");
		if (aNbrJoysticks > 0)
		{
			myHandler->GetJoystick(0, &aJoystick);
			if (aJoystick)
			{
				// Set the URL for sending PTZ commands to the camera
				aJoystick->put_PTZControlURL(anURL);

				// Set username and password to appropriate values if required.
				//aJoystick->put_Username(_bstr_t("").GetBSTR());
				//aJoystick->put_Password(_bstr_t("").GetBSTR());

				aJoystick->Activate();

				// Set hooks for catching the events fired by AxisJoystick
				__hook(&_IAxisJoystickEvents::StatusChange, aJoystick, &JoystickManager::OnStatusChange);
				__hook(&_IAxisJoystickEvents::ButtonDown, aJoystick, &JoystickManager::OnButtonDown);
				__hook(&_IAxisJoystickEvents::ButtonUp, aJoystick, &JoystickManager::OnButtonUp);
				__hook(&_IAxisJoystickEvents::JoystickMove, aJoystick, &JoystickManager::JoystickMove);
			}
		}

		while(m_bJoyAlive)
		{
			Sleep(1000);
		}

		if (aNbrJoysticks > 0 && aJoystick)
		{
			__unhook(&_IAxisJoystickEvents::StatusChange, aJoystick, &JoystickManager::OnStatusChange);
			__unhook(&_IAxisJoystickEvents::ButtonDown, aJoystick, &JoystickManager::OnButtonDown);
			__unhook(&_IAxisJoystickEvents::ButtonUp, aJoystick, &JoystickManager::OnButtonUp);
			__unhook(&_IAxisJoystickEvents::JoystickMove, aJoystick, &JoystickManager::JoystickMove);
			aJoystick->DeActivate();
			aJoystick->Close();
			aJoystick->Release();
		}
		
	}

public:
	unsigned long m_dwDataUser; 
	CallbackFuncRealData m_InfoCallFun;
	CallbackFuncLog m_CallLoginfo;
	Logger *m_plog;
	int m_iLogLevel;
	bool m_bJoyAlive;
	int m_iKeyId;
	int m_iBtnCount;
	int m_iScreenTotal;
	int m_iIpcSelect;
	CmdState m_OldCmdState[4];  //SAVE UP DOWN LEFT RIGHT STATE
	Index2Ipc m_Index2IpcMap;
	MapAxisScreen m_AxisScreenMap;
	bool m_ipcAutoChoose;//是否自动获取绑定的IP（根据EIO）2017/1/4

private:
    //com
	IAxisJoystickHandlerPtr myHandler;
	//info
	HANDLE m_StartJoyStickThread;

}; 
JoystickManager* JoystickManager::pInstance = 0;  
#endif