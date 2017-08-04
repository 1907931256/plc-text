
// PlcServerDlg.h : 头文件
//

#pragma once
#define BCCWIN
#include "nodave.h"
#include "openSocket.h"
#include "nodavesimple.h"
#include "fontListBox.h"
#include "afxwin.h"
#include <map>
#include <vector>
#include "IniFile.h"
#include "log/logger.h"
#include <list>
using namespace std;

#define debug 10
#define M2W(mStrNull, pWBuf, nWSize) MultiByteToWideChar(CP_ACP, 0, mStrNull, -1, pWBuf, nWSize)//string to wstring

typedef struct _portInfo {
	int fd;
}portInfo;

typedef struct{
	uc prot;
	uc ch1;
	uc ch2;
	uc len;
	uc xxxx1;
	uc func;
	uc xxxx2;
} ISOpacket;



class CPlcServerDlg;

typedef struct{
	CPlcServerDlg *_pthis;
	SOCKET _socket;
}ThreadParam;

typedef struct{
	int iDB;
	int iBit;
	int iValue;
}DBDATA;
// CPlcServerDlg 对话框
class CPlcServerDlg : public CDialog
{
// 构造
public:
	CPlcServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CPlcServerDlg();

// 对话框数据
	enum { IDD = IDD_PLCSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	//HANDLE m_hServerThread;
	list<HANDLE>m_vServerThread;

	HANDLE m_hMainThread;
	HANDLE m_hLoopThread;
	BOOL m_bStart;
	SOCKET m_nSocket;
	CColorListBox m_RevInfolist;
	std::map<int, unsigned char*> m_groupDataMap;
	std::map<int, DBDATA> m_DbDataMap;
	CCriticalSection m_Datalock;

	CComboBox m_DBCombox;
	Logger *m_pLog;

// 实现
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

public:
	static void WorkParseThread(LPVOID lpParam, LPARAM lParam);
	static void WorkParseMainThread(LPVOID lpParam, LPARAM lParam);
	afx_msg void OnBtnStartServer();
	afx_msg void OnBnClickedBtnGetinfo();
	afx_msg void OnBnClickedBtnSetinfo();
	afx_msg void OnBnClickedBit();

	CString GetBuildTime();
	CString GetVersionData();
	int bit2Int();
	void Int2Bit(int iValue);
	void ServerPort(int fd);
	void MainFunction();
	SOCKET m_sockSrv;
	volatile bool m_bWorkThreadRun;
	static BOOL DisConnect(SOCKET &s)
	{
		linger sLinger;
		sLinger.l_onoff = 1;	//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		// 如果m_sLinger.l_onoff = 0;则功能和2.)作用相同;
		sLinger.l_linger = 2;//优雅关闭
		__try
		{
			if (s == INVALID_SOCKET)
				__leave;
			if (setsockopt(s,SOL_SOCKET,SO_LINGER,(const char*)&sLinger,sizeof(linger)) != SOCKET_ERROR)
			{//优雅关闭
				if (shutdown(s,SD_BOTH) != SOCKET_ERROR)
				{
					char szBuff[1023] = {0};
					recv(s,szBuff,1023,0);
					closesocket(s);
					__leave;
				}
				else
				{
					shutdown(s,SD_BOTH);
					closesocket(s);
					__leave;
				}
			}
			else
			{//强制关闭
				shutdown(s,SD_BOTH);
				closesocket(s);
				__leave;
			}
		}
		__finally
		{
			s = INVALID_SOCKET;

		}
		return TRUE;
	}
	void StartServer();
	void StopServer();
	void StopMainThread();
	void InitDBData();
	void UInitData();
	std::string GetCurrentPath();
	std::vector<CString> Split(CString string, CString separator = _T(","));
	BOOL MStrToWStr(CStringW & strCS, std::string & sStr);
	void analyze(daveConnection * dc);
	void ShowInfo(CString str, BOOL bType);
	void ShowInfoToList(CString str, BOOL bType = FALSE);
	int SplitCString(CString &strIn, CStringArray& strAryRe, char str);
	void UpdateDataBtnState(BOOL bBol);
	void RunLoopThread();
	void SetInfo();
	void StartLoopThread();
	void StopLoopThread();
	static unsigned __stdcall LoopThread( void* pArguments);
	afx_msg void OnEnChangeEditValue();
	afx_msg void OnBnClickedBtnBegin();
	afx_msg void OnBnClickedBtnCancel();
	virtual BOOL DestroyWindow();
};
