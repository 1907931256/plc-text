#pragma once
#include <stdafx.h>
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
//===opc 2017/8/9
#include "opcserver.h"
#include <algorithm>

const CLSID CLSID_OPCServerList = {0x13486D51,0x4821,0x11D2,{0xA4,0x94,0x3C,0xB3,0x06,0xC1,0x00,0x00}};
#define __GUID_DEFINED__
#include "OPCDa_Cats.c"
//#include "opcda.h"
//#include "OPCClient.h"
//#include "OPCHost.h"
//#include "OPCServer.h"
//#include "OPCGroup.h"
//#include "OPCItem.h"
//#include <sys\timeb.h>
#include <errno.h>
#include <signal.h>
#include <ws2tcpip.h>
#include <winsock.h>
#pragma comment(lib,"winsock.lib")
#pragma comment(lib,"ws2_32.lib")



#include "..\IPNBSSDK\IPNBSSDK_Face.h"
#pragma comment (lib, "..//IPNBSSDK//IPNBSSDK.lib")
#pragma warning(disable:4018)

static modbus_t *mModbusServer;
static modbus_mapping_t *mbMapping;
static bool mModbusServerListen;
static int mModbusServerSocket;


class CClient;
typedef std::map<long, CClient*> Clientmap;

#define BLOCK_SIZE_COPY 256  //=====2017/03/15 ��ʾ����ʱ copy���ݵĳ���
#define _showData 1
//#define GET_READPLC_TIME
#define MODBUS_NB_CONNECTION 5 //modbus server


//====PTZ Ctrl 2017/6/27
typedef struct  
{
	int				nDVRPort;
	std::string		userName;
	std::string		password;
	std::string     pchDVRIP;
}LoginDevInfo;
typedef enum
{
	PTZ_Up = 0,
	PTZ_Down,
	PTZ_Left,
	PTZ_Right,
	PTZ_ZoomAdd,   //�䱶
	PTZ_ZoomDec,
	PTZ_FocusAdd,  //�۽�
	PTZ_FocusDel,
	PTZ_ApeAdd,    //��Ȧ
	PTZ_ApeDel,
	PTZ_PointMove,
	PTZ_PointSet,
	PTZ_PointDel,
	PTZ_PointLoop,		// Ѳ���鿪�����ر�
	PTZ_Lamp,           // �ƹ���ˢ
	PTZ_RedLight,       // ���⣬��������ͨ��AUXIȥ����

	PTZ_LeftUp = 0X20,
	PTZ_RightUp ,
	PTZ_LeftDown,
	PTZ_RightDown,
	PTZ_AddtoLoop,
	PTZ_DelFromLoop,
	PTZ_CloseLoop,
	PTZ_StartPanCruise,
	PTZ_StopPanCruise,
	PTZ_SetLeftBorder,
	PTZ_SetRightBorder,
	PTZ_StartLineScan,
	PTZ_CloseLineScan,
	PTZ_SetModesStart,
	PTZ_SetModesStop,
	PTZ_RunMode,
	PTZ_StopMode,
	PTZ_DeleteMode,
	PTZ_ReverseComm,
	PTZ_FastGoto,
	PTZ_AuxiOpen,
	PTZ_AuxiClose,
	PTZ_OpenMenu,
	PTZ_CloseMenu,
	PTZ_MenuOk,
	PTZ_MenuCancel,
	PTZ_MenuUp,
	PTZ_MenuDown,
	PTZ_MenuLeft,
	PTZ_MenuRight,
	PTZ_AlarmHandle = 0x40,
	PTZ_MatrixSwitch = 0x41,
	PTZ_LightControl,
	PTZ_Total,
	PTZ_ZoomSet
}PTZ_Ctrl;

typedef struct
{
	long lParam1; //�������� �ٶ�
	long lParam2; //ZOOM �ٶ�,��С
	long lParam3;
	std::string presetpointName;
	bool bStop;
	bool bGet;
}Vix_PtzCfgParam;

typedef struct PlcScreenAddrInfo
{
	int iAddr;
	int iBit;
	int iPriority;	//���ȼ�
	int iMode;		//ģʽ
	PlcScreenAddrInfo()
	{
		iBit = 0;
		iAddr = 0;
		iPriority = 0;
		iMode = 0;
	}

}PlcScreenAddrInfo;
/* ;SCREENID<<8|MODE ,--->ADDROFFSET,BIT��ΪΨһ��*/
typedef std::map<int, PlcScreenAddrInfo> PlcScreenAddrMap;

/* ;IPCIDΪΨһ��*/
typedef std::map<int, int> PlcIPCAddrMap;

typedef struct TvWallInfo
{
	std::string Ip;
	int Port;
	int iTVSSCREENID;
}TvWallInfo;
/* Group<<8|SCREENID,��ΪΨһ��*/
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

//////////////===== 2016/12/18 ��ͷ���� ====////////////////
typedef std::vector<std::string> PtzIpcVector;
typedef std::map<std::string,int>IPCgroupID;//���������ID
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
// Feet��߶Ⱥ�Zoomֵ�Ķ�Ӧ����
typedef map<int ,HeightZoomValuePtr>	FeetHeightZoomValueMap;
typedef shared_ptr<FeetHeightZoomValueMap> FeetHeightZoomValueMapPtr;

typedef std::map<std::string, FeetHeightZoomValueMapPtr> ZoomPuid2Zoom;
typedef std::tr1::shared_ptr<ZoomPuid2Zoom> ZoomPuid2ZoomPtr;

/* Group<<16|SCREENID<<8|MODE,��ΪΨһ��*/
typedef std::map<int, ScreenModeInfo> Screen2GroupPCMap;
typedef std::map<std::string,int> zoomSceneMap;//2017/1/19 �泡���佹ѡ��
//========��߶�����=============2016/12/11zs============
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
typedef struct HeightAddress{ //Ĭ��ΪINT16
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
//2017/4/24 ����MONITORID textBar
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
//==2017/5/4 freeModeSwitch ����ģʽ��
typedef struct{
	int screenID;
	AddressBoolValue modeAddr;
	int mode;//��������[SCREENCAMERA]������
}freeModeSt;
typedef std::vector<freeModeSt>freeModeVec;
typedef std::map<int,freeModeVec>freeModeMap;
//==2017/5/24 preset point call
typedef struct{
	int ipcIndex;
	std::string presetPointName;
	bool pointStatuse;
	int erroTime;
}ipcPresetPointMsg;
typedef std::vector<ipcPresetPointMsg>ipcPresetPointMsgVec;
typedef std::map<int,ipcPresetPointMsgVec>ipcPresetPointMsgMap;

typedef struct IPCIDInfo
{
	std::string Ip;
	int Port;
	std::string strPUID;
	int stream_type;
	int istep;
}IPCIDInfo;
/* Group<<8|IPCID ,��ΪΨһ��*/
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

////////////////��������////////////////////////////////////////////////
typedef struct CutScreenInfo
{
	int ScreenID;	
	int IPCIndex;	//IPC���
	int OffSet;		//λ��
	int monitor_ID;
	int SwitchGroup;//��һ������ֵ
	std::string strIP;	//��һ�η��͵�Ŀ�ĵ�ַ
	int iPort;
	std::string  sIPCPuid;
	int iTVScreenID;
	int nGroup;
	int stream_type;	//��������
} CutScreenInfo;
/* Group<<8|INDEX ,��ΪΨһ��*/
typedef std::map<int, CutScreenInfo> CutScreenMap;

typedef struct FreeCutInfo
{
	int nScreenID;
	int CutNum;
}FreeCutInfo;

///////////////////������ƽ̨����///////////////////////////////////////////////////
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
	int addr1;			//10000����ʼ��ַ��Ϊ-1ʱ�����ʾ�õ�ַ������
	int addr3;			//30000����ʼ��ַ��Ϊ-1ʱ�����ʾ�õ�ַ������
	int addr4;			//40000����ʼ��ַ��Ϊ-1ʱ�����ʾ�õ�ַ������
}ModBusDB;

typedef struct ModbusADDR
{
	int iAddr;
	int iPriority;
}ModbusADDR;
/* ;SCREENID<<8|MODE ,--->ADDROFFSET,BIT��ΪΨһ��*/
typedef std::map<int, ModbusADDR> ModbusScreenAddrMap;
typedef std::map<int, int> ModbusIPCAddrMap;

typedef std::map<int, int> ModbusZoomFeet2Addr;
typedef std::map<int,std::vector<std::string>>ZoomMualReset; //group CAM_index

//==2017/8/9 OPC SET nanSha
typedef std::map<std::string,Item*> OpcItemMap; //OPC Item (name,item)
//typedef std::map<int,std::string>RosSwitchGroupMap; //int ROS���� string��Ӧ��OPCITEM
//typedef std::map<int,std::string>OpcFeetMap; //feet���� itemName (20,2020,40,45)
//typedef std::map<int,std::string>OpcScreenModeMap ;// ģʽ���� itemName 
typedef std::map<int,std::string>OpcValueItemNameMap; 

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
	void InitGroupData();	//��ʼ���������ļ�
	void InitModbusData();	//��ʼ��Modbus�����ļ�
    void SetIpcState(const char* pIpcStatelist);
	void SetGroupsWitch(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm);

    /*TVWALL*/
	void ScreenSwitch2TVWALL(int nGroup,int nScreenID, int nMode,int iSwitchGroup);
	void PtzOpration2TVALL(int nGroup, int nIPCIndex, std::string SubCmd,bool isStart, int iSwitchGroup = 0);
	//===============2017/5/27 ptz operate to cam=================
	bool PtzOpration2CAM(int nGroup, int nIPCIndex, bool isStart,int cmdIndex, int iSwitchGroup = 0);
	bool PtzCmdCtrl(LoginDevInfo logDevice, PTZ_Ctrl lConfigType, Vix_PtzCfgParam nParam); 
	//===============2016/12/18 ��ͷ�Զ�����===============================================//
	int sendCgiCommad(const std::string command,int nGroup);
	//=======2017/4/3 cgi setZoom ==========================
	int sendCgiCommadZoom(std::string sPuid, int nZoomValue, int nGroup);
	int CamIpGet(std::string sPuid, int nGroup,std::string & camIp);
	//======2017/4/8 textBar=================================
	bool TextIpcFind(int nGroup,std::string puid);//find:true ,not find:false
	bool TextIpcRetVecGet(std::string& RetVec,int nGroup,std::string puid,int stream_type,int monitorID );
	int textIpcMonitorIdRefresh(std::string puid,int monitor_id);//����mTextIpcMonitor
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

	//��TVWALL��������ʱ�������µ�����������Ϣ
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
		// ��1601��1��1��0:0:0:000��1970��1��1��0:0:0:000��ʱ��(��λ100ns)
		FILETIME ft;
		LARGE_INTEGER li;
		UINT64 Time64 = 0;
		GetSystemTimeAsFileTime(&ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		// ��1970��1��1��0:0:0:000�����ڵ�΢����(UTCʱ��)
		Time64 = (li.QuadPart - EPOCHFILETIME) /10;
		return Time64;
	}

	//==2017/6/7
	void StartModbusServer();
	static void *WorkThreadModbusServer(void *lpParam);
	void RunModbusServer();
	static void ModbusCloseSigint(int dummy);
	void stopModbusServer();
	//static void *WorkThreadModbusServerRec(void *lpParam);
	//==2017/8/6 
	void ptzOperationTest(int nGroup,int ipcIndex,int ptzType,int type);

	//===2017/8/9  OPC nanShan
	void opcInit(); //load opcģʽ�µ�����
	void readConfigIntString(CIniFile iniFile,std::string groupName,std::string valueName,OpcValueItemNameMap& valueMap);
	void readConfigIntStringVec(CIniFile iniFile,std::string groupName,OpcValueItemNameMap& valueMap);
	void GetOpcDataInfo();
	void RunGetOpcData();
	void StartReConnectOpc();
	void RunReConnectOpc();
	void StopReConnectOpc();
	void StartGetOpcData();
	void StopGetOpcData();
	void opcServerConnect();
	static void *WorkThreadReadOpcData(void *lpParam);
	static void *WorkThreadReConnectOpc(void *lpParam);
	wchar_t * UTF8ToUnicode( const char* str );
	void opcItemAdd(OpcValueItemNameMap ItemNameMap,int mode=0); //mode=0 ȫ��� 1:���rmg, 2:���accs
	void opcSingleItemAdd(Item* item,std::string name);
    bool opcGroupConnectAlways(int nGroup);
	//void StopGetOpcData();

	//===2017/8/18 ��Ƶ�л�
	std::string mAudioServerIp;
	uint mAudioServerPort,mAudioServerStatusPort;
	//std::string mOpcAudio; //��Ƶ�л���OPC itemName
	std::map<int,int>mAudioRosIdMap;//�����豸IDӳ�� <ROS,AUDIO>
	std::map<int,int>mAudioDeviceIdMap;//<DEVICE,AUDIO>
	BOOL mAudioEnable;
	OpcValueItemNameMap mAudioIdItemMap;
	void readAudioConfigure();
	void AudioServerSet();
	char* UnicodeToUtf8(const wchar_t* buf);
	LRESULT OnRecvStatus(WPARAM wParam, LPARAM lParam);
	static void CALLBACK OnStatusCallBack(DWORD dwInstance, WPARAM wParam, LPARAM lParam);


protected:
	CClientManager();
	~CClientManager();
	static CClientManager* m_pThis;
	uint32 GetSequence(void);
	uint32 m_nIDGenerator;

public:
    BOOL ipcStaFromAs300; 

	BOOL m_bKeepAlive;
	BOOL m_bReConnect;
	BOOL m_bGetPlcData;
	HANDLE m_ReConnectThread;
	HANDLE m_GetPlcDataThread;
	HANDLE m_KeepAliveThread;
	int m_nRefresh; //default 50ms
	Logger *m_plog;

	//===2017/6/7 modbus server
	//static modbus_t *mModbusServer;
	//static modbus_mapping_t *mbMapping;
	//bool mModbusServerListen;
	//static int mModbusServerSocket;
	bool mModbusServerCreateF,mModbusZoomOnly;
	int mZoomOnlyHeightAddr,mZoomOnlyFeetAddr;
	HANDLE m_ModbusServerThread;

	// PLC����,ʵʱˢ��,����ֵ����һ��ʱ��δˢ�£��������������
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

	////////////////*2016/12/18 ��ͷ����*////////////////////////
	PtzIpc2GroupMap m_PtzIpc2GroupMap;
	PiudDefaultPtzPoint m_PuidDefaultPtzPointMap;
	IPCgroupID m_IPCgroupID;
	PtzPuid2Ptz m_ptzPuid2FeetHeightPresentPMap;//<Puid, feet��heighr��ɵĶ�ά����>
	///////////////////////////////////////////////////////////////
	ZoomFeet2Addr m_zoomFeet2AddrMap;
	//PiudDefaultZoom m_PuidDefaultZoomMap;

	//ZoomPuid2Zoom m_zoomPuid2FeetHeightMap; //<Puid, feet��heighr��ɵĶ�ά����>
	// ���ݸ߶ȵ�ַ��ͬ����������Ľ��з���
	map<INT ,ZoomPuid2ZoomPtr> m_zoomPuid2FeetHeightMap;
	// �޸�Ϊ����ʹ�ö���߶ȵ�ַ  by xionggao.lee@2017.02.16
	vector<PlcScreenAddrInfo> m_ZoomHeightAddr;
	int m_iIpcStateOffSet;
	int m_iMaxHeight;  //high height
	int m_iMinHeight;
	int m_iMaxFeet;    //high feet

	///////////////////�����ӵ�����
	PLCADDRDataInfo m_PLCFormatInfo;
	// �Ƿ��������ȼ�
	int m_bEnablePriority;

	std::map<int, int> m_GroupDBMap;/*���PLC DB */
	Group2PcMap m_Group2PcMap;  //tv_wall�������ӳ��
	Screen2GroupPCMap m_GroupPCScreen;  //����Ԥ���ķ���ӳ��
    GroupIPCIDMap m_GroupIpcId;/* ipc list*/

	IpPort2CutMapInfo m_TvWallIp2CutMapInfo;//�����Ӧtv_wall������Ԥ����Ϣ��tv_wal����ʱʹ��

	CutScreenMap m_MapFreeCutScreen;//��������
	int m_nMaxIndex;	//����������������
	std::map<int, FreeCutInfo> m_GroupScreenMap;//���ǵ��ķ���ģʽ�л���������ʱ����Ӧ�÷��ĸ���������
	int m_nSendSwitch;		//0��ֻ���͸�TVWALL 1��ֻ���͸�������ƽ̨ 2:����������
	std::map<int, ThirdADDR> m_MapThirdAddr;	//������ƽ̨����IP��ַ,�˿�

	/////////������modbus/////////////
	MODBUSADDRDataInfo m_ModBusFormatInfo;
	std::map<int, ModBusIp> m_GroupModBusMap;/*���Modbus ip */
	std::map<int, ModBusDB> m_GroupModBusDBMap;/*Modbus ���ڶ�����ݿ� */
	int m_iModbusIpcStateOffSet;
	ModbusScreenAddrMap m_ModbusScreenAddMap;
	ModbusIPCAddrMap m_ModbusIPCAddrMap;
	ModbusZoomFeet2Addr m_ModbuszoomFeet2AddrMap;
	int m_iModbusZoomHeightAddr;
	int m_iModbusSwitchGroupAddr;

	unsigned short m_usHeartBeet;

	BOOL m_bBolModbus;	//��ǰ�Ƿ���Modbus����

	BOOL m_bReConnectModbus;
	HANDLE m_ReConnectThreadModbus;
	HANDLE m_GetModbusDataThread;
	HANDLE m_KeepAliveThreadModbus;
	BOOL m_bGetModbusData;
	BOOL m_bKeepAliveModbus;

	PlcScreenAddrMap m_mapPriority;	//����������ȼ���ģʽ

	long m_lHeartBeetIndex;
	long m_lHeartBeetTime[50];	//����дtimeoutʱ��

	//==================��߶�����=============2016/12/11zs=
	//std::map<int,heightScreenModeInfo> HeightScreenModeMap; //group, vector 
	heightScreenModeInfo m_heightScrrenModeInfo;
	switchWithHeightFlag m_switchWithHeigt;
	oldScreenModeStateH m_oldScreenMode;
	oldScreenModeMap m_oldScreenModeMap;
	//==========================================================
	//=============���ݳ���ZOOM 2017/1/19======================
	bool m_ZoomChoose;//���ݳ������ñ佹����
	int m_ZoomChooseAddr;
	zoomSceneMap m_zoomCameraC;
	//=============����ˢ�·�������============
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
	//========2017/4/22 ˫����ģʽ��ַ
	AddressBoolValue mDoubleSpreaderAddr;
	map<INT ,ZoomPuid2ZoomPtr> m_zoomPuid2FeetHeightMap2;//˫�����µ�zoomֵӳ��

	//======2017/4/24 textBar monitorID;
	textBarMonitorIDMap mTextBarMonitorID;
	//===2017/5/4 ����ģʽ������
	freeModeMap mFreeModeMap;
	//====2017/5/24 Ԥ�õ�
	AddressBoolValue mPresetpointCallAddr;
	ipcPresetPointMsgMap mIpcPresetPointMsgMap;
	map<int,bool> mPresetPointCallResetMap;
	//===2017/6/2  ���������м�¼ģʽ����������
	std::map<int,bool>mGroupSwitchFirst;
	//==2017/6/9 PTZָ��ֱ�ӷ��͵����
	bool mPtzCommandToCam;

	int ptzOperationCmdOld;
	//===========TCP/IP Connect PLC 2017/3/27=======
	//bool tcpConnect;
	//SOCKADDR_IN addrSrv;
	//SOCKET sockClient;
	//====2017/5/3 ���ȼ�����=====
	//bool priorityEnable;
	//=========2017/8/9 opc nanSha
	std::string mOpcServerName;
	COPCServer* mOpcServer;
	COPCGroup* mOpcGroup;
	OpcItemMap mOpcItemMap; //OPC Item (name,item)
	OpcValueItemNameMap mOpcRosMap,mOpcRmgMap,mOpcFeetMap,mOpcModeMap,mOpcRccsFeetMap,mOpcPtzManualMap;  //mRosMap :ros��Ӧ��item,mOpcRmgMap
	std::vector<int> mOpcConnetAlwaysRos;
	//OpcFeetMap mOpcFeetMap; //feet���� itemName (20,2020,40,45)
    //OpcScreenModeMap mOpcModeMap;// ģʽ���� itemName 
	std::string mOpcHeight,mOpcRccsHeight;
	bool m_bGetOpcData ; 
	bool m_bReconnectOpc,mConnectOpc;
	HANDLE m_ReConnectOpcThread;
	HANDLE m_GetOpcDataThread;
	bool mOpcFlag;
	std::map<int,std::vector<int>> mModeNoZoomMap; //�ڴ�ģʽ�²����б佹
	std::map<int,std::vector<int>> mSpecialModeZoomMap; //��ģʽ��ִ��ZOOM2

private:
	AX_Mutex m_lockClient;	
	Clientmap m_clients;
	//===2017/6/7 modbus server
	AX_Mutex m_lockModbusServer;
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
	void WriteIpcStateFromAs300(const char* pIpcStatelist);
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
	//��߶ȱ仯�Զ�����,ʹ�õ�һ��߶�
	void SwitchScreenHeight(int nGroup,const unsigned char *pBuf, int iSwitchGroup=0); 

	//2016/12/18 ��ͷ����Ĭ�ϣ�ʹ�õ�һ��߶�
	void IpcPtzFollowOperation(int nGroup,const unsigned char*pBuf,int iSwitchGroup=0);
	//=======2017/4/8 textBar=================================
	int textBarDataSend(int nGroup,const unsigned char*pBuff,int iSwitchGroup=0);
	//=======2017/4/18 textBar message======
	int textBarSpreaderMessage(int nGroup,const unsigned char*pBuf,SpreaderMessage* mMsg);
	int textBarHeightMessage(int nGroup,const unsigned char*pBuf,HeightMessage& mMsg);
	int textBarLiftModeMessage(int nGroup,const unsigned char*pBuf,LiftModeMessage& mMsg);
	//2017/4/22 motify ����˫����ģʽ mode==1
	int  GetValueFromZoomMap(std::string sPuid, int iFeet,int nHeightGroup, int iHeight,int mode);
	bool ZoomAutoManual(int nGroup , const unsigned char *pBuf);
	//2017/5/3 ����ģʽ�л� 
	int FreeModeSwitch(int nGroup,const unsigned char*pBuf,int iSwitchGroup=0);
	//2017/5/3 preset point call
	void PresetPointCall(int nGroup,const unsigned char*pBuff,int iSwitchGroup=0);
	void FreeCutScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void FreeCutScreenToTVwall(int nGroup, unsigned short iIPC, int iMapIndex, int i, int iSwitchGroup = 0);
	void SendToThirdDevice(int nGroup, const unsigned char *pBuff, int nDB, int nOffSet, int nByteNum);  //���͸�������ƽ̨

	void SwitchScreenModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcPtzOperationModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup = 0);
	void IpcZoomOperationModbus(int nGroup, const unsigned char *pBuf,const unsigned short *buf, int iSwitchGroup = 0);
	void CheckWriteIpcStateModbus(int iGroup, int iIndex, int iState);
	//===2017/6/8 modbus ���ڼӶ� 
	void IpcZoomOperationOnlyModbus(int nGroup ,int height, int feet, int iSwitchGroup);
	void HeartBeetModbus();
	bool AutoZoom(int nGroup, string strPUID);
	bool sceneZoom(const unsigned char*pBuf,std::string puid);//���ݳ���ZOOM �ж� 2017/1/9
protected:
	CClient(CClientManager *pClientMgr, PLCADDRDataInfo info);

	std::string m_strIp;
	int m_nPort;
	int m_nRack;
	int m_nSlot;
	

	int m_nNumIndex;	//�������
	CNetUDP net;
	
	PLCADDRDataInfo m_PLCFormatInfo;/* plc��ʽ���ݣ���Ҫ�Ƕ�ȡ����*/

	unsigned short m_stGroupSwitchState[32];
	std::map<int, unsigned short *> m_MapScreenState;/*screen operation status, group,screen,mode*/
	unsigned short m_stPtzState[32][64];/*ptz operation status, group,ptz,mode*/
	stPtzValue m_stPtzPresentPosition; //2016/12/18 ptz value ��ͷ���棬ptzֵ
	unsigned short m_stIpcOnlineState[32][64];/*ipc croup, ipc index*/
    Puid2ZoomVaule m_GroupZoomState; /*zoom state*/
	//std::map<int, unsigned short *> m_MapPtzState;
	std::map<int, unsigned char *> m_MapRecvData;	//�յ�������

	bool m_bConnect;
	bool m_bSentFirst;			// �Ƿ��Ѿ���һ�η���
	_daveOSserialType _fds;
	daveInterface * _di;
	daveConnection * _dc;

	CClientManager *m_pClientMgr;

	friend class CClientManager;

	//Modbus
	int m_nCurrentTime;
	int m_nGroup;	//��ǰ��
	bool m_bZoomAuto[32][64][1]; //�Զ����濪��
	modbus_t *m_ctx;
	unsigned short m_stModbusIpcOnlineState[32][64];/*ipc croup, ipc index*/
	unsigned short m_stModbusPtzState[32][64][7];/*ptz operation status, group,ptz,mode*/
	int m_iServerKeyID;

	//===22017/8/10 OPC
	void ReadOpcDataProcess();
	Item* opcItemFind(std::string name);
	void OpcSwitchScreen(int nGroup, int mode, int iSwitchGroup);
	void OpcIpcZoomOperation(int nGroup ,int height, int feet,int mode,int iSwitchGroup);

	//===2017-8-21
	void closeAllMode(int nGroup);

	//==2017/8/22 audio 
	std::map<int,bool>mAudioStatusMap;//��¼ÿ������̨�������豸״̬
	void Broadcast(int nGroup,int iSwitchGroup,bool bStart);
	//==2017-9-2 opc ptz manual
	void IpcPtzOperationOpc(int nGroup, int ipcIndex, int nCaramOperation, int iSwitchGroup);
};
