#ifndef LOADDLL_INSTANCE
#define LOADDLL_INSTANCE

#define CONNECT_PLC_SUC 1
#define CONNECT_PLC_ERR 2
#define CONNECT_PLC_REC 3
#define CONNECT_PLC_DATA 4 // 2017/03/15 dataViewer
//一般情况下按照默认值
typedef struct PLCADDRDataInfo{
	int nDB;
	int nOffSet;
	int nByteNum;

	int nModeMax;
	int nScreenNum;/*几个屏幕*/


	int nPtzNum;

	int nZoomOffset;/*mode*/
	int nZoom20;
	int nZoom40;
	int nZoom45;

	int nSwitchGroup;/*switchgroup*/


	PLCADDRDataInfo()
	{
		nDB = 501;
		nOffSet = 0;
		nByteNum = 190;

		nModeMax = 4;
		nScreenNum = 6;

		nPtzNum = 33;

		nZoomOffset = 2;

		nZoom20 = 0;
		nZoom40 = 0;
		nZoom45 = 0;

		nSwitchGroup = 0;

	}
}PLCADDRDataInfo;

typedef bool (CALLBACK * CallbackFuncRealData)(const char *pStr, const char *Ip, const int nPort, unsigned long dwUser);
typedef void (CALLBACK * CallbackFuncLog)(int ityp, const char *pStr, unsigned long dwUser);
typedef void (_cdecl *INIT_SDK)(CallbackFuncRealData CallBackFun, CallbackFuncLog CallBackLogFun, int nRefresh, unsigned long dwDataUser);
typedef void (_cdecl *UNINIT_SDK)();
typedef long (_cdecl *LOGIN_PLC_SERVER)(const char *Ip, int Port, PLCADDRDataInfo info, int Rack, int Slot);
typedef void (_cdecl *SET_REV_MSG_ADDR)(const char *Ip, int Port, int nKeyOd);
typedef void (_cdecl *LOGOUT_PLC_SERVER)(long lLoginId);
typedef void (_cdecl *SETPRERSETHEIGHTMAP)(int nSpreader, int nInterval, int nPresetNum);
typedef void (_cdecl *SETPRESETIPC)(int *pIpcNum, int iNum);
typedef void (_cdecl *SETSCHEME)(const char *pscheme);
typedef void (_cdecl *SETGROUPSSITCH)(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm); 
typedef void (_cdecl *SETIPCSTATE)(const char* pIpcStatelist);
typedef void (_cdecl *HEARTBEET)();
typedef  UINT64 (_cdecl *PLCHeartBeat)();
typedef void (_cdecl *GROUP_CONFIGURE_REFRESH)();
typedef void (_cdecl *PTZOPERATIONTEST)(int nGroup,int ipcIndex,int ptzType,int type);

class LoadInstance
{
public:
	LoadInstance(CString DllName);
	virtual~LoadInstance();

public:
	static LoadInstance * Instance(CString DllName = _T(""));
	static void Uninstance();
	void GetDllFuncAddr();
	BOOL GetLoadSuccess();

public:
	static LoadInstance* _this;
	HINSTANCE            _hmydll;
	INIT_SDK             init_sdk;
	UNINIT_SDK           uninit_sdk;
    LOGIN_PLC_SERVER     login_plc_server;
	SET_REV_MSG_ADDR     set_rev_msg_addr;
	LOGOUT_PLC_SERVER    logout_plc_server;
	SETPRERSETHEIGHTMAP  set_preset_height_map;
	SETPRESETIPC         set_preset_ipc;
	SETSCHEME            set_scheme;
	SETGROUPSSITCH       set_group_switch;
	SETIPCSTATE          set_ipc_state;
	HEARTBEET			 heart_beet;
	PLCHeartBeat		 pProcPLCHeartBeat;
	GROUP_CONFIGURE_REFRESH group_configure_refresh;
	PTZOPERATIONTEST  ptz_operation_test;
};

#endif