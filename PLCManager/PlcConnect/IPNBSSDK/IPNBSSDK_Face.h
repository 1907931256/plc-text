#ifndef _IPNBSSDK_FACE_H
#define _IPNBSSDK_FACE_H

#define UM_IPNBSSDK_STATE			(WM_USER+200)		// ����ô��ڷ��ͽ���״̬����Ϣ
#define DGRAMHEADER_HEADER_SIZE		8					// ����ͷ�ߴ�(δ��������ĸ�������)
#define BSSOCKET_SERVER_PORT		2048				// �㲥���������ƽ��ն˿�
#define BSSOCKET_MULTI_PORT			2046				// ���ó�������鲥״̬������˿�
#define BSSOCKET_STATE_PORT			2058				// ���ó�����յ���״̬������˿�
#define BSSOCKET_STATE_IP			_T("234.0.0.254")	// ���ó������״̬���鲥��ַ
#define BROADCAST_TO_ID_LEN			16					// �ն˲ɲ�Ŀ���ն�ѡ��ĸ������ݳ���(128����)
#define BROADCAST_TO_ID_LEN_EX		125					// �ն˲ɲ�Ŀ���ն�ѡ��ĸ������ݳ���(1000����)

//������״̬���
typedef enum 
{
	IPNBSSDK_STATE_TERMINAL_NULL = 1,			// �ն˿���
	IPNBSSDK_STATE_TERMINAL_LIVE_PLAY,			// �ն�ʵʱ�ɲ�
	IPNBSSDK_STATE_TERMINAL_TERMER_RING,		// �ն˶�ʱ����
	IPNBSSDK_STATE_TERMINAL_TERMER_PROGRAMS,	// �ն˶�ʱ��Ŀ
	IPNBSSDK_STATE_TERMINAL_SERVER_FIRE_ALARM,	// �ն˽��շ�������������
	IPNBSSDK_STATE_DIALOG_CALL,					// �Խ�����
	IPNBSSDK_STATE_DIALOG_BEGIN,				// �Խ���ʼ
	IPNBSSDK_STATE_DIALOG_END,					// �Խ�����
	IPNBSSDK_STATE_TERMINAL_ALARM1,				// �ն˱���1
	IPNBSSDK_STATE_TERMINAL_ALARM2,				// �ն˱���2
	IPNBSSDK_STATE_TASK_NULL,					// �������
	IPNBSSDK_STATE_TASK_TERMER_RING_BEGIN,		// ����ʱ����ִ��
	IPNBSSDK_STATE_TASK_TERMER_RING_END,		// ����ʱ����ֹͣ
	IPNBSSDK_STATE_TASK_FIRE_ALARM_BEGIN,		// ������������
	IPNBSSDK_STATE_TASK_FIRE_ALARM_END,			// ������������
	IPNBSSDK_STATE_TERMINAL_IP,					// ��ID�Ų鵽��IP��ַ
	IPNBSSDK_STATE_TERMINAL_ID,					// ��IP��ַ�鵽��ID��
	IPNBSSDK_STATE_TERMINAL_COUNT,				// ��ѯ���ն�����
	IPNBSSDK_STATE_PORT_STATE,					// ��ѯ���ն˶˿�״̬
	IPNBSSDK_STATE_SD_PLAY_STATE,				// SD������״̬
	IPNBSSDK_STATE_TERMINAL_ALARM_EX,			// �ն���չ��������
	IPNBSSDK_STATE_DIALOG_TALKING,				//ͨ����
	IPNBSSDK_STATE_DIALOG_REJECT,				//�ܽ���
	IPNBSSDK_STATE_DIALOG_OFFLINE,				//Ŀ�겻����
	IPNBSSDK_STATE_DIALOG_OFFLINE2,				//Ŀ�겻�ɴ�
	IPNBSSDK_STATE_DIALOG_BUSY,					//Ŀ����æ
	IPNBSSDK_STATE_DIALOG_SLEEP					//���˽���
}IPNBSSDK_STATE;

//�鲥�����ݽṹ(������״̬���)
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

extern "C" __declspec(dllexport) void IPNBSSDK_SetParentWnd(HWND hWnd);//��Ϣ��ʽ����״̬(δ���ûص���������Ч)
extern "C" __declspec(dllexport) BOOL IPNBSSDK_TCPConnect(LPTSTR strServerIP,int Port);//����TCP���ӣ�IP���˿�
extern "C" __declspec(dllexport) BOOL IPNBSSDK_SetServerIP(LPTSTR strServerIP);//���÷�����IP
extern "C" __declspec(dllexport) BOOL IPNBSSDK_TCPClose(); //����TCP�Ͽ�
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