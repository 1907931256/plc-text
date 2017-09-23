
//#include <Winsock2.h>
//#include "GetAxisJoyStick.h"
#include "GetAxisJoyStickCom.h"
#include "GetJjhAlarmInfo.h"
#pragma comment(lib,"ws2_32.lib")

VIXHZ_EXPORT void InitSdk(CallbackFuncRealData CallBackFun, CallbackFuncLog CallBackLogFun, int nRefresh, unsigned long dwDataUser)
{
	GetJjhAlarmInfo::Instantialize()->m_InfoCallFun = CallBackFun;
	GetJjhAlarmInfo::Instantialize()->m_CallLoginfo = CallBackLogFun;
	GetJjhAlarmInfo::Instantialize()->m_dwDataUser = dwDataUser;
	GetJjhAlarmInfo::Instantialize()->m_nRefresh = nRefresh;
	GetJjhAlarmInfo::Instantialize()->InitData();
	if (GetJjhAlarmInfo::Instantialize()->m_sJjhServerIp != "0.0.0.0")
	{
		GetJjhAlarmInfo::Instantialize()->StartReConnect();//开启定时重连线程
		GetJjhAlarmInfo::Instantialize()->StartGetJjhInfo(); //开启50ms读取plc_server数据的线程
	}

	JoystickManager::Instantialize()->m_dwDataUser = dwDataUser;
	JoystickManager::Instantialize()->m_CallLoginfo = CallBackLogFun;
	JoystickManager::Instantialize()->m_InfoCallFun = CallBackFun;
	JoystickManager::Instantialize()->InitData();
	JoystickManager::Instantialize()->StartJoyStickThread(); //获取安讯士摇杆信息
	JoystickManager::Instantialize()->Run();

}

VIXHZ_EXPORT void UnInitSdk()
{
	GetJjhAlarmInfo::Unstantialize();
	JoystickManager::Unstantialize();
}

VIXHZ_EXPORT void SetRevMsgAddr(const char *Ip, int Port, int nKeyOd)
{
}

VIXHZ_EXPORT long LoginPlcServer(const char *Ip, int Port, PLCADDRDataInfo info, int Rack, int Slot)
{
	return GetJjhAlarmInfo::Instantialize()->LogInJjhServer(Ip, Port);
}

VIXHZ_EXPORT void LogOutplcServer(long lLoginId)
{
}

VIXHZ_EXPORT void SetPresetHeightMap(int nSpreader, int nInterval, int nPresetNum)
{
}

VIXHZ_EXPORT void SetPresetIpc(int *pIpcNum, int iNum)
{
	
}

VIXHZ_EXPORT void SetScheme(const char *pscheme)
{
}

VIXHZ_EXPORT void SetGroupsWitch(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm)
{
}

VIXHZ_EXPORT void SetIpcState(const char* pIpcStatelist)
{
}

VIXHZ_EXPORT void HeartBeet()
{

}