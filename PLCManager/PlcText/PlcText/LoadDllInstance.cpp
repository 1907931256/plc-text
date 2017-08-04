#include "stdafx.h"
#include "LoadDllInstance.h"

LoadInstance * LoadInstance::_this = NULL;

LoadInstance::LoadInstance(CString DllName)
{
	_hmydll=::LoadLibrary(DllName);
	init_sdk = NULL;
	uninit_sdk = NULL;
	login_plc_server = NULL;
	set_rev_msg_addr = NULL;
	logout_plc_server = NULL;
	set_preset_height_map = NULL;
    set_preset_ipc = NULL;
    set_scheme = NULL;
	set_group_switch = NULL;
	set_ipc_state = NULL;
	heart_beet = NULL;
	pProcPLCHeartBeat = NULL;
	group_configure_refresh = NULL;
}

void LoadInstance::GetDllFuncAddr()
{
	init_sdk = (INIT_SDK)::GetProcAddress(_hmydll,"InitSdk");
	uninit_sdk = (UNINIT_SDK)::GetProcAddress(_hmydll, "UnInitSdk");
	login_plc_server = (LOGIN_PLC_SERVER)::GetProcAddress(_hmydll, "LoginPlcServer");
	set_rev_msg_addr = (SET_REV_MSG_ADDR)::GetProcAddress(_hmydll, "SetRevMsgAddr");
	logout_plc_server = (LOGOUT_PLC_SERVER)::GetProcAddress(_hmydll, "LogOutplcServer");
	set_preset_height_map = (SETPRERSETHEIGHTMAP)::GetProcAddress(_hmydll, "SetPresetHeightMap");
	set_preset_ipc = (SETPRESETIPC)::GetProcAddress(_hmydll, "SetPresetIpc");
	set_scheme = (SETSCHEME)::GetProcAddress(_hmydll, "SetScheme");
	set_group_switch = (SETGROUPSSITCH)::GetProcAddress(_hmydll, "SetGroupsWitch");
	set_ipc_state = (SETIPCSTATE)::GetProcAddress(_hmydll, "SetIpcState");
	heart_beet = (HEARTBEET)::GetProcAddress(_hmydll, "HeartBeet");
	pProcPLCHeartBeat = (PLCHeartBeat)::GetProcAddress(_hmydll, "GetPLCHeartBeat");
	group_configure_refresh = (GROUP_CONFIGURE_REFRESH)::GetProcAddress(_hmydll,"GroupConfigureRefresh");
}
BOOL LoadInstance::GetLoadSuccess()
{
	return _hmydll != NULL;
}
LoadInstance::~LoadInstance()
{
	if(_hmydll)
	{
		FreeLibrary(_hmydll);
	}

	_hmydll = NULL;
}

LoadInstance* LoadInstance::Instance(CString DllName)
{
	if(_this != NULL)
	{
		return _this;
	}
	_this = new LoadInstance(DllName);
	return _this;
}

void LoadInstance::Uninstance()
{
	if(_this != NULL)
	{
		delete _this;
		_this = NULL;
	}

	return ;
}