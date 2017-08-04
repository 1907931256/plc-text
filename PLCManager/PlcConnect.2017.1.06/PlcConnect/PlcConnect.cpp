
#include <Winsock2.h>
#include "Client.h"
#include "PlcConnect.h"
#include "Dump.h"

VIXHZ_EXPORT void InitSdk(CallbackFuncRealData CallBackFun, CallbackFuncLog CallBackLogFun, int nRefresh, unsigned long dwDataUser)
{
	WSADATA wsData;
	WORD wVer = MAKEWORD(2,2);
	int iRet = 0;
	iRet = WSAStartup(wVer, &wsData);
	if (iRet != 0)
	{
		OutputDebugString("socket ³õÊ¼»¯Ê§°Ü!");
	}
	InitMinDump();

	CClientManager::GetInstance()->m_InfoCallFun = CallBackFun;
	CClientManager::GetInstance()->m_CallLoginfo = CallBackLogFun;
	CClientManager::GetInstance()->m_dwDataUser = dwDataUser;
	CClientManager::GetInstance()->m_nRefresh = nRefresh;
	CClientManager::GetInstance()->InitModbusOrPlc();
}

VIXHZ_EXPORT void UnInitSdk()
{
	WSACleanup();
	CClientManager::UnInstance();
}

VIXHZ_EXPORT void SetRevMsgAddr(const char *Ip, int Port, int nKeyOd)
{
}

VIXHZ_EXPORT long LoginPlcServer(const char *Ip, int Port, PLCADDRDataInfo info, int Rack, int Slot)
{
	long lLoginID = CClientManager::GetInstance()->getClient(Ip, Port, Rack, Slot);
	if(lLoginID)
	{
		CClient* pClient = CClientManager::GetInstance()->getClient(lLoginID);
		if (pClient)
		{
			pClient->release();
		}
		return lLoginID;
	}
	lLoginID = CClientManager::GetInstance()->createClient(Ip, Port, info, Rack, Slot);

	if (lLoginID > 0)
	{
		CClient* pClient = CClientManager::GetInstance()->getClient(lLoginID);
		if (pClient)
		{
			int nRet = pClient->Login(/*Ip, Port, Rack, Slot*/);
			if (nRet<0)
			{
				pClient->release();
				CClientManager::GetInstance()->remvoeClient(lLoginID);
				return nRet;
			}
			pClient->release();

			return lLoginID; 
		}
	}
	return /*RV_ET_LOGIN_ERROR_NETWORK*/0;
}

VIXHZ_EXPORT void LogOutplcServer(long lLoginId)
{
	CClient* pClient = CClientManager::GetInstance()->getClient(lLoginId);
	if (pClient)
	{
		pClient->Logout();
		pClient->release();
		CClientManager::GetInstance()->remvoeClient(lLoginId);
	}
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
    CClientManager::GetInstance()->SetGroupsWitch(iGroupNum, iScreenNum, iModeNum, iScreenNUm);
}

VIXHZ_EXPORT void SetIpcState(const char* pIpcStatelist)
{
	CClientManager::GetInstance()->SetIpcState(pIpcStatelist);
}
VIXHZ_EXPORT void HeartBeet()
{
	CClientManager::GetInstance()->DealKeepAlive();
}