
// PlcTextDlg.h : ͷ�ļ�
#pragma once
#include <string>
#include "afxwin.h"
#include "afxcmn.h"
#include "IniFile.h"
//#include "fontListBox.h"
#include "LoadDllInstance.h"
#include "AX_Thread.h"
#include "value.h"
#include "reader.h"
#include "writer.h"
#include "HttpConfig.h"
#include "Language.h"
#include "SoftSet.h"
#include "ExtendListbox.h"
#include <process.h>
#include "TimeUtility.h"
#include "Runlog.h"
#include "DataViewer.h"
#include "MySQLConnector.h"
//#include "WndSizeManager.h"
#include "PtzTestPanel.h"

//#define DEBUG_MODE_Z
#define LOG_MAX_LEN 2048
#define REQUEST_SLEEP_TIME 30000
// CPlcTextDlg �Ի���
#define WM_LOG_INFO				(WM_USER + 50)




#define M2W(mStrNull, pWBuf, nWSize) MultiByteToWideChar(CP_ACP, 0, mStrNull, -1, pWBuf, nWSize)//string to wstring
#define W2M(wStrNull, pMBuf, nMSize) WideCharToMultiByte(CP_ACP, 0, wStrNull, -1, pMBuf, nMSize, NULL, NULL)//wstring to string


typedef struct TreeItemOgj
{
	CString csIpcID;
	CString csIpcName;
	int iState;
	int iGroup;
}TreeItemOgj;
//cam  status
typedef struct{
	int group;
	int index;
	std::string ipcId;
	std::string ipcName;
	int ipcStatus;//2���ߣ����಻����
}ipcInfo;

class CPlcTextDlg : public CDialog
{
// ����
public:
	CPlcTextDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PLCTEXT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:

	CComboBox m_OffSet;
	CComboBox m_ModelType;
	CComboBox m_IPC1;
	CComboBox m_IPC2;

	//cam status
	std::map<std::string,ipcInfo>mIpcInfoMap;
	////==2017/4/24 ipcInfo from AS300
	bool ipcStaFromAs300,mAs300DatabaseConnect;
	CMySQLConnector* m_pDBConnector;
	std::map<std::string,ipcInfo> ipcInfoMap;	
public:
	void mySqlConnect();
	void getIpcStatusMysql();

	/*CColorListBox m_LogList;*/
	CListBox m_LogList;
	CListBox m_LogListFilter;
	CCriticalSection m_Datalock;
	CTreeCtrl m_TreeIpcList;
	std::map<int, HTREEITEM> m_GroupItemMap;
	CImageList  m_imagelist;
	//CCuHttpConfigClient *m_pHttpGetState;
	HANDLE m_HandleThread;
	BOOL m_bStart;
	int m_nGroup;
	CString m_csRestartTime;		//����ʱ��
	
	BOOL m_bRestart;
	HANDLE m_HandleRestart;
	long m_lHeartTime;
	long m_lIpcStateTime;

	bool listStop;
	RECT	m_rtTree,m_rtLoglist;
	bool	m_bEnableFilter;
	CString m_strFilter;
public:
	int m_nPlcHeartBeatTimeout;		// PLC������ʱ��
	HANDLE	m_hThreadPLCWatchdog;		// ���Ź��߳̾��
	bool	m_bThreadPLCWatchdog;
	void  StartThreadWatchdog()
	{
		m_bThreadPLCWatchdog = true;
		if (!m_nPlcHeartBeatTimeout)
			m_nPlcHeartBeatTimeout = 3000;
		m_hThreadPLCWatchdog = (HANDLE)_beginthreadex(0,0,ThreadWatchDog,this,0,0);
	}
	void StopThreadWatchdog()
	{
		m_bThreadPLCWatchdog = false;
		WaitForSingleObject(m_hThreadPLCWatchdog,INFINITE);
		CloseHandle(m_hThreadPLCWatchdog);
		m_hThreadPLCWatchdog = NULL;
	}
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
	static UINT __stdcall ThreadWatchDog(void *p)
	{
		CPlcTextDlg *pThis = (CPlcTextDlg *)p;
		LoadInstance *pLoadInstance = LoadInstance::Instance();
		UINT64 nTimeNow = GetSysTimeMicros()/1000;
		UINT64 nDelay = 500;		// 2����Ź�����
		while(pThis->m_bThreadPLCWatchdog)
		{
			if ((GetSysTimeMicros()/1000 -nTimeNow ) > nDelay)
			{
				break;
			}
			Sleep(200);
		}

		while(pThis->m_bThreadPLCWatchdog)
		{
			nTimeNow = GetSysTimeMicros()/1000;
			UINT64 nHeartBeat = pLoadInstance->pProcPLCHeartBeat();
			UINT64 nTimeSpan = (nTimeNow - pLoadInstance->pProcPLCHeartBeat()) ;
			if (nTimeSpan > pThis->m_nPlcHeartBeatTimeout)
			{// PLC ��ȡ���ݳ�ʱ������PLC�Ѿ��Ͽ�����
				shared_ptr<CRunlog> pRunlog = shared_ptr<CRunlog>(new CRunlog(_T("PLCWatchdog")));
				pRunlog->Runlog(_T("PLC Heartbeat time out:%d.Now Restart Application.\n"),nTimeSpan);
				::PostMessage(pThis->m_hWnd,WM_SYSCOMMAND,SC_CLOSE,NULL);
				break;
			}
			Sleep(10);
		}

		return 0;
	}
// ʵ��
protected:
	HICON m_hIcon;
	BOOL m_bInitFailed;
	BOOL m_bConnect;
	long m_lLoginId;
	std::map<CString, int> m_MapTvWallIp2Port;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg LRESULT ShowlogInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonLogIn();
	afx_msg void OnBnClickedBtnDis();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnCleanLog();
	DECLARE_MESSAGE_MAP()

public:  //2017/03/15 dataViewer
	DataViewer* dataGet;
	void dataMessage(const char* buffer);
	afx_msg LRESULT OnDataViewerClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDataViewerClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGroupConfigureRefresh(WPARAM wParam, LPARAM lParam);//ˢ�·�������

	//2017/8/5 ptz test panel
	afx_msg LRESULT OnPtzTestClick(WPARAM wParam,LPARAM lParam);
	CPtzTestPanel* mPtzTest;
	afx_msg LRESULT OnPtzOperation(WPARAM wParam,LPARAM lParam);

public:
	void InitLanguage();
    void PaintbackGround();
	BOOL WStrToMStr(CStringW & strCS, std::string & sStr);
	BOOL MStrToWStr(CStringW & strCS, std::string & sStr);
	CString GetVersionData();
	CString GetBuildTime();
	int CreateRun();
	int DeleteRun();
	bool QueryRun();
	
	void ParseAS300IpcList(int iState, CString strIdState, CString strName, CString schlId);

	void StopRestartThread();
	void StartRestartThread();
	void Restart();
	static void *WorkThreadRestartInfo(LPVOID lpParam);
	void ClearLogFile(int iClear = 0);
	void StartGetInfoThread();
	void StopGetInfoThread();
	void Run();
	BOOL SendHttpmMsgToRest(std::string str, const char *Ip, const int nPort);
	void GetIpcStateList();
	void InsertTree(int iGroup, int iState, CString strIdState, CString strName, CString schlId);
	void ParseIpcList(std::string strIpcList);
	wchar_t * UTF8ToUnicode( const char* str );
	void DeleteItemData(CTreeCtrl* pCtrl,HTREEITEM hItem);
	void InitTree();
	void CleanTree();
	std::string GetCurrentPath();
	void ShowLog(CString strLog, BOOL bSucc = TRUE, int nColor = 0);
	std::vector<CString> Split(CString string, CString separator = _T(","));
	static bool CALLBACK InfoCallBack(const char *pStr, const char *Ip, const int nPort, unsigned long dwUser);
	static void CALLBACK LogCallback(int ityp, const char *pStr, unsigned long dwUser);
	static void *WorkThreadGetIpcInfo(LPVOID lpParam);
	afx_msg void OnBnClickedCheckRun();
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnBnClickedCheckListstop();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedCheckFilter();
	afx_msg void OnBnClickedButtonDataviewer();
};
