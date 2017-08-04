#pragma once

#include "PlcHZ_def.h"

typedef bool (CALLBACK * CallbackFuncRealData)(const char *pStr, const char *Ip, const int nPort, unsigned long dwUser);
typedef void (CALLBACK * CallbackFuncLog)(int ityp, const char *pStr, unsigned long dwUser);

extern "C"
{
	VIXHZ_EXPORT void InitSdk(CallbackFuncRealData CallBackFun, CallbackFuncLog CallBackLogFun, int nRefresh = 50, unsigned long dwDataUser = 0);
	VIXHZ_EXPORT void UnInitSdk();
    VIXHZ_EXPORT long LoginPlcServer(const char *Ip, int Port, PLCADDRDataInfo info, int Rack = 0, int Slot = 2);
	VIXHZ_EXPORT void SetRevMsgAddr(const char *Ip = "0.0.0.0", int Port = 5000, int nKeyOd = 1);
	VIXHZ_EXPORT void LogOutplcServer(long lLoginId);
	VIXHZ_EXPORT void SetPresetHeightMap(int nSpreader = 20, int nInterval = 1, int nPresetNum = 255);
	VIXHZ_EXPORT void SetPresetIpc(int *pIpcNum, int iNum = 0);
	VIXHZ_EXPORT void SetScheme(const char *pscheme);
	VIXHZ_EXPORT void SetGroupsWitch(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm);
	VIXHZ_EXPORT void SetIpcState(const char* pIpcStatelist);
	VIXHZ_EXPORT void HeartBeet();
}