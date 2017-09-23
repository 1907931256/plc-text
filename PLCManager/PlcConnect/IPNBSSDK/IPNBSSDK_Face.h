#ifndef _IPNBSSDK_FACE_H
#define _IPNBSSDK_FACE_H

#define UM_IPNBSSDK_STATE			(WM_USER+200)		// 向调用窗口发送接收状态的消息
#define DGRAMHEADER_HEADER_SIZE		8					// 数据头尺寸(未包括后面的附加数据)
#define BSSOCKET_SERVER_PORT		2048				// 广播服务器控制接收端口
#define BSSOCKET_MULTI_PORT			2046				// 调用程序接收组播状态的网络端口
#define BSSOCKET_STATE_PORT			2058				// 调用程序接收单播状态的网络端口
#define BSSOCKET_STATE_IP			_T("234.0.0.254")	// 调用程序接收状态的组播地址
#define BROADCAST_TO_ID_LEN			16					// 终端采播目标终端选择的附带数据长度(128个点)
#define BROADCAST_TO_ID_LEN_EX		125					// 终端采播目标终端选择的附带数据长度(1000个点)

//服务器状态输出
typedef enum 
{
	IPNBSSDK_STATE_TERMINAL_NULL = 1,			// 终端空闲
	IPNBSSDK_STATE_TERMINAL_LIVE_PLAY,			// 终端实时采播
	IPNBSSDK_STATE_TERMINAL_TERMER_RING,		// 终端定时打铃
	IPNBSSDK_STATE_TERMINAL_TERMER_PROGRAMS,	// 终端定时节目
	IPNBSSDK_STATE_TERMINAL_SERVER_FIRE_ALARM,	// 终端接收服务器消防报警
	IPNBSSDK_STATE_DIALOG_CALL,					// 对讲呼叫
	IPNBSSDK_STATE_DIALOG_BEGIN,				// 对讲开始
	IPNBSSDK_STATE_DIALOG_END,					// 对讲结束
	IPNBSSDK_STATE_TERMINAL_ALARM1,				// 终端报警1
	IPNBSSDK_STATE_TERMINAL_ALARM2,				// 终端报警2
	IPNBSSDK_STATE_TASK_NULL,					// 任务空闲
	IPNBSSDK_STATE_TASK_TERMER_RING_BEGIN,		// 任务定时打铃执行
	IPNBSSDK_STATE_TASK_TERMER_RING_END,		// 任务定时打铃停止
	IPNBSSDK_STATE_TASK_FIRE_ALARM_BEGIN,		// 任务消防报警
	IPNBSSDK_STATE_TASK_FIRE_ALARM_END,			// 任务消防报警
	IPNBSSDK_STATE_TERMINAL_IP,					// 由ID号查到的IP地址
	IPNBSSDK_STATE_TERMINAL_ID,					// 由IP地址查到的ID号
	IPNBSSDK_STATE_TERMINAL_COUNT,				// 查询到终端数量
	IPNBSSDK_STATE_PORT_STATE,					// 查询到终端端口状态
	IPNBSSDK_STATE_SD_PLAY_STATE,				// SD卡播放状态
	IPNBSSDK_STATE_TERMINAL_ALARM_EX,			// 终端扩展报警输入
	IPNBSSDK_STATE_DIALOG_TALKING,				//通话中
	IPNBSSDK_STATE_DIALOG_REJECT,				//拒接听
	IPNBSSDK_STATE_DIALOG_OFFLINE,				//目标不在线
	IPNBSSDK_STATE_DIALOG_OFFLINE2,				//目标不可达
	IPNBSSDK_STATE_DIALOG_BUSY,					//目标正忙
	IPNBSSDK_STATE_DIALOG_SLEEP					//无人接听
}IPNBSSDK_STATE;

//组播的数据结构(服务器状态输出)
typedef struct tagDGRAMHEADER_STATUS
{
	WORD wFlag;
	BYTE ucFunction;
	union 
	{
		BYTE  pParams[5];
		struct 
		{
			BYTE ucParam1;
			BYTE ucParam2;
			BYTE ucParam3;
			BYTE ucParam4;
			BYTE ucParam5;
		} ucParam;
	};
	BYTE ucAttachData1[50];
	BYTE ucAttachData2[50];
} DGRAMHEADER_STATUS, *LPDGRAMHEADER_STATUS;

extern "C" __declspec(dllexport) void IPNBSSDK_SetParentWnd(HWND hWnd);//消息方式接收状态(未设置回调函数才有效)
extern "C" __declspec(dllexport) BOOL IPNBSSDK_TCPConnect(LPTSTR strServerIP,int Port);//进行TCP连接，IP，端口
extern "C" __declspec(dllexport) BOOL IPNBSSDK_SetServerIP(LPTSTR strServerIP);//设置服务器IP
extern "C" __declspec(dllexport) BOOL IPNBSSDK_TCPClose(); //设置TCP断开
extern "C" __declspec(dllexport) BOOL IPNBSSDK_SetServerPort(WORD wPort);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_SetStatePort(WORD wPort);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlCall(WORD wFromID, WORD wToID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlAnswer(WORD wFromID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlHang(WORD wFromID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPCall(LPTSTR strFromIP, LPTSTR strToIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPAnswer(LPTSTR strFromIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPHang(LPTSTR strFromIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPCallEx(LPTSTR strFromIP, LPTSTR strToIP, BYTE ucPanel);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPAnswerEx(LPTSTR strFromIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIPHangEx(LPTSTR strFromIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlIO(WORD wID, BYTE ucPort, BOOL bIsOn);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlBroadcast(WORD wFromID, BYTE *pToID, BOOL bStart);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlMonitor(WORD wFromID, WORD wToID, BOOL bStart);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlQueryState(WORD wID);
//extern "C" __declspec(dllexport) WORD IPNBSSDK_CtrlQueryStateEx(WORD wID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlQueryIP(WORD wID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlQueryID(LPTSTR strTermIP);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlQueryTermCount(void);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlFireAlarm(WORD wAlarmArea, BOOL bStart);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlTimerRing(WORD wNO, BOOL bStart);
extern "C" __declspec(dllexport) void IPNBSSDK_SetStatusCallBack(DWORD dwCallBack, DWORD dwInstance);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlSetName(WORD wID, LPTSTR strName);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlSetVolume(WORD wID, BYTE ucVolume);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlQueryPort(WORD wID, BYTE ucPort);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlSDPlay(WORD wID, BOOL bPlay, BYTE ucFileIndex);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlVoiceControl(WORD wID, BYTE ucCtrlIn, BYTE ucCtrlOut);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlCallEx(WORD wFromID, WORD wToID, BYTE ucPanel);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlAnswerEx(WORD wFromID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlHangEx(WORD wFromID);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlBroadcastEx(WORD wFromID, BYTE *pToID, BOOL bStart);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlBroadcastSingle(WORD wFromID, WORD wToID, BYTE ucArea, BOOL bStart);
extern "C" __declspec(dllexport) void IPNBSSDK_SetIsUnicode(BOOL bIsUnicode);
extern "C" __declspec(dllexport) BOOL IPNBSSDK_CtrlCallPhone(WORD wFromID, WORD wToID, BYTE ucPanel, LPTSTR strPhone);


extern "C" __declspec(dllexport) BOOL GetMultiCallStat();

extern "C" __declspec(dllexport) void SetMultiCallNetParms(WORD wID,char* sIp,WORD wPort);

extern "C" __declspec(dllexport) BOOL MultiCall(bool bStart,int nType,int TypeParam,PWORD pwTerms,int TermCount);

#endif	//_IPNBSSDK_FACE_H