#ifndef   _VS300CLIENTSDK_H   
#define   _VS300CLIENTSDK_H   


#ifdef WIN32
#define CALLBACK	__stdcall
#define DLL_API extern "C" _declspec(dllexport)

#else
#define  CALLBACK
#define DLL_API
#endif 

#ifndef uint64
typedef unsigned long long	uint64;
#endif

#ifndef uint32
#if defined(WIN32)
typedef unsigned long uint32;
#else
typedef unsigned int uint32;
#endif
#endif

#define MAX_SCHEME_NAME_LEN 256
#define MAX_ID_LENGTH		32
#define MAX_CHANNEL_DEVICE  256
#define SDK_MAX_NAME_LEN		256
#define MAX_RECORD_NAME_LEN 64 

#define POINT_NAME_LEN		 32	//预置点名称长度
#define PTZ_PRESETNUM		 64 //预置点的个数
#define PTZ_TOURPOINT		 64 //巡航线路中预置点的数量
#define PTZ_TOURNUM			 8  //巡航线路的数目




#define _EC(x)						(0x80000000|x)

#define   	SDK_Success						0						//成功
#define 	SDK_Error_Unknow				_EC(1)			//未知错误

#define     SDK_Error_NotLogin				_EC(10000)		//未登录或者未初始化
#define		SDK_Error_Timeout				_EC(10001)		//超时
#define		SDK_Error_ParamError			_EC(10002)		//参数错误

//登录服务器返回结果,SDK_Login接口使用:
#define SDK_Login_Timeout_Connect		_EC(2)	//登录服务器超时;
#define SDK_Login_Timeout_Group			_EC(3)	//获取用户所属域信息超时.
#define SDK_Login_Timeout_ORG			_EC(4)	//获取用户组权限信息超时.
#define SDK_Login_Timeout_SCHEME		_EC(5)	//获取轮巡计划超时.
#define SDK_Login_Timeout_ALERTSCHEME	_EC(6)	//获取报警策略超时.
#define SDK_Login_Timeout_DECPOLLSCHEME _EC(7)	//获取解码器轮询策略失败

#define SDK_Login_Fail_IP				_EC(8)	//连接服务器失败，请检查IP地址和端口.
#define SDK_Login_Fail_FILE				_EC(9)	//文件解析失败，请向管理员咨询.
#define SDK_Login_Fail_DATABASE_USER	_EC(10)	//没有找到当前用户.
#define SDK_Login_Fail_DATABASE_GROUP	_EC(11)	//没有组信息.
#define SDK_Login_Fail_USER				_EC(12)	//没有找到当前用户.
#define SDK_Login_Fail_DATA_LONG		_EC(13)	//数据超长.
#define SDK_Login_Fail_PASSWORD			_EC(14)	//密码错误.
#define SDK_Login_Fail_USER_ID			_EC(15)	//无效的用户ID.
#define SDK_Login_Fail_USER_EXIST		_EC(16)	//用户已登录.
#define SDK_Login_Fail_SESSION_EXIST	_EC(17)	//当前会话已经存在.
#define SDK_Login_Fail_USER_LOCKED		_EC(18)	//用户已被锁定.
#define SDK_Login_Fail_NO_GROUP			_EC(19)	//没有组信息.

//打开视频接口返回结果和打开回放返回,对讲打开时的返回错误
#define		SDK_Error_Video_Opened				_EC(20)				//视频已打开
#define 	SDK_Error_Rtsp_Connect_Failed		_EC(21)				//建立RTSP连接失败
#define		SDK_Error_Rtsp_Ack_Failed			_EC(22)				//RTSP应答失败
#define 	SDK_Error_Rtp_Listen_Failed			_EC(23)				//RTP建立侦听失败
#define		SDK_Error_Rtp_Connect_Failed		_EC(24)				//建立RTP连接失败
#define		SDK_Error_Player_Port_UseOut		_EC(25)				//解码端口用完
#define		SDK_Error_No_Right					_EC(26)				//无权限
#define		SDK_Error_Not_Found_Device			_EC(27)				//找不到对应的设备
#define		SDK_Error_Device_Offline			_EC(28)				//设备离线
#define		SDK_Error_LoginDev_Failed			_EC(29)				//设备登录失败 
#define		SDK_Error_No_DevVisitorRight		_EC(30)				//没有直连权限
	//ELR_Pdu_Ack_Failed,协议栈交互失败，具体原因参见CMS或DMS服务的应答码
#define		SDK_Error_CMS_Add_Camera_Failed		_EC(31)				//添加摄像头失败
#define     SDK_Error_CMS_No_MTS_To_Use			_EC(32)				//没有MTS可用
#define		SDK_Error_CMS_No_Device				_EC(33)				//找不到设备
#define		SDK_Error_CMS_Media_Session_Exist	_EC(34)				//媒体会话已存在
#define		SDK_Error_CMS_Server_Reconnect		_EC(35)				//功能服务正在重连
#define		SDK_Error_CMS_No_Right				_EC(36)				//无权限
#define 	SDK_Error_CMS_Not_Find_Child_Domain	 _EC(37)			//没有找到下级域
#define		SDK_Error_CMS_Not_Login_Child_Domain _EC(38)			//未登陆下级域
#define		SDK_Error_CMS_Child_Domain_Exception _EC(39)			//下级域异常
#define		SDK_Error_CMS_Reach_System_Limit	 _EC(40)			//达到系统上限
#define		SDK_Error_Chan						 _EC(41)			//通道数不正确(超出范围)
#define		SDK_Error_Playback_Opened		     _EC(42)			//回放已打开
#define		SDK_Error_Open_File_Failed			 _EC(43)			//打开文件失败
#define     SDK_Error_No_SS_To_Use				 _EC(44)			//没有SS
#define		SDK_Error_Playback_Limit			 _EC(45)			//达到回放限制
#define		SDK_Error_OtherUser_Playback		 _EC(46)			//其他用户正在回放
#define     SDK_Error_SSPlayback_Failed			 _EC(47)			//回放失败


//录像查询返回结果----------------------------------------------------------------
#define		SDK_Error_Search_No_Right					 _EC(50)	//
#define		SDK_Error_Search_Dev_Offline				 _EC(51)


//云台错误类型----------------------------
#define     SDK_Error_Ptz_No_Right						_EC(60)
#define     SDK_Error_Ptz_CruiseExist					_EC(61)	//当巡航线路中的预置点少于2时报错
#define     SDK_Error_Ptz_CruiseNoExist					_EC(62)	//巡航线路不存在
#define     SDK_Error_Ptz_CruiseRuning					_EC(63) //巡航线路正在运行，不允许删除


//对讲错误类型-----------------------
#define		SDK_Error_TalkOpened						 _EC(70) //对讲已打开
#define     SDK_Error_Talk_DeviceIsTalking				 _EC(71) //其他用户正在对讲	


//枚举类型-----------------------------
//事件通知
typedef enum
{
	EVENT_CMS_DISCONNECTED	=	0,	//cms断线；
	EVENT_CMS_RECONNECT,			//重连
	EVENT_RELOAD_DEVICE,			//通知外部重新获取设备
	EVENT_DEL_DEVICE,				//设备删除
	EVENT_VIDEO_EXCEPTION,			//视频错误
	EVENT_PLAYBACK_STATUS,			//回放状态
	EVENT_TALK_EXCEPTION,			//对讲错误
	EVENT_MODIFY_EMAP,				//电子地图更新

	//add xwy
	EVENT_PREVIEW_RESULT,
	EVENT_PLAYBACK_RESULT,
	//end
}EventCallbackType;

//CMS断线即与服务器断开链接时具体原因
typedef enum
{
	Disconnect_unknown = 0,	//未知，网络或者CMS重启之类的断线，此种模式下SDK会自动重连
	Disconnect_usererror	//用户被删除或者锁定，此种模式下SDK不会重连
}EnumCmsDisconnected;
//观看视频异常时，具体异常信息----------
typedef enum
{
	Exc_unknow = 0,							//未知
	Exc_MTS_Unregistered,				//MTS断线
	Exc_Right_Canceled,					//权限被取消
	Exc_Dev_Removed,					//设备被删除
	Exc_Session_Removed,				//会话被取消
}EnumVideoException;					

//回放时的消息回调
typedef enum
{
	//异常消息
	Playback_SS_Unregistered,
	Playback_Right_Canceled,
	Playback_Dev_Removed,
	Playback_SessionRemoved,
	Playback_SessionAbnormal,
	//状态通知
	Playback_Finished,
}EnumPlaybackStatus;

//对讲异常的消息回调
typedef enum
{
	Exc_Talk_Unknown,
	Exc_Talk_Mts_Unregistered,
	Exc_Talk_Dev_Removed,
}EnumTalkExcept;	

typedef enum
{
	Dev_Visiondigi = 0,
	Dev_Dahua,
	Dev_Hik,
}VSEnumDevProvider; 

//设备类型
typedef enum
{
	Dev_DVR,
	Dev_IPC,
	Dev_NVS,
	Dev_MCE,
	Dev_Decoder
}VSEnumDevType;
typedef enum
{
	Camera_Normal, //枪机
	Camera_SD,		//球机
	Camera_HalfSD //半球
}VSEnumCameraType;

//云台命令
typedef enum
{
	PTZ_INVALIADE,                    //无效命令
	//云台方向命令
	PTZ_UP,                              //上
	PTZ_DOWN,							 //下
	PTZ_LEFT,                            //左
	PTZ_RIGHT,                           //右
	PTZ_LEFTUP,                          //左上
	PTZ_LEFTDOWN,                        //左下
	PTZ_RIGHTUP,						 //右上
	PTZ_RIGHTDOWN=8,                       //右下

	//变倍变焦等命令
	PTZ_ZOOMADD=15,							//变倍+        //Camera operation 的枚举顺序勿改，关系其他代码
	PTZ_FOCUSADD,							//变焦+
	PTZ_APERTUREADD,	                    //光圈+
	PTZ_ZOOMREDUCE,			                //变倍-
	PTZ_FOCUSREDUCE,                        //变焦-
	PTZ_APERTUREREDUCE,                     //光圈-

	PTZ_GoPrepoint,							//转到预置点
	PTZ_StartCruise,						//启动巡航
	PTZ_StopCruise,							//停止巡航

	PTZ_LINESCAN_SETLEFT,					//线扫设置左边界
	PTZ_LINESCAN_SETRIGHT,					//设置右边界
	PTZ_LINESCAN_STARTSCAN,					//开始线扫
	PTZ_LINESCAN_STOPSCAN,					//停止线扫
	PTZ_TRACE_SETMODESTART,					//巡迹设置模式开始
	PTZ_TRACE_SETMODESTOP,					//设置模式结束
	PTZ_TRACE_RUNMODE,						//开始运行模式
	PTZ_TRACE_STOPMODE						//停止运行模式
}VSEnumPtzCommand;

//对讲类型
typedef enum
{
	Talk_Null = 0, //没有对讲
	Talk_G711a,
	Talk_Pcm,
}VSEnumTalkType;

//录像来源
typedef enum
{
	Record_Source_Device =2,			//设备录像
	Record_Source_SS				//平台录像
}VSEnumRecordSource;

//1全部，2报警录像，3移动侦测，4手动录像，5卡号录像，6定时录像
typedef enum
{
	Record_all = 1,					//所有录像	
	Record_Alarm_External,			//外部报警
	Record_Alarm_MotionDetect,		//移动侦测
	Record_Card =5,					//卡号录像
	Record_Timer,					//定时录像
	Record_Alarm_VideoLost,			//视频丢失，仅平台查找有用	
	Record_Alarm_VideoShelter,		//视频遮挡,仅平台查找有用
	Record_Time_SubStream = 30,		//子码流定时录像
}VSEnumRecordType;
//----------------

typedef enum
{
	PlayCtrl_Pause,//暂停恢复
	PlayCtrl_Speed //加放慢放
}VSEnumPlayCtrlType;

typedef struct  VSGroup_Info
{
	int				nID;	//ID
	int				nParentID;
	char				sztitle[SDK_MAX_NAME_LEN];	//设备名称
}VSGroup_Info_t;
typedef struct  VSDevice_Info
{
	char				szID[MAX_ID_LENGTH];	//设备ID
	int					nGoupID;
	VSEnumDevProvider	nProvider;				//设备提供方
	VSEnumDevType		nType;					//设备类型
	int					nTotalChnlNum;			//总通道数
	int					nAlarmIn;
	int					nAlarmOut;
	char				sztitle[SDK_MAX_NAME_LEN];	//设备名称
	char				szChannelIDs[MAX_CHANNEL_DEVICE][MAX_ID_LENGTH];//该设备下所有的通道ID
	char				szIP[16];
	int					nPort;
}VSDevice_Info_t;

typedef struct VSChannel_Info
{
	char szID[MAX_ID_LENGTH];
	VSEnumCameraType	 nType;
	int	 nChannelNo;
	char szDevID[MAX_ID_LENGTH];
	char szTitle[SDK_MAX_NAME_LEN];
}VSChannel_Info_t;

//录像查询结构体
typedef struct VSQuery_Record_Info
{
	char				szCameraId[MAX_ID_LENGTH];		// 摄像头ID
	int					nChannelNo;						// 通道号
	VSEnumRecordSource	nSource;						// 来源
	VSEnumRecordType	nRecordType;					// 类型
	uint64				nStartTime;						// 起始时间
	uint64				nEndTime;						// 结束时间
} VSQuery_Record_Info_t;

typedef struct VSRecord_Info
{
	unsigned char	source;							//来源类型：1未知，2设备，3中心
	unsigned char	recordType;						//录像类型。1全部，2报警录像，3移动侦测，4手动录像，5卡号录像，6定时录像,VSEnumRecordType
	bool			bDownloadByTime;				//是否按时间下载
	uint32			startTime;							//起始时间
	uint32			endTime;							//结束时间
	char			name[MAX_RECORD_NAME_LEN];			//录像名字（不同厂家对文件的标识不同）
	uint32			length;							//文件长度

	//下面是中心录像所需要的信息
	uint32			planId;							//录像计划ID
	uint32			ssId;							//存储服务ID
	char			diskId[MAX_ID_LENGTH];			//磁盘ID
	uint32			fileHandle;						//文件句柄

	char			devId[MAX_ID_LENGTH];			//支持报警录像回放，因为报警源与录像设备是分开的
	int				channelNo;

	char			szCamerID[MAX_ID_LENGTH];
	bool			bIpc;
}VSRecord_Info_t,*PVSRecord_Info_t;

//云台控制相关结构体
typedef struct PTZCommandParam
{
	long lParam1;
	long lParam2;
	long lParam3;
}VSPTZParam;

//预置点信息
typedef struct PTZPointInfo
{
	unsigned char	code;							//预置点编号 从1 开始
	unsigned char	revered[3];						//revered
	char			name[POINT_NAME_LEN];			//名字
}VSPtzPointInfo;

//巡航线路结构体中需要使用的预置点信息
typedef struct TourPointInfo
{
	unsigned char code;	//预置点编号		//从1开始
	//unsigned char id;	//预置点在巡航线路中的标识,
	short	 int  nStay;//预置点停留时间
}VSTourPointInfo;

typedef struct 
{
	int nCount;
	VSPtzPointInfo struPreset[PTZ_PRESETNUM];
}VSPtzPointSchedule;

typedef struct
{
	int		nId;					//巡航号
	int		nPresetCount;			//巡航线路中预置点数
	char	name[POINT_NAME_LEN];	//巡航线路名称
	VSTourPointInfo  point[PTZ_TOURPOINT];	//预置点数
}VSTourInfo;

typedef struct
{
	int nCount;
	VSTourInfo struTour[PTZ_TOURNUM];
}VSTourSchedule;

typedef struct
{
	unsigned char	channel;						//通道
	char			deviceId[32];			//设备ID（DVR+通道）
	unsigned char	type;							//类型。1视频丢失，2外部报警，3移动侦测
	unsigned char	status;							//状态。1发生，2消失
	int				alarmLevel;						//报警级别
	uint32			alarmTime;						//报警发生时间,精度为秒，值为time(NULL)值
	int				deviceType;						//设备类型。1编码器，2报警主机
	char			message[256];		//报警附加信息，支持GPS上报，gaowei 09-07-29
}VSAlarmInfo;

typedef enum
{
	right_monitor = 1,
	right_playback,
	right_record,
	right_ptz,
	right_alarm,
	right_view,
	right_devvisit,
	right_alarm_out,
}VSEnumRightType;

//定义事件类型-----------------------

typedef struct
{
	char ip[16];		//		ip : 对端ip地址
	int port;			//		port: 对端端口
	char username[64];	//		username: 使用用户名
	char password[64];	//		password : 使用密码
	int chstart;		//		chstart: 开始通道号
	int chnum;			//		chnum : 操作通道号
}VSConfigInfo;

//add xwy
typedef struct
{
	unsigned long nSessionId;
	bool IsDownload;
	long type;
	char szCameraId[64];
}VSParamInfo;
//end
#endif   

//回调函数指针的定义-------------------------------------------------
typedef int (CALLBACK *loginCallBack)(long login_id, long res, void* pUser);

//add xwy
//结果回调
typedef void (CALLBACK *ResCallBack)(uint32 nSequence,long res,VSParamInfo *paramInfo,void* pUser);
//end

//事件通知，如服务器断线等-----------------------------------------------------
//详见EventCallbackType
typedef void (CALLBACK *EventCallBack)(long login_id, long res, char*szID, int nParam1,int nParam2,void* pUser);	

//通知外部设备状态信息----------------------------------
typedef void (CALLBACK *DeviceStatusCallback)(long login_id, char* szDevId, int nStatus,void* pUser);

//实时预览数据回调
typedef void (CALLBACK *RealPlayCallback)(int nSessionId, char* buf, int len, void* user);
typedef bool (CALLBACK *PlayBackCallback)(int nSessionId, char* buf, int len, void* user);
typedef void (CALLBACK *TalkDataCallback)(int nSessionId, char* buf, int len, void* user);//对讲数据回调

typedef void (CALLBACK *PlayBackPosCallback)(int nSessionId, int pos, int total, void* user);

typedef void (CALLBACK *AlarmCallback)(long login_id, char* szDevId, VSAlarmInfo* alarmInfo, void* user);

//typedef void (__stdcall *OnMsgNotify)(Inner_Msg_t msgReq, Inner_Msg_t msg, void* pUser);
/***********************************
*初始化 
************************************/
DLL_API long SDK_CUInit();
/***********************************
*清理
************************************/
DLL_API long SDK_CUClear();

//add xwy
/***********************************
*设置响应回调函数
************************************/
DLL_API int SDK_CUSetResCallback(long login_id,ResCallBack cb,void* user);
//end

/***********************************
*设置设备状态回调函数
************************************/
DLL_API void SDK_CUSetDevStatusCallback(DeviceStatusCallback cb,void* user);
/***********************************
*设置实时监视数据的回调函数
************************************/
DLL_API int SDK_CUSetRealPlayCallBack(long logic_id, RealPlayCallback cb,void* user);
/***********************************
*设置回放数据的回调函数
************************************/
DLL_API int SDK_CUSetPlaybackCallBack(long logic_id,PlayBackCallback cb,void* user);


//设置回放进度回调函数
DLL_API int SDK_CUSetPlaybackPos(long logic_id, PlayBackPosCallback cb, void* user);

/***********************************
*设置对讲数据的回调函数
************************************/
DLL_API int SDK_CUSetTalkDataCallback(long logic_id, TalkDataCallback cb,void* user);
/***********************************
*设置事件回调函数
************************************/
DLL_API void SDK_CUSetEventCallback(EventCallBack ecb, void* pUser);

/***********************************
*设置报警回调函数
************************************/
DLL_API void SDK_CUSetAlarmCallback(AlarmCallback alarmCB, void* pUser);
/***********************************
*SDK登录，只有一个用户能登录
//!返回用户登陆id
bMediaTcp:1为tcp转输，其它为udp
************************************/
DLL_API long SDK_CULogin(char* serverIp, long port, char* username, char* password, long* pLogin_id, int bMediaTcp);
/***********************************
*登出服务器
************************************/
DLL_API long SDK_CULogout(long logic_id);

/***********************************
*获取到组织数目
*依据组织的数目开辟足够大小的空间，然后再调用SDK_CUGetGroupList去获取所有的组织信息
************************************/
DLL_API long SDK_CUGetGroupCount(long login_id, int& nCount);
/***********************************
*获取组织信息
************************************/
DLL_API long SDK_CUGetGroupList(long login_id, void* groupinfoList, int& groupCount);

//!获取组织信息
DLL_API long SDK_CUGetGroupInfo(long login_id, VSGroup_Info_t* groupinfoList);

/***********************************
*获取到设备数目
*依据设备的数目开辟足够大小的空间，然后再调用SDK_GetDeviceList去获取所有的设备信息
************************************/
DLL_API long SDK_CUGetDeviceCount(long login_id, int& nCount);
/***********************************
*获取设备信息
************************************/
DLL_API long SDK_CUGetDeviceList(long login_id, void* deviceinfoList, int& deviceCount);

//!获取设备信息
DLL_API long SDK_CUGetDeviceInfo(long login_id, VSDevice_Info_t* deviceinfoList);

/***********************************
*获取设备状态
************************************/
DLL_API long SDK_CUGetDeviceStatus(long login_id, char* devId, int& nstatus);
/***********************************
*获取通道信息
************************************/
DLL_API long SDK_CUGetChannelInfo(long login_id,char* cameraId,VSChannel_Info_t& channelInfo);
/***********************************
*视频请求，同步接口
*返回为sessionID,视频数据回调中通过sessionId来区分
************************************/
DLL_API long SDK_CUVideoRequest(long login_id, char* camerId, int type, int substream,int nWaiteTime, char* destIP, int destPort);
/***********************************
*停止实时监视
************************************/
DLL_API long SDK_CUStopVideoRequest(long login_id, char* camerId);
/***********************************
*查询录像接口
*PVSRecord_Info_t 查询输出结果
*ncount为指示当前有多少个录像
*nMaxCount为最大录像数目
*nWaitTime 为等待时间
************************************/

DLL_API long SDK_CUQueryRecord(long login_id, VSQuery_Record_Info_t queryInfo,PVSRecord_Info_t recordInfo,int& nCount, int nMaxCount,int nWaitTime);
/***********************************
*按文件回放，同步接口
*返回播放句柄
*返回值大于0表正常，不正常返回错误值如SDK_Error_Rtsp_Connect_Failed，SDK_Error_OtherUser_Playback
************************************/

DLL_API long SDK_CUPlaybackByFile(long login_id, PVSRecord_Info_t recordInfo,int nWaiteTime, char* destIP, int destPort);

DLL_API long SDK_CUDownloadByFile(long login_id, PVSRecord_Info_t recordInfo, char* filename, int nWaiteTime);

/***********************************
*按时间回放
*返回播放句柄
************************************/
//DLL_API long SDK_CUPlaybackByTime(char *szCamerID, uint64 lStartTime, uint64 lEndTime,VSEnumRecordSource nSource,int nWaiteTime);
/***********************************
*停止回放，通过句柄关闭回放
*nType 0：按文件回放， 1表示按时间回放
************************************/
DLL_API long SDK_CUStopPlayback(long login_id, long lPlayID);	

/***********************************
*定位码流获取位置-------
*timepose 为按时间定位时的时间,输入时间
************************************/
DLL_API long SDK_CUSeekPlayPose(long login_id, long lPlayID,uint64 timePose);

/***********************************
*播放控制-------
*playType为PlayCtrl_Pause时， param为1表暂停， 0表恢复
为PlayCtrl_Speed时，param为播放倍速*1000
************************************/
DLL_API long SDK_CUPlayCtrl(long login_id, long lPlayID, VSEnumPlayCtrlType playType, int param);

/***********************************
*云台控制
*方向控制，变倍变焦等，后续线扫之类的也进这里---
************************************/
DLL_API long SDK_CUPtzControl(long login_id, char* szCamerId, VSEnumPtzCommand ncommand,VSPTZParam param);

/***********************************
*云台控制
*获取预置点,改接口为同步接口
*另外提供接口获取结果
************************************/
DLL_API long SDK_CUPtzGetPrePoint(long login_id, char* szCanerId,VSPtzPointSchedule& schpoint,int nWaitTime);

/***********************************
*云台控制
*预置点相关操作 主要包含：添加，删除
*nType: 1为添加 2：为删除
************************************/
DLL_API long SDK_CUPtzPrePointOper(long login_id, char* szCanerId,VSPtzPointInfo pointInfo, int nType);

/***********************************
*云台控制
*获取巡航线路,该接口为同步接口
************************************/
DLL_API long SDK_CUPtzGetTourInfo(long login_id, char* szCanerId,VSTourSchedule& schTour,int nWaitTime);
/***********************************
*云台控制
*nType 0:表示增加， 1表示删除，2：表示修改
************************************/
DLL_API long SDK_CUPtzOperateTourInfo(long login_id, char* szCamerId,int nType,VSTourInfo schTour);

/***********************************
*开启对讲--
*szDevId:设备ID
*nTalk：对讲类型
*nWaitTime：超时等待时间,
*返回值 >0时为 对讲返回句柄
************************************/
DLL_API long SDK_CUStartTalk(long login_id, char* szDevId,VSEnumTalkType nTalk,int nWaitTime); 
/***********************************
*开启对讲--
*lHandle SDK_CUStartTalk 返回值
*------------------------------------*/
DLL_API long SDK_CUSendTalkData(long login_id, long lHandle,char* pdata,int nDataLen);
/****************************************
*关闭对讲
**************************************/
DLL_API long SDK_CUStopTalk(long login_id, long lHandle);


//获取设备操作权限
DLL_API long SDK_CUGetRight(long login_id, const char* szID, bool bIpc, VSEnumRightType type);

//获取电子地图
DLL_API long SDK_CUGetEmap(long login_id,char* mappath);

//解码器
DLL_API long SDK_CUSetDecoder(long login_id,char *decoderid,int decoderchannelId,char *cameraid,bool bipc, int streamtype,
							  int nXBlock, int nYBlock, int nX, int nY);

DLL_API long SDK_CUGetDecoderCount(long login_id,int& nCount);

DLL_API long SDK_CUGetDecoderList(long login_id,void* deviceinfoList, int& deviceCount);

DLL_API long SDK_CUStopDecoderVideo(long login_id,char *decoderid,int decoderchannelId);

DLL_API long SDK_CUSetDecoderSplit(long login_id,char *decoderid,int split);

//28181上级平台控制下级平台的录像启停
DLL_API long SDK_CUStartCenterRecord(long login_id,char* camerId, int substream, int nWaitTime);
DLL_API long SDK_CUStopCenterRecord(long login_id,char* camerId);


//!短连接操作配置
//!param 1 : input: 字符串，输入参数, VSConfigInfo
//!param 2 	char *querystr;	//		querystr: 请求字符串
//!param 3  :config_output: 字符串，输出参数，获取到的配置结果, json格式,内容配置具体配置有关
//!返回值: 0--表示成功
DLL_API long SDK_CUGetConfig(const VSConfigInfo* input, const char *querystr, char* config_output, const int size);

//!param 1 : input: 字符串，输入参数, VSConfigInfo, 格式见SDK_CUGetConfig input描述
//!param 2 : config: 字符串，输入参数, json格式, 具体格式和具体配置内容有关
DLL_API long SDK_CUSetConfig(const VSConfigInfo* input, const char *querystr, const char* config);
