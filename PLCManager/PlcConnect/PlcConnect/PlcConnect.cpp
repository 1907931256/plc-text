
#include <Winsock2.h>
#include "Client.h"
#include "PlcConnect.h"
#include "Dump.h"

VIXHZ_EXPORT void InitSdk(CallbackFuncRealData CallBackFun, CallbackFuncLog CallBackLogFun, int nRefresh, unsigned long dwDataUser)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	WSADATA wsData;
	WORD wVer = MAKEWORD(2,2);
	int iRet = 0;
	iRet = WSAStartup(wVer, &wsData);
	if (iRet != 0)
	{
		OutputDebugString("socket ³õÊ¼»¯Ê§°Ü!");
	}
	//InitMinDump();

	pInstance->m_InfoCallFun = CallBackFun;
	pInstance->m_CallLoginfo = CallBackLogFun;
	pInstance->m_dwDataUser = dwDataUser;
	pInstance->m_nRefresh = nRefresh;
	pInstance->InitModbusOrPlc();
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
	CClientManager* pInstance = CClientManager::GetInstance();
	long lLoginID = pInstance->getClient(Ip, Port, Rack, Slot);
	if(lLoginID)
	{
		CClient* pClient = pInstance->getClient(lLoginID);
		if (pClient)
		{
			pClient->release();
		}
		return lLoginID;
	}
	lLoginID = pInstance->createClient(Ip, Port, info, Rack, Slot);

	if (lLoginID > 0)
	{
		CClient* pClient = pInstance->getClient(lLoginID);
		if (pClient)
		{
			int nRet = pClient->Login(/*Ip, Port, Rack, Slot*/);
			if (nRet<0)
			{
				pClient->release();
				pInstance->remvoeClient(lLoginID);
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

VIXHZ_EXPORT UINT64 GetPLCHeartBeat()
{
	return CClientManager::GetInstance()->m_nPLCHeartBeat;
}
VIXHZ_EXPORT void GroupConfigureRefresh()
{
	return CClientManager::GetInstance()->groupConfigureRefreshCall();
}