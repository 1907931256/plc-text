#pragma once
#include <Winsock2.h>
#include "nodavesimple.h"
#include "openSocket.h"
#include "nodave.h"
#include <map>
#include <memory>
#include "PlcHZ_def.h"
#include "Global.h"
#include "IniFile.h"
#include "PlcConnect.h"
#include "log/logger.h"
#include "AX_Mutex.h"
#include "AX_IAddRefAble.h"
#include "AX_Thread.h"
#include "HttpConfig.h"
#include "NetUDP.h"
#include "value.h"
#include "reader.h"
#include "writer.h"
#include <sstream>
#include "modbus.h"
#include "curl.h"
using namespace std::tr1;
//===opc 2017/5/27
#include "opcda.h"
#include "OPCClient.h"
#include "OPCHost.h"
#include "OPCServer.h"
#include "OPCGroup.h"
#include "OPCItem.h"
#include <sys\timeb.h>

class CClient;
typedef std::map<long, CClient*> Clientmap;

#define BLOCK_SIZE_COPY 256  //=====2017/03/15 显示数据时 copy数据的长度
//#define _showData 1
typedef struct PlcScreenAddrInfo
{
	int iAddr;
	int iBit;
	int iPriority;	//优先级
	int iMode;		//模式
	PlcScreenAddrInfo()
	{
		iBit = 0;
		iAddr = 0;
		iPriority = 0;
		iMode = 0;
	}

}PlcScreenAddrInfo;
/* ;SCREENID<<8|MODE ,--->ADDROFFSET,BIT此为唯一性*/
typedef std::map<int, PlcScreenAddrInfo> PlcScreenAddrMap;

/* ;IPCID为唯一性*/
typedef std::map<int, int> PlcIPCAddrMap;

typedef struct TvWallInfo
{
	std::string Ip;
	int Port;
	int iTVSSCREENID;
}TvWallInfo;
/* Group<<8|SCREENID,此为唯一性*/
typedef std::map<int, TvWallInfo> Group2PcMap;

typedef struct stWin_iD
{
	int winNum;
	std::string DeviceId;
	int stream_type;
}WINDOWS_IPCID;
typedef std::vector<WINDOWS_IPCID> stWinIpcId;

typedef struct stScreenModeInfo
{
	int splitNum;
	int windowsIndex;
	stWinIpcId winId;
}ScreenModeInfo;

typedef std::vector<std::string> ZoomIpcVec;

typedef std::vector<std::string> ZoomIpcVector;

//////////////===== 2016/12/18 锁头跟随 ====////////////////
typedef std::vector<std::string> PtzIpcVector;
typedef std::map<std::string,int>IPCgroupID;//相机的组内ID
typedef std::map<int,PtzIpcVector>PtzIpc2GroupMap;
typedef std::map<std::string,int>PiudDefaultPtzPoint;
typedef std::map<std::string,int*>PtzPuid2Ptz;
typedef std::map<std::string,int>stPtzValue;
//////////////////////////////////////////////////////////////

typedef std::map<int, ZoomIpcVector> ZoomIpc2GroupMap;
typedef std::map<int, PlcScreenAddrInfo> ZoomFeet2Addr;
//typedef std::map<int, PlcHeight2ZoomValue> ZoomFeet2height;
typedef std::map<std::string, int> PiudDefaultZoom;
struct  HeightZoomValue
{
	int		nDefaultZoom;
	int		*pZoomValueArray;
	HeightZoomValue(int nDefaultZoom,int nMaxHeight)
	{
		this->nDefaultZoom = nDefaultZoom;
		pZoomValueArray = new int[nMaxHeight];
		ZeroMemory(pZoomValueArray,sizeof(int)*nMaxHeight);
	}
	~HeightZoomValue()
	{
		delete[]pZoomValueArray;
	}
};

typedef std::tr1::shared_ptr<HeightZoomValue> HeightZoomValuePtr;
// Feet与高度和Zoom值的对应参数
typedef map<int ,HeightZoomValuePtr>	FeetHeightZoomValueMap;
typedef shared_ptr<FeetHeightZoomValueMap> FeetHeightZoomValueMapPtr;

typedef std::map<std::string, FeetHeightZoomValueMapPtr> ZoomPuid2Zoom;
typedef std::tr1::shared_ptr<ZoomPuid2Zoom> ZoomPuid2ZoomPtr;

/* Group<<16|SCREENID<<8|MODE,此为唯一性*/
typedef std::map<int, ScreenModeInfo> Screen2GroupPCMap;
typedef std::map<std::string,int> zoomSceneMap;//2017/1/19 随场景变焦选择
//========随高度切屏=============2016/12/11zs============
typedef struct stHeightScreenModeInfo
{
	int minHeight;
	int maxHeight;
	int screenID;
	int mode;
	int group;
}HeightScreenMode;
typedef struct stScreenModeUseInfo
{
	int screenID;
	int mode;
}ScreenModeUse;

//typedef std::vector<ScreenModeUse>screenModeInfo;
typedef std::vector<ScreenModeUse>oldScreenModeStateH;
typedef std::map<int,oldScreenModeStateH>oldScreenModeMap;
typedef	std::vector<HeightScreenMode>heightScreenModeInfo;
typedef std::map<int,bool>switchWithHeightFlag;
//=========================================
//========TextBar 2017/4/7==========================
typedef std::map<int,std::vector<std::string>>IpcText;//group, puid
typedef std::map<int,int>ipcTextF; //goup<<16|SCREEN<<8|MODE
//====-====TextBar 2017/4/17 ADD TEXTBAR MODE========
typedef std::map<std::string,int>textBarInfo;//puid,textBarMode
//========2017/4/17 motify textBarMessage====
typedef std::map<std::string,std::string>ipcPosition;//ipcID,position(WS LS PS)
typedef std::map<int,ipcPosition>IpcPositionMap;//group,position
#define SPREADER_COUNT 3
typedef enum Spreader{WSPREADER=0,LSPREADER,PSPREADER};
typedef struct{
	int iAddress;
	int iBit;
}AddressBoolValue;
typedef struct SpreaderAddress{
	AddressBoolValue mland;
	AddressBoolValue mlock;
	AddressBoolValue mtwinMode;
	AddressBoolValue munlocked;
	std::map<int,AddressBoolValue>mfeet;//Feet Address
	bool addressReady;
	SpreaderAddress(){
		addressReady = false;
	} 
};
typedef struct HeightAddress{ //默认为INT16
	int TargetDistanceAddress;
	int HoistPositionAddress;
	int PHoistPositionAddress;
	int PTrolleyPositionAddress;
	int TrolleyPositionAddress;
	bool addressReady;
	HeightAddress(){
		addressReady = false;
	}
};
typedef struct LiftModeAddress{
	AddressBoolValue ScrInSSMD;
	AddressBoolValue ScrInDSMD;
	AddressBoolValue ScrInPSMD;
	bool addressReady;
	LiftModeAddress(){
		addressReady = false;
	}
};
typedef struct{
	int SpreaderLanded;
	int SpreaderLocked;
	int SpreaderUnlocked;
	int SpeederFeet; 
	int TwinMode;
}SpreaderMessage;
typedef struct{
	float TargetDistance;
	float HositPosition;
	float PHoistPosition;
	float PTrolleyPosition;
	float TrolleyPosition;
}HeightMessage;
typedef struct{
	int ScrInSSMD;
	int ScrInDSMD;
	int ScrInPSMD;
}LiftModeMessage;

typedef struct{
	SpreaderMessage mSpreaderMsg[SPREADER_COUNT];
	HeightMessage mHeightMsg;
	LiftModeMessage mLiftMsg;
}TextBarMessage;
typedef std::map<int,TextBarMessage>textBarInfoMap;
//2017/4/24 增加MONITORID textBar
typedef struct  
{
	int lsMonitorID;
	int wsMonitorID;
	int psMonitorID;
}textBarMonitorID;
typedef std::map<int,textBarMonitorID>textBarMonitorIDMap;
//typedef struct TrolleyHeightAddress{
//	int heightAddress;
//	int length;
//	bool addressReady ;
//	TrolleyHeightAddress()
//	{
//		addressReady = false;
//	}
//};
//typedef struct {
//	int SpreaderLanded;
//	int SpreaderLocked;
//	int SpeederFeet;
//	int TwinMode;
//	float HoistPosition;
//	float TrolleyPosition;
//}TextBarMessage;
typedef std::map<std::string,int>IpcMonitorID;
//============================================
//==2017/5/4 freeModeSwitch 特殊模式点
typedef struct{
	int screenID;
	AddressBoolValue modeAddr;
	int mode;//方案，在[SCREENCAMERA]中配置
}freeModeSt;
typedef std::vector<freeModeSt>freeModeVec;
typedef std::map<int,freeModeVec>freeModeMap;
//==2017/5/24 preset point call
typedef struct{
	int ipcIndex;
	std::string presetPointName;
	bool pointStatuse;
}ipcPresetPointMsg;
typedef std::vector<ipcPresetPointMsg>ipcPresetPointMsgVec;
typedef std::map<int,ipcPresetPointMsgVec>ipcPresetPointMsgMap;
//====2017/5/29 OPC
typedef std::map<int,std::string> OpcGroupMap;
typedef struct
{
	std::string feet20;
	std::string feet40;
	std::string feet45;
	std::string feetDou20;
}OpcItemFeet;

typedef struct IPCIDInfo
{
	std::string Ip;
	int Port;
	std::string strPUID;
	int stream_type;
	int istep;
}IPCIDInfo;
/* Group<<8|IPCID ,此为唯一性*/
typedef std::map<int, IPCIDInfo> GroupIPCIDMap;

typedef std::map<std::string, int> Puid2ZoomVaule;

typedef struct sCutIpc
{
	std::string sCutInfo;
	std::string sIpcMap;
	sCutIpc()
	{
		sCutInfo = "";
		sIpcMap = "";
	}
}sCutIpc;

typedef std::map<int, sCutIpc> ScreenId2Cut;

typedef struct StateScreenId2Cut
{
	int iTvState;
	ScreenId2Cut ScreenId2CutMap;
	StateScreenId2Cut()
	{
		//bOnLine = false;
		//lCountTime = 0;
		iTvState = 0;
	}

}StateScreenId2Cut;

typedef std::map<std::string, StateScreenId2Cut> IpPort2CutMapInfo;

////////////////自由切屏////////////////////////////////////////////////
typedef struct CutScreenInfo
{
	int ScreenID;	
	int IPCIndex;	//IPC序号
	int OffSet;		//位置
	int monitor_ID;
	int SwitchGroup;//上一次切屏值
	std::string strIP;	//上一次发送的目的地址
	int iPort;
	std::string  sIPCPuid;
	int iTVScreenID;
	int nGroup;
	int stream_type;	//主副码流
} CutScreenInfo;
/* Group<<8|INDEX ,此为唯一性*/
typedef std::map<int, CutScreenInfo> CutScreenMap;

typedef struct FreeCutInfo
{
	int nScreenID;
	int CutNum;
}FreeCutInfo;

///////////////////第三方平台接入///////////////////////////////////////////////////
typedef struct ThirdADDRInfo
{
	std::string strIP;
	int Port;
}ThirdADDR;

///////////////////modbus ip addr///////////////////////////////////////////////////////
typedef struct ModBusIp
{
	std::string strIP;
	int nPort;
	int ServerKeyId;
}ModBusIp;
typedef struct ModBusDB
{
	int addr1;			//10000的起始地址，为-1时，则表示该地址不可用
	int addr3;			//30000的起始地址，为-1时，则表示该地址不可用
	int addr4;			//40000的起始地址，为-1时，则表示该地址不可用
}ModBusDB;

typedef struct ModbusADDR
{
	int iAddr;
	int iPriority;
}ModbusADDR;
/* ;SCREENID<<8|MODE ,--->ADDROFFSET,BIT此为唯一性*/
typedef std::map<int, ModbusADDR> ModbusScreenAddrMap;
typedef std::map<int, int> ModbusIPCAddrMap;

typedef std::map<int, int> ModbusZoomFeet2Addr;
typedef std::map<int,std::vector<std::string>>ZoomMualReset; //group CAM_index
class CClientManager
{
public:
	static CClientManager* GetInstance();
	static void UnInstance() {delete GetInstance();}
	/*PLC*/
	long createClient(const char* szIp, int nPort, PLCADDRDataInfo info, int nRack, int nSlot, int nServerKeyID = 0, int nGroup = 0);
	CClient* getClient(long lID);
	long getClient(const char* szIp, int nPort, int nRack, int nSlot);
	void remvoeClient(long lID);
	void SendOldScreeninfo(std::string strIpPort, ScreenId2Cut vec);
	void DealKeepAlive();
	void GetPlcDataInfo();
	void StartReConnect();
	void StopReConnect();
	void StartGetPlcData();
	void StopGetPlcData();
	void StartKeepAlive2TvWall();
	void stopKeepAlive2TvWall();

	CallbackFuncRealData m_InfoCallFun;
	CallbackFuncLog m_CallLoginfo;
	unsigned long m_dwDataUser;

	std::string GetCurrentPath();
	void InitData();
	void InitGroupData();	//初始化组配置文件
	void InitModbusData();	//初始化Modbus配置文件
	void SetIpcState(const char* pIpcStatelist,int group=-1);
	void SetGroupsWitch(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm);
	//====opc Client 2017/5/27
	void InitOpcData();
	void StartGetOpcData();
	void StartReConnectOpc();
	static void *WorkThreadReadOpcData(void *lpParam);
	void StopGetOpcData();
	static void *WorkThreadReConnectOpc(void *lpParam);
	void RunGetOpcData();
	void GetOpcDataInfo();


	/*TVWALL*/
	void ScreenSwitch2TVWALL(int nGroup,int nScreenID, int nMode,int iSwitchGroup);
	void PtzOpration2TVALL(int nGroup, int nIPCIndex, std::string SubCmd,bool isStart, int iSwitchGroup = 0);
	//===============2017/5/27 ptz operate to cam=================
	void PtzOpration2CAM(int nGroup, int nIPCIndex, std::string SubCmd,int cmdIndex, int iSwitchGroup = 0);
	//===============2016/12/18 锁头自动跟随===============================================//
	int sendCgiCommad(const std::string command,int nGroup);
	//=======2017/4/3 cgi setZoom ==========================
	int sendCgiCommadZoom(std::string sPuid, int nZoomValue, int nGroup);
	int CamIpGet(std::string sPuid, int nGroup,std::string & camIp);
	//======2017/4/8 textBar=================================
	bool TextIpcFind(int nGroup,std::string puid);//find:true ,not find:false
	bool TextIpcRetVecGet(std::string& RetVec,int nGroup,std::string puid,int stream_type,int monitorID );
	int textIpcMonitorIdRefresh(std::string puid,int monitor_id);//更新mTextIpcMonitor
	int textBarSendToTVwall(int nGroup,TextBarMessage mMessage,int iSwitchGroup=0);
	//===2017/4/18 textBar ipc Position MonitorID find==
	int positionMonitorIDfind(int nGroup,int* monitorId);
	int textBarModeLoad(std::string& RetVec,int id);


	void IpcZoom2TVWALL(std::string sPuid, int nZoomValue, int iSwitchGroup = 0);
	bool SendInfo2TVWALLServer(std::string sRet, std::string ip, int nPort);

	void Showlog2Dlg(const char *pStr, int itype = 0);
	void SendLog2Dlg(const char *fmt,...);
	void StringSplit(const std::string& src, const std::string& separator, std::vector<std::string>& dest);
	static void *WorkThreadReConnect(void *lpParam);
	static void *WorkThreadReadPlcData(void *lpParam);
	static void *WorkThreadKeepAlive(void *lpParam);
	void RunReConnect();
	void RunGetPlcData();
	void RunKeepAlive();

	//当TVWALL断线重连时发送最新的自由切屏信息
	void SendOldFreeCutScreeninfo(std::string strIpPort);
	void MySleep(const long sec);

	//modbus
	void InitModbusOrPlc();
	void StartReConnectModbus();
	void StopReConnectModbus();
	static void *WorkThreadReConnectModbus(void *lpParam);
	void RunReConnectModbus();
	void StartGetModbusData();
	static void *WorkThreadReadModbusData(void *lpParam);
	void RunGetModbusData();
	void GetModbusDataInfo();
	void StopGetPlcDataModbus();
	void StartKeepAlive2Modbus();
	void stopKeepAlive2Modbus();
	static void *WorkThreadKeepAliveModbus(void *lpParam);
	void RunKeepAliveModbus();
#define EPOCHFILETIME   (116444736000000000UL)
	static UINT64 GetSysTimeMicros()
	{
		// 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
		FILETIME ft;
		LARGE_INTEGER li;
		UINT64 Time64 = 0;
		GetSystemTimeAsFileTime(&ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		// 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
		Time64 = (li.QuadPart - EPOCHFILETIME) /10;
		return Time64;
	}

protected:
	CClientManager();
	~CClientManager();
	static CClientManager* m_pThis;
	uint32 GetSequence(void);
	uint32 m_nIDGenerator;

public:
	BOOL m_bKeepAlive;
	BOOL m_bReConnect;
	BOOL m_bGetPlcData;
	HANDLE m_ReConnectThread;
	HANDLE m_GetPlcDataThread;
	HANDLE m_KeepAliveThread;
	int m_nRefresh; //default 50ms
	Logger *m_plog;

	// PLC心跳,实时刷新,若该值持续一定时间未刷新，则软件将会重起
	volatile UINT64 m_nPLCHeartBeat;

	/* PLC IP,PORT*/
	std::string m_PLCHttpIp;
	int m_nPLCHttpPort;
	int m_KeyId;
	std::string m_TvWallPromat;

	int m_SwitchGroupAddr;
	PlcScreenAddrMap m_PlcScreenAddMap;
	PlcIPCAddrMap m_PlcIPCAddrMap;

	ZoomIpc2GroupMap  m_ZoomIpc2GroupMap;

	////////////////*2016/12/18 锁头跟随*////////////////////////
	PtzIpc2GroupMap m_PtzIpc2GroupMap;
	PiudDefaultPtzPoint m_PuidDefaultPtzPointMap;
	IPCgroupID m_IPCgroupID;
	PtzPuid2Ptz m_ptzPuid2FeetHeightPresentPMap;//<Puid, feet和heighr组成的二维数组>
	///////////////////////////////////////////////////////////////
	ZoomFeet2Addr m_zoomFeet2AddrMap;
	//PiudDefaultZoom m_PuidDefaultZoomMap;

	//ZoomPuid2Zoom m_zoomPuid2FeetHeightMap; //<Puid, feet和heighr组成的二维数组>
	// 根据高度地址不同，对摄像机的进行分组
	map<INT ,ZoomPuid2ZoomPtr> m_zoomPuid2FeetHeightMap;
	// 修改为可以使用多个高度地址  by xionggao.lee@2017.02.16
	vector<PlcScreenAddrInfo> m_ZoomHeightAddr;
	int m_iIpcStateOffSet;
	int m_iMaxHeight;  //high height
	int m_iMinHeight;
	int m_iMaxFeet;    //high feet

	///////////////////新增加的内容
	PLCADDRDataInfo m_PLCFormatInfo;
	// 是否启用优先级
	int m_bEnablePriority;

	std::map<int, int> m_GroupDBMap;/*多个PLC DB */
	Group2PcMap m_Group2PcMap;  //tv_wall服务分组映射
	Screen2GroupPCMap m_GroupPCScreen;  //切屏预案的分组映射
	GroupIPCIDMap m_GroupIpcId;/* ipc list*/

	IpPort2CutMapInfo m_TvWallIp2CutMapInfo;//保存对应tv_wall的切屏预案信息，tv_wal重启时使用

	CutScreenMap m_MapFreeCutScreen;//自由切屏
	int m_nMaxIndex;	//自由切屏最大组序号
	std::map<int, FreeCutInfo> m_GroupScreenMap;//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
	int m_nSendSwitch;		//0：只发送给TVWALL 1：只发送给第三方平台 2:两方都发送
	std::map<int, ThirdADDR> m_MapThirdAddr;	//第三方平台接入IP地址,端口

	/////////新增加modbus/////////////
	MODBUSADDRDataInfo m_ModBusFormatInfo;
	std::map<int, ModBusIp> m_GroupModBusMap;/*多个Modbus ip */
	std::map<int, ModBusDB> m_GroupModBusDBMap;/*Modbus 组内多个数据块 */
	int m_iModbusIpcStateOffSet;
	ModbusScreenAddrMap m_ModbusScreenAddMap;
	ModbusIPCAddrMap m_ModbusIPCAddrMap;
	ModbusZoomFeet2Addr m_ModbuszoomFeet2AddrMap;
	int m_iModbusZoomHeightAddr;
	int m_iModbusSwitchGroupAddr;

	unsigned short m_usHeartBeet;

	BOOL m_bBolModbus;	//当前是否是Modbus控制

	BOOL m_bReConnectModbus;
	HANDLE m_ReConnectThreadModbus;
	HANDLE m_GetModbusDataThread;
	HANDLE m_KeepAliveThreadModbus;
	BOOL m_bGetModbusData;
	BOOL m_bKeepAliveModbus;

	PlcScreenAddrMap m_mapPriority;	//保留最高优先级的模式

	long m_lHeartBeetIndex;
	long m_lHeartBeetTime[50];	//心跳写timeout时间

	//==================随高度切屏=============2016/12/11zs=
	//std::map<int,heightScreenModeInfo> HeightScreenModeMap; //group, vector 
	heightScreenModeInfo m_heightScrrenModeInfo;
	switchWithHeightFlag m_switchWithHeigt;
	oldScreenModeStateH m_oldScreenMode;
	oldScreenModeMap m_oldScreenModeMap;
	//==========================================================
	//=============根据场景ZOOM 2017/1/19======================
	bool m_ZoomChoose;//根据场景启用变焦开关
	int m_ZoomChooseAddr;
	zoomSceneMap m_zoomCameraC;
	//=============在线刷新分组配置============
	bool groupDataRefresh;
	void groupConfigureRefreshCall();
	ZoomMualReset mZoomMualReset;
	AX_Mutex groupDataLock;
	//=======TextBar 2017/4/7================
	IpcText mIpcText;
	ipcTextF mIpcTextScreen;
	SpreaderAddress mSpreaderAddr[SPREADER_COUNT];
	//TrolleyHeightAddress mTrolleyHeightAddr;
	IpcMonitorID mTextIpcMonitor;
	//======textBar message motify 2017/4/17==
	textBarInfo mTextBarInfo;
	IpcPositionMap mIpcPositionMap;
	LiftModeAddress mLiftModeAddr;
	HeightAddress mHeightAddr;
	//===textBar 2017/4/21
	textBarInfoMap mTextBarInfoMap;
	//=====ipc state 2017/4/19
	//bool ipcStateFromAs;
	//========2017/4/22 双吊具模式地址
	AddressBoolValue mDoubleSpreaderAddr;
	map<INT ,ZoomPuid2ZoomPtr> m_zoomPuid2FeetHeightMap2;//双吊具下的zoom值映射

	//======2017/4/24 textBar monitorID;
	textBarMonitorIDMap mTextBarMonitorID;
	//===2017/5/4 特殊模式点切屏
	freeModeMap mFreeModeMap;
	//====2017/5/24 预置点
	AddressBoolValue mPresetpointCallAddr;
	ipcPresetPointMsgMap mIpcPresetPointMsgMap;
	map<int,bool> mPresetPointCallResetMap;
	//==2017/5/29 OPC
	BOOL m_bReConnectOpc;
	BOOL m_bGetOpcData;
	HANDLE m_ReConnectOpcThread;
	HANDLE m_GetOpcDataThread;
	volatile UINT64 m_nOpcHeartBeat;


	std::string mOpcServerName;
	OpcGroupMap mOpcGroupMap;
	std::string mOpcHost;
	std::string mOpcItemHoistPos,mopcItemTrolleyPos;
	OpcItemFeet mOpcItemFeet;
	std::string mOpcHoistUp,mOpcHoistDown;
	std::string mOpcSpreaderLanded,mOpcSpreaderLock,mOpcSpreaderUnlock;
	//std::string mOpcItemFeet20,mOpcItemFeet40,mOpcItemFeet45,mOpcItemFeetDou20;
	//std::string mopcItemTrolleyPos;
	//std::string mOpcItemFeet20;
	//std::string mOpcItemFeet40;
	//std::string mOpcItemFeet45;
	//std::string mOpcItemFeetDou20;
	//===========TCP/IP Connect PLC 2017/3/27=======
	//bool tcpConnect;
	//SOCKADDR_IN addrSrv;
	//SOCKET sockClient;
	//====2017/5/3 优先级开关=====
	//bool priorityEnable;

private:
	AX_Mutex m_lockClient;	
	Clientmap m_clients;
};

class CClient : public AX_IAddRefAble
{

public:
	~CClient(void);
	int  Login(/*const char *Ip, int nPort,  int nRack, int nSlot*/);
	int  LoginModbus(/*const char *Ip, int nPort,  int nRack, int nSlot*/);
	int  Logout();
	bool isSameDevice(const char* szIP, int nPort, int nRack, int nSlot);

	void ReadPLCDataProcess();
	void ReadModbusDataProcess();
	void WriteIpcState(const char* pIpcStatelist);
	void CheckWriteIpcState(int iGroup, int iIndex, int iState);

	wchar_t* AnsiToUnicode(const char* buf);
	char* UnicodeToUtf8(const wchar_t* buf);


	void ClearPriority(int nGroup);
	void CloseAllIpcInGroup(int iGroup, int iMode = 0);
	void SetPlcServerInfo(std::string sIp, int nPort = 102, int nRack = 0, int nSlot = 2, int nCurrentGroup = 0, int ServerKeyId = 0);
	void InitOldState();
	void DeleteOldState();
	void SwitchScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcPtzOperation(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcZoomOperation(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	//随高度变化自动切屏,使用第一组高度
	void SwitchScreenHeight(int nGroup,const unsigned char *pBuf, int iSwitchGroup=0); 

	//2016/12/18 锁头跟随默认，使用第一组高度
	void IpcPtzFollowOperation(int nGroup,const unsigned char*pBuf,int iSwitchGroup=0);
	//=======2017/4/8 textBar=================================
	int textBarDataSend(int nGroup,const unsigned char*pBuff,int iSwitchGroup=0);
	//=======2017/4/18 textBar message======
	int textBarSpreaderMessage(int nGroup,const unsigned char*pBuf,SpreaderMessage* mMsg);
	int textBarHeightMessage(int nGroup,const unsigned char*pBuf,HeightMessage& mMsg);
	int textBarLiftModeMessage(int nGroup,const unsigned char*pBuf,LiftModeMessage& mMsg);
	//2017/4/22 motify 增加双吊具模式 mode==1
	int  GetValueFromZoomMap(std::string sPuid, int iFeet,int nHeightGroup, int iHeight,int mode);
	bool ZoomAutoManual(int nGroup , const unsigned char *pBuf);
	//2017/5/3 自由模式切换 
	int FreeModeSwitch(int nGroup,const unsigned char*pBuf,int iSwitchGroup=0);
	//2017/5/3 preset point call
	void PreserPointCall(int nGroup,const unsigned char*pBuff,int iSwitchGroup=0);
	void FreeCutScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void FreeCutScreenToTVwall(int nGroup, unsigned short iIPC, int iMapIndex, int i, int iSwitchGroup = 0);
	void SendToThirdDevice(int nGroup, const unsigned char *pBuff, int nDB, int nOffSet, int nByteNum);  //发送给第三方平台

	void SwitchScreenModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcPtzOperationModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcZoomOperationModbus(int nGroup, const unsigned char *pBuf,const unsigned short *buf, int iSwitchGroup = 0);
	void CheckWriteIpcStateModbus(int iGroup, int iIndex, int iState);

	void HeartBeetModbus();
	bool AutoZoom(int nGroup, string strPUID);
	bool sceneZoom(const unsigned char*pBuf,std::string puid);//根据场景ZOOM 判断 2017/1/9
protected:
	CClient(CClientManager *pClientMgr, PLCADDRDataInfo info);

	std::string m_strIp;
	int m_nPort;
	int m_nRack;
	int m_nSlot;


	int m_nNumIndex;	//发包序号
	CNetUDP net;

	PLCADDRDataInfo m_PLCFormatInfo;/* plc格式内容，主要是读取参数*/

	unsigned short m_stGroupSwitchState[32];
	std::map<int, unsigned short *> m_MapScreenState;/*screen operation status, group,screen,mode*/
	unsigned short m_stPtzState[32][64];/*ptz operation status, group,ptz,mode*/
	stPtzValue m_stPtzPresentPosition; //2016/12/18 ptz value 锁头跟随，ptz值
	unsigned short m_stIpcOnlineState[32][64];/*ipc croup, ipc index*/
	Puid2ZoomVaule m_GroupZoomState; /*zoom state*/
	//std::map<int, unsigned short *> m_MapPtzState;
	std::map<int, unsigned char *> m_MapRecvData;	//收到的数据

	bool m_bConnect;
	bool m_bSentFirst;			// 是否已经第一次发送
	_daveOSserialType _fds;
	daveInterface * _di;
	daveConnection * _dc;

	CClientManager *m_pClientMgr;

	friend class CClientManager;

	//Modbus
	int m_nCurrentTime;
	int m_nGroup;	//当前组
	bool m_bZoomAuto[32][64][1]; //自动跟随开关
	modbus_t *m_ctx;
	unsigned short m_stModbusIpcOnlineState[32][64];/*ipc croup, ipc index*/
	unsigned short m_stModbusPtzState[32][64][7];/*ptz operation status, group,ptz,mode*/
	int m_iServerKeyID;
	//====2017/5/29 OPC
	bool m_bConnectOpc;
	COPCHost *mOpcHost;
	COPCServer *mOpcServer; 
	ServerStatus mOpcStatus;
	void loginOpcServer();

	void ReadOpcDataProcess();

};
