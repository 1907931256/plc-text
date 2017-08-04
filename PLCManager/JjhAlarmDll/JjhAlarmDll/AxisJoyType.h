#pragma once
#include <map>
typedef struct CmdState
{
	std::string strCmd;
	std::string strState;
	int Step;
	bool bOperate;
	CmdState()
	{
		Step = 0;
		strCmd = "";
		strState = "stop";
		bOperate = false;
	}
}CmdState;

typedef struct IpcItem
{
	std::string strIpcPuid;
	std::string strTvWallIp;
	int TvWallPort;
	IpcItem()
	{
		strIpcPuid = "";
		strTvWallIp = "";
		TvWallPort = 0;
	}
}IpcItem;

typedef std::map<int, IpcItem> Index2Ipc;

typedef struct Ipc2Index
{
	std::string strIpcId;
	int iMonitorId;
    Ipc2Index()
	{
		strIpcId = "";
		iMonitorId = 0;
	}
}Ipc2Index;

typedef std::vector<Ipc2Index> IpcVec;

typedef struct AxisScreen
{
	std::string _sIp;
	int _iPort;
	int _iScreen;
	int _iSplit;
	int _iMode;
	IpcVec _VecIpc;
	AxisScreen()
	{
		_iPort = 0;
		_iScreen = 0;
		_iSplit = 0;
		_iMode = 0;
		_sIp = "";
	}
}AxisScreen;

typedef std::map<int, AxisScreen> MapAxisScreen;