
// PlcTextDlg.cpp : 实现文件

#include "stdafx.h"
#include "PlcText.h"
#include "PlcTextDlg.h"
#pragma comment(lib, "Version.lib ")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "..\..\Angel\ShareDaemon\ShareDaemon.h"
#pragma comment(lib,"../../angel/release/sharedaemon")

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

#pragma data_seg("Daemon_Share")
BOOL	g_bDeamonRun = FALSE;
#pragma data_seg()
#pragma comment(linker,"/SECTION:Daemon_Share,RWS")

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()
// CPlcTextDlg 对话框
CPlcTextDlg::CPlcTextDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPlcTextDlg::IDD, pParent)
,m_bConnect(FALSE)
,m_lLoginId(0)
,m_HandleThread(NULL)
,m_bStart(FALSE)
,m_nGroup(0)
,m_bRestart(FALSE)
,m_HandleRestart(NULL)
,m_bInitFailed(FALSE)
,m_lHeartTime(0)
,m_lIpcStateTime(0)
,m_nPlcHeartBeatTimeout(3)
,m_hThreadPLCWatchdog(NULL)
,m_bThreadPLCWatchdog(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_pHttpGetState = new CCuHttpConfigClient();
}

void CPlcTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_LogList);
	DDX_Control(pDX, IDC_LIST_LOG_FILTER, m_LogListFilter);
	
	//	DDX_Radio(pDX, IDC_RADIO1, m_GroupShow);
	DDX_Control(pDX, IDC_TREE1, m_TreeIpcList);
}

BEGIN_MESSAGE_MAP(CPlcTextDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CPlcTextDlg::OnBnClickedButtonLogIn)
	ON_BN_CLICKED(IDC_BTN_DIS, &CPlcTextDlg::OnBnClickedBtnDis)
	ON_WM_CTLCOLOR()
	//ON_BN_CLICKED(IDC_BTN_TEXT, &CPlcTextDlg::OnBnClickedBtnText)
	ON_BN_CLICKED(IDC_BTN_TEST, &CPlcTextDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_CLEAN_LOG, &CPlcTextDlg::OnBnClickedBtnCleanLog)
	ON_MESSAGE(WM_LOG_INFO, &CPlcTextDlg::ShowlogInfo)
	ON_BN_CLICKED(IDC_CHECK_RUN, &CPlcTextDlg::OnBnClickedCheckRun)
	ON_BN_CLICKED(IDC_BTN_SET, &CPlcTextDlg::OnBnClickedBtnSet)
	ON_BN_CLICKED(IDC_CHECK_LISTSTOP, &CPlcTextDlg::OnBnClickedCheckListstop)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_FILTER, &CPlcTextDlg::OnBnClickedCheckFilter)
	ON_BN_CLICKED(IDC_BUTTON_DATAVIEWER, &CPlcTextDlg::OnBnClickedButtonDataviewer)
	ON_MESSAGE(WM_DATAVIEWER_CLOSE,&CPlcTextDlg::OnDataViewerClose)
	ON_MESSAGE(WM_CONFIGURE_REFRESH,&CPlcTextDlg::OnGroupConfigureRefresh)
	ON_MESSAGE(WM_DATAVIEWER_CLICK,&CPlcTextDlg::OnDataViewerClick)
	ON_MESSAGE(WM_PTZTESTPANEL_CLICK,&CPlcTextDlg::OnPtzTestClick)
	ON_MESSAGE(WM_PTZOPERATIONTEST,&CPlcTextDlg::OnPtzOperation)
END_MESSAGE_MAP()

// CPlcTextDlg 消息处理程序
BOOL CPlcTextDlg::WStrToMStr(CStringW & strCS, std::string & sStr)
{
	std::string sRet;
	CStringW strW;
	strW = strCS;
	wchar_t wcTmp[MAX_PATH] = {0};
	wchar_t * wcpTmp = strW.GetBuffer(strW.GetLength() + 1);
	wcscpy_s(&wcTmp[0], sizeof(wcTmp) / sizeof(wcTmp[0]), wcpTmp);
	strW.ReleaseBuffer();
	wcTmp[sizeof(wcTmp) / sizeof(wcTmp[0]) - 1] = 0;
	char cTmp[MAX_PATH * 2] = {0};
	W2M(&wcTmp[0], &cTmp[0], sizeof(cTmp) / sizeof(cTmp[0]));
	cTmp[sizeof(cTmp) / sizeof(cTmp[0]) - 1] = 0;
	sRet = &cTmp[0];
	sStr = sRet;
	return TRUE;
}

BOOL CPlcTextDlg::MStrToWStr(CStringW & strCS, std::string & sStr)
{
	std::string sRet;
	CStringW strW;
	strW = strCS;
	wchar_t wcTmp[1024] = {0};
	M2W(sStr.c_str(), &wcTmp[0], sizeof(wcTmp) / sizeof(wcTmp[0]));
	wcTmp[sizeof(wcTmp) / sizeof(wcTmp[0]) - 1] = TCHAR('\0');
	strCS = &wcTmp[0];
	return TRUE;
}

CString CPlcTextDlg::GetVersionData()
{
	TCHAR szFullPath[MAX_PATH];
	DWORD dwVerInfoSize = 0;
	DWORD dwVerHnd;
	VS_FIXEDFILEINFO * pFileInfo;

	GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize)
	{
		// If we were able to get the information, process it:
		HANDLE  hMem;
		LPVOID  lpvMem;
		unsigned int uInfoSize = 0;

		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);

		::VerQueryValue(lpvMem, (LPTSTR)_T("\\"), (void**)&pFileInfo, &uInfoSize);

		int ret = GetLastError();
		WORD m_nProdVersion[4];
		// Product version from the FILEVERSION of the version info resource 
		m_nProdVersion[0] = HIWORD(pFileInfo->dwProductVersionMS); 
		m_nProdVersion[1] = LOWORD(pFileInfo->dwProductVersionMS);
		m_nProdVersion[2] = HIWORD(pFileInfo->dwProductVersionLS);
		m_nProdVersion[3] = LOWORD(pFileInfo->dwProductVersionLS); 

		CString strVersion ;
		strVersion.Format(_T("%d.%d.%d.%d"),m_nProdVersion[0],m_nProdVersion[1],m_nProdVersion[2],m_nProdVersion[3]);
		GlobalUnlock(hMem);
		GlobalFree(hMem);
		return _CS("Version") + _T(": ") + strVersion;
	}
}

CString CPlcTextDlg::GetBuildTime()
{
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260    
	memset(exeFullPath,0,MAX_PATH);  
	GetModuleFileName(NULL,exeFullPath,MAX_PATH);

	WIN32_FIND_DATA   filestruct;     
	HANDLE   hf;     
	hf   = FindFirstFile(exeFullPath,   &filestruct);   
	filestruct.ftCreationTime;//创建时间     
	filestruct.ftLastAccessTime;//访问时间   
	FILETIME ft=    filestruct.ftLastWriteTime;//修改时间     
	CString tmp;  
	CTime time(ft);//先把FILETIME类型的数据转换为CTime类型  
	tmp.Format(L"%d-%d-%d %d:%d:%d",time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond());  
	//再将CTime类型转换为CTime类型  
	CString fileWriteTime(tmp);//为了能把字符串连接起来，用CString val（）的方法声明
	return _CS("Build_data") + _T(": ") + fileWriteTime; 
}

BOOL CPlcTextDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

#ifdef DEBUG_MODE_Z
	((CEdit*)GetDlgItem(IDC_BUTTON_DATAVIEWER))->ShowWindow(TRUE);
#endif
	CString cstrPath;
	MStrToWStr(cstrPath, GetCurrentPath());
	// 语言设置
	CString strLanguage;
	CIniFile iniLanguageFile(cstrPath + _T("\\Language.ini"));
	iniLanguageFile.ReadString(_T("LANGUAGE"), _T("TYPE"), strLanguage);
	CLanguage::Instance()->SetLangPath(cstrPath + _T("\\Language.ini"), strLanguage);
	InitLanguage();

	
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_STOPDAEMON, _CS("KillDaemon"));
// 		BOOL bNameValid;
// 		CString strAboutMenu;
// 		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
// 		ASSERT(bNameValid);
// 		if (!strAboutMenu.IsEmpty())
// 		{
// 			pSysMenu->AppendMenu(MF_SEPARATOR);
// 			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
// 		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码


	((CButton*)GetDlgItem(IDC_CHECK_RUN))->SetCheck(QueryRun());

	WSADATA wsData;
	WORD wVer = MAKEWORD(2,2);
	int iRet = 0;
	iRet = WSAStartup(wVer, &wsData);
	if (iRet != 0)
	{
		//MessageBox(_CS("InitSockErr"), _CS("Error"), MB_OK);
	}
	//   GetDlgItem(IDC_IPADDRESS1)->SetWindowText(_T("172.16.35.45"));
	//GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("102"));

	//GetDlgItem(IDC_EDIT_RACK)->SetWindowText(_T("0"));
	//GetDlgItem(IDC_EDIT_SOLT)->SetWindowText(_T("2"));

	

	CIniFile iniRestartFile(cstrPath + _T("\\softset.ini"));

	int iRestart = 0;
	iniRestartFile.ReadInt(_T("RESTART"), _T("enable"), iRestart);
	iniRestartFile.ReadString(_T("RESTART"), _T("time"), m_csRestartTime);

	// 判断是否启用PLC看门狗
	/*需要在SOFTSET.INI文件加下以入设置,Enable=1,则启用, 否则禁用
	Timeout为超时值,即超过这个时间没收到PLC数据就认为PLC网络故障,重起PLC
	[PLCWatchdog]
	Enable=1
	Timeout=3000
	*/
	int nEnableWatchdog = 0;

	iniRestartFile.ReadInt(_T("PLCWatchdog"),_T("Enable"),nEnableWatchdog);
	iniRestartFile.ReadInt(_T("PLCWatchdog"), _T("Timeout"), m_nPlcHeartBeatTimeout);

	m_MapTvWallIp2Port.clear();
	CFileFind filefind;
	CString sFilePath(cstrPath + _T("\\config\\*.*"));


	InitTree();
	CleanTree();

	if (filefind.FindFile(sFilePath, 0))
	{
		BOOL bStatBol = FALSE;
		BOOL bBool = TRUE;
		while(bBool)
		{
			bBool = filefind.FindNextFile();
			if (filefind.IsDots())
			{
				continue;
			}
			CString sFileName = filefind.GetFileName();

			if (sFileName != "General.ini" && sFileName != "modbus.ini")
			{
				CIniFile iniFile(cstrPath+ _T("\\config\\")+sFileName);

				CStringArray GroupArray;
				iniFile.ReadSectionString(_T("GROUP"), GroupArray);
				for(int i = 0; i < GroupArray.GetSize(); i++)
				{
					bStatBol = TRUE;
					std::vector<CString> vectvWall;
					vectvWall = Split(GroupArray[i]);
					m_MapTvWallIp2Port.insert(std::make_pair(vectvWall[2], _ttoi(vectvWall[3])));//TVWall ip, port from Group_x.ini
				}


				CStringArray  GroupIPCIDlist;
				iniFile.ReadSectionString(_T("CARAMPTZ"), GroupIPCIDlist);

				//通过本地文件初始化设备列表
				for(int i = 0; i < GroupIPCIDlist.GetSize(); i++)
				{
					std::vector<CString> GroupIPCId;
					GroupIPCId = Split(GroupIPCIDlist[i], _T(","));
					//group1=1,1,3301061000003,172.16.40.95,80,root,123456,2,设备40.95
					unsigned short iGroup = _ttoi(GroupIPCId[0]) & 0xff;
					CString strIdState = _T("(") + GroupIPCId[2] + _T(",") + _CS("OffLine") + _T(")");
					InsertTree(iGroup, 0, strIdState, GroupIPCId[8], GroupIPCId[2]);
					//cam info 2017/6/26
					ipcInfo ipcGet;
					ipcGet.group = iGroup;
					ipcGet.index = _ttoi(GroupIPCId[1])&0xff;
					WStrToMStr(GroupIPCId[8],ipcGet.ipcName);
					std::string ipcIdStr;
					WStrToMStr(GroupIPCId[2],ipcIdStr);
					ipcGet.ipcId = ipcIdStr;
					ipcGet.ipcStatus = 0;
					mIpcInfoMap.insert(std::make_pair(ipcIdStr.substr(0,ipcIdStr.length()-2),ipcGet));//AS300 IPCID 比配置少两位
				}
				m_nGroup++;
			}
		}
		if (!bStatBol)
		{
			ShowLog(_CS("All_Cfg_files_are_empty"));
			m_bInitFailed = TRUE;
			return FALSE;
		}
	}
	else
	{
		ShowLog(_CS("No_Cfg_File"));
		m_bInitFailed = TRUE;
		return FALSE;
	}

	CIniFile iniFile(cstrPath + _T("\\config\\General.ini"));

	/////==========ipc get from as3oo database=====
	int ipcStaFromdBbEnable=0;
	iniFile.ReadInt(_T("IPCINFOFROMAS300"),_T("Enable"),ipcStaFromdBbEnable);
	if(ipcStaFromdBbEnable)
	{
		ipcStaFromAs300 = true;
		mAs300DatabaseConnect = false;
	}
	else
	{
		ipcStaFromAs300 = false;
		mAs300DatabaseConnect = false;
	}

	m_LogList.SetHorizontalExtent(2000);
	m_LogListFilter.SetHorizontalExtent(2000);

	GetDlgItem(IDC_EDIT_GROUPNUM)->SetWindowText(_T("1"));
	GetDlgItem(IDC_EDIT_SCREENNUM)->SetWindowText(_T("1"));
	GetDlgItem(IDC_EDIT_MODENUM)->SetWindowText(_T("1"));
	GetDlgItem(IDC_EDIT_SWITCHNUM)->SetWindowText(_T("1"));
	

	//加载配置文件中指明的DLL
	CString strDllName;
	iniFile.ReadString(_T("LOAD_DLL"), _T("DllName"), strDllName);
	LoadInstance::Instance(strDllName);
	if(LoadInstance::Instance()->GetLoadSuccess())
	{
		LoadInstance::Instance()->GetDllFuncAddr();

		LoadInstance::Instance()->init_sdk(InfoCallBack, LogCallback,  50, (DWORD)this);

		// 启动获取TVWALL IPC状态线程
		StartGetInfoThread();
		// 是否启用看门狗
		if (1 == nEnableWatchdog)
		{
			StartThreadWatchdog();
		}

		if (1 == iRestart)
		{
			// 定时重起线程
			StartRestartThread();
		}
	}
	else
		ShowLog(_CS("Load_Dll_fail"));

	listStop = false;
	GetDlgItem(IDC_TREE1)->GetWindowRect(&m_rtTree);
	ScreenToClient(&m_rtTree);
	GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&m_rtLoglist);
	ScreenToClient(&m_rtLoglist);
	SetForegroundWindow();
	dataGet = NULL;//=====2017/03/15 dataViewer
	mPtzTest = NULL;
	CWnd::ShowWindow(SW_SHOWMINIMIZED);
	CString dllNameTitle;
	if(strDllName==_T("PlcConnect.dll"))
	{
		int connectType =0;
		iniFile.ReadInt(_T("PLCORMODBUS"), _T("Switch"), connectType);
		if(connectType==1)
			dllNameTitle = _T("-PLC");
		else if(connectType==2)
			dllNameTitle = _T("-Modbus");

	}
	else if(strDllName==_T("JjhAlarmDll.dll"))
	{
		dllNameTitle = _T("-EIO");
	}
	CString title = _T("Mago ") + _CS("SoftwareTitle") + dllNameTitle + _T("         ") + GetVersionData() + _T("    ") + GetBuildTime();
	this->SetWindowText(title);

	getIpcStatusMysql();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPlcTextDlg::InitLanguage()
{
	//   GetDlgItem(IDC_PLC_SET)->SetWindowText(_CS("static_plc_set"));
	//	GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_CS("static_server_failed"));
	GetDlgItem(IDC_BTN_SET)->SetWindowText(_CS("Software_Set"));
	GetDlgItem(IDC_STATIC_TEST)->SetWindowText(_CS("group_test"));
	GetDlgItem(IDC_STATIC_GROUP_NUM)->SetWindowText(_CS("group_num"));
	GetDlgItem(IDC_STATIC_SCREEN_NUM)->SetWindowText(_CS("screen_num"));
	GetDlgItem(IDC_STATIC_MODE_NUM)->SetWindowText(_CS("mode_num"));
	GetDlgItem(IDC_STATIC_SWITCH_NUM)->SetWindowText(_CS("switch_num"));
	GetDlgItem(IDC_BTN_TEST)->SetWindowText(_CS("group_test"));
	GetDlgItem(IDC_BTN_CLEAN_LOG)->SetWindowText(_CS("clean_log"));
	GetDlgItem(IDC_CHECK_RUN)->SetWindowText(_CS("add_powered_up"));
	GetDlgItem(IDC_BUTTON_DATAVIEWER)->SetWindowText(_CS("dataViewer"));
	SetDlgItemText(IDC_CHECK_LISTSTOP,_CS("PauseScroll"));
	SetDlgItemText(IDC_CHECK_FILTER,_CS("StringFilter"));
}

void *CPlcTextDlg::WorkThreadGetIpcInfo(LPVOID lpParam)
{
	CPlcTextDlg* pThis = (CPlcTextDlg*)lpParam;
	pThis->Run();
	return 0;
}
void *CPlcTextDlg::WorkThreadRestartInfo(LPVOID lpParam)
{
	CPlcTextDlg* pThis = (CPlcTextDlg*)lpParam;
	pThis->Restart();
	return 0;
}
void CPlcTextDlg::StartRestartThread()
{
	m_bRestart = TRUE;
	AX_Thread::spawn(WorkThreadRestartInfo, this, 0, 0, &m_HandleRestart, 0, 0, 0);
}

void CPlcTextDlg::StartGetInfoThread()
{
	m_bStart = TRUE;
	AX_Thread::spawn(WorkThreadGetIpcInfo, this, 0, 0, &m_HandleThread, 0, 0, 0);
}
void CPlcTextDlg::StopRestartThread()
{
	if (m_HandleRestart)
	{
		m_bRestart = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_HandleRestart,1000))
		{
			TerminateThread(m_HandleRestart,-1);
			m_HandleRestart = NULL;
		}
	}
}
void CPlcTextDlg::StopGetInfoThread()
{
	if(m_HandleThread)
	{
		m_bStart = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_HandleThread,1000))
		{
			TerminateThread(m_HandleThread,-1);
			m_HandleThread = NULL;
		}
	}
}
void CPlcTextDlg::InitTree()
{
	HICON icon;
	icon=AfxGetApp()->LoadIcon(IDI_ICON1);
	m_imagelist.Add(icon);  //把图标载入图像列表控件
	m_TreeIpcList.SetImageList(&m_imagelist,TVSIL_NORMAL);  //为m_mytree设置一个图像列表，使CtreeCtrl的节点显示不同的图标
	m_TreeIpcList.InsertItem(_CS("device_list"), TVI_ROOT);
	m_GroupItemMap.empty();
	/////////////////////////////ya_wang test
	//std::string str = "[{\"group\":1, \"chlId\" : \"330106100035701\",\"name\" : \"IPC58.44\",\"state\" : 0},{\"group\":2, \"chlId\" : \"330106100035701\",\"name\" : \"IPC58.44\",\"state\" : 0}]";
	//ParseIpcList(str);
	//ParseIpcList();
}

LRESULT CPlcTextDlg::ShowlogInfo(WPARAM wParam, LPARAM lParam)
{
	char *pstr = (char*)lParam;
	CString cstr(pstr);
	int iType = (int)wParam;
	int nColor = 0;
	if (CONNECT_PLC_SUC == iType)
	{ 
		m_bConnect = TRUE;
		//	GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_CS("static_server_success"));//连接成功
	}
	else if (CONNECT_PLC_REC == iType)
	{
		m_bConnect = FALSE;
		nColor = RGB(255,0,0);
		//	GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_CS("static_server_recon"));//连接断开,重连
	}
	else if (CONNECT_PLC_ERR == iType)
	{
		m_bConnect = FALSE;
		nColor = RGB(255,0,0);
		//	GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_CS("static_server_failed"));//连接失败,重连
	} 
	else if(CONNECT_PLC_DATA == iType)// 2017/03/15 dataViewer
	{
		if(dataGet)
		{
			//char data[BLOCK_SIZE_COPY]={0};
			//memcpy(data,pstr,sizeof(data));
			dataMessage(pstr);
		}
	
		delete []pstr;
		return 1;
	}
	ShowLog(_T("data_info: ") + cstr,TRUE,nColor);
	delete []pstr;
	return 1;
}

void CPlcTextDlg::ShowLog(CString strLog, BOOL bSucc, int nColor)
{
	//设置宽度
	SIZE nWidth;
	HDC hdc = ::GetDC(m_hWnd);
	GetTextExtentPoint32(hdc, strLog, strLog.GetLength(), &nWidth);
	int lWidth = nWidth.cx - nWidth.cy;

	m_Datalock.Lock();
	if (lWidth > 2000)
	{
		m_LogList.SetHorizontalExtent(lWidth);
		m_LogListFilter.SetHorizontalExtent(lWidth);
	}
	::ReleaseDC(m_hWnd , hdc) ;

	if (strLog.GetLength() > 900)
	{
		strLog = strLog.Left(900);
	}
	CTime CurTime = CTime::GetCurrentTime();
	CString StrTime;
	StrTime.Format(_T("%.4d-%.2d-%.2d %.2d:%.2d:%.2d   "), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
	strLog += _T("\r\n");
	StrTime += strLog;
	//数据过长删除部分
	
	m_LogList.AddString(StrTime);

// 	OutputDebugString(StrTime.GetString());
// 	OutputDebugString(_T("\r\n"));

	m_LogList.Invalidate(TRUE);

	if (m_bEnableFilter && 
		m_strFilter.GetLength() && 
		StrTime.Find(m_strFilter) >= 0)
	{
		m_LogListFilter.AddString(StrTime);
	}

	if(!listStop)
	{
		m_LogList.SetCurSel(m_LogList.GetCount() - 1);
		m_LogListFilter.SetCurSel(m_LogListFilter.GetCount() - 1);
	}
	int nMaxLines = 2048;
	if (m_LogList.GetCount() > nMaxLines)
	{
		m_LogList.ResetContent();
	}
	if (m_LogListFilter.GetCount() > nMaxLines)
		m_LogListFilter.ResetContent();
	m_Datalock.Unlock();
}


std::vector<CString> CPlcTextDlg::Split(CString string, CString separator)
{
	CString oriStr=string;
	vector<CString> strVec;
	while (true)
	{
		CString n = oriStr.SpanExcluding(separator);
		strVec.push_back(n);
		oriStr = oriStr.Right(oriStr.GetLength()-n.GetLength()-1);
		if (oriStr.IsEmpty())
		{
			break;
		}
	}
	return strVec;
}

std::string CPlcTextDlg::GetCurrentPath()
{
	HMODULE module = GetModuleHandle(0); 
	char pFileName[MAX_PATH]; 
	GetModuleFileNameA(module, pFileName, MAX_PATH); 

	std::string csFullPath = pFileName; 
	int nPos = csFullPath.rfind( '\\' );
	if( nPos < 0 ) 
	{
		return ""; 
	}
	else 
	{
		return csFullPath.substr(0, nPos); 
	}
	return "";
}

void CPlcTextDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == IDM_STOPDAEMON)
	{
		SetShareDaemon(FALSE);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlcTextDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//PaintbackGround();
		CDialog::OnPaint();
	}
}

void CPlcTextDlg::PaintbackGround()
{
	CPaintDC dc(this);
	CRect   myrect;   
	GetClientRect(&myrect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP2);   //IDB_BITMAP_TOOL是你自己的图对应的ID 
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&
		bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,myrect.Width(),myrect.Height(),&dcMem,0,0,   
		bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPlcTextDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlcTextDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//定时请求设备列表 
	CDialog::OnTimer(nIDEvent);
}

void CPlcTextDlg::OnDestroy()
{
	CDialog::OnDestroy();
	// TODO: 在此处添加消息处理程序代码

	StopThreadWatchdog();
	StopGetInfoThread();
	StopRestartThread();
	CleanTree();

	CString cstrPath;
	MStrToWStr(cstrPath, GetCurrentPath());
	CIniFile iniRestartFile(cstrPath + _T("\\softset.ini"));
	int iClear = 0;
	iniRestartFile.ReadInt(_T("LOG"), _T("autoclear"), iClear);
	ClearLogFile(iClear);
	if (!m_bInitFailed)
	{
		LoadInstance::Instance()->uninit_sdk();
		LoadInstance::Uninstance();
	}
	m_pDBConnector = NULL;
}

bool CALLBACK CPlcTextDlg::InfoCallBack(const char *pStr, const char *Ip, const int nPort, unsigned long dwUser)
{
	CPlcTextDlg *pThis = (CPlcTextDlg*)dwUser;
	bool bRet = pThis->SendHttpmMsgToRest(pStr, Ip, nPort);
	char *pszlog = new char[LOG_MAX_LEN];
	memset(pszlog, LOG_MAX_LEN, 0);
	_snprintf(pszlog, LOG_MAX_LEN-1, "ip:%s; port:%d; info:%s", Ip, nPort, pStr);
	pThis->PostMessage(WM_LOG_INFO, 0, (LPARAM)(pszlog));
	return bRet;
}

void CALLBACK CPlcTextDlg::LogCallback(int itype, const char *pStr, unsigned long dwUser)
{
	CPlcTextDlg *pThis = (CPlcTextDlg*)dwUser;
	if(itype==CONNECT_PLC_DATA) //2016/11/26 数据显示
	{
		char*data = new char[BLOCK_SIZE_COPY];
		memcpy(data,pStr,BLOCK_SIZE_COPY);
		pThis->PostMessage(WM_LOG_INFO,itype,(LPARAM)data);
		return ;
	}//====
	char *pszlog = new char[LOG_MAX_LEN];
	memset(pszlog, LOG_MAX_LEN, 0);
	_snprintf(pszlog, LOG_MAX_LEN-1, "%s",pStr);
	pThis->PostMessage(WM_LOG_INFO, itype, (LPARAM)(pszlog));
}
void CPlcTextDlg::Restart()
{
	while(m_bRestart)
	{
		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		CString csTime;
		csTime.Format(_T("%02d:%02d:%02d"), systime.wHour, systime.wMinute, systime.wSecond);
		if (csTime == m_csRestartTime)
		{
			Sleep(1000);
			//::PostMessage(this->m_hWnd,WM_SYSCOMMAND,SC_CLOSE,NULL);  
			//获取exe程序当前路径   
			extern CPlcTextApp theApp;
			TCHAR szAppName[MAX_PATH]; 
			:: GetModuleFileName(theApp.m_hInstance, szAppName, MAX_PATH);  
			CString strAppFullName;  
			strAppFullName.Format(_T("%s"),szAppName);  
			//重启程序   
			STARTUPINFO StartInfo;  
			PROCESS_INFORMATION procStruct;  
			memset(&StartInfo, 0, sizeof(STARTUPINFO));  
			StartInfo.cb = sizeof(STARTUPINFO);  
			bool bState = false;
			bState = ::CreateProcess(  
				(LPCTSTR)strAppFullName,  
				NULL,  
				NULL,  
				NULL,  
				FALSE,  
				NORMAL_PRIORITY_CLASS,  
				NULL,  
				NULL,  
				&StartInfo,  
				&procStruct);
			if (!bState)
			{
				Sleep(500);
				bState = ::CreateProcess(  
					(LPCTSTR)strAppFullName,  
					NULL,  
					NULL,  
					NULL,  
					FALSE,  
					NORMAL_PRIORITY_CLASS,  
					NULL,  
					NULL,  
					&StartInfo,  
					&procStruct);
			}
			::PostMessage(this->m_hWnd,WM_SYSCOMMAND,SC_CLOSE,NULL);
			if (!bState)
			{
				int err = GetLastError();
				CString str;
				str.Format(_T(", %d"), err);
				MessageBox(_CS("Restart_Failed")+str);
			}
		}
		Sleep(200);
	}
}
// 获取TVWALL IPC状态
void CPlcTextDlg::Run()
{
	while(m_bStart)
	{
		if(!ipcStaFromAs300)
		{
			if (GetTickCount() - m_lHeartTime >= 5000)
			{
				m_lHeartTime = GetTickCount();
				LoadInstance::Instance()->heart_beet();
			}
			if (GetTickCount() - m_lIpcStateTime >= REQUEST_SLEEP_TIME)
			{
				m_lIpcStateTime = GetTickCount();
				GetIpcStateList();
			}
		}
		else 
		{
			getIpcStatusMysql();
		}
		Sleep(1000);
		
		//if (GetTickCount() - m_lHeartTime >= 5000)
		//{
		//	m_lHeartTime = GetTickCount();
		//	LoadInstance::Instance()->heart_beet();
		//}
		//if (GetTickCount() - m_lIpcStateTime >= REQUEST_SLEEP_TIME)
		//{
		//	m_lIpcStateTime = GetTickCount();
		//	GetIpcStateList();
		//}
		//Sleep(100);
		//Sleep(REQUEST_SLEEP_TIME);
	}
}
void CPlcTextDlg::GetIpcStateList()
{

		Json::Value RevRoot;   
		RevRoot["key_id"] = 12;
		RevRoot["command"] = "COMMON";
		RevRoot["sub_cmd"] = "LIST_CHANNEL_ALL";
		Json::FastWriter Write;
		std::string strGetList = Write.write(RevRoot);

		for (std::map<CString, int>::iterator ite = m_MapTvWallIp2Port.begin(); ite != m_MapTvWallIp2Port.end(); ite++)
		{
			std::string strIp;
			CString cstrIp = ite->first;
			WStrToMStr(cstrIp, strIp);

			CCuHttpConfigClient* m_pHttpGetState = new CCuHttpConfigClient;
			m_pHttpGetState->SetQueryStr("/api/v1/netkbd");
			m_pHttpGetState->SetChn(0, 0);
			m_pHttpGetState->ReSetRespance(FALSE);
			m_pHttpGetState->SetJsonBody(strGetList.c_str());

			m_pHttpGetState->SetIP(strIp.c_str(), ite->second);

			std::string json_reponse = "";
			if(m_pHttpGetState->Start(json_reponse))
			{
				ParseIpcList(json_reponse);
				//delete m_pHttpGetState;
				//break;
			}
			else
			{
				char *pszlog = new char[LOG_MAX_LEN];
				memset(pszlog, LOG_MAX_LEN, 0);
				_snprintf(pszlog,LOG_MAX_LEN-1, "ip:%s; port:%d; error: get ipc state connect tv_wall server failed or timeout!", strIp.c_str(), ite->second);
				PostMessage(WM_LOG_INFO, 0, (LPARAM)(pszlog));
			}
			delete m_pHttpGetState;
		}
}

void CPlcTextDlg::ParseIpcList(std::string strIpcList)
{
	if (0 != strIpcList.length() && -1 == strIpcList.find("\"HeartBeat\""))		//当strIpcList内容是心跳包的内容时
	{
		LoadInstance::Instance()->set_ipc_state(strIpcList.c_str());//TUDO ya_wang设置对应字段的DB数据,直接传递json格式有dll自己解析比较

		Json::Value RevRoot;   
		Json::Reader reader;
		reader.parse(strIpcList, RevRoot);

		int iSize = RevRoot.size();
		if (iSize <= 0)
		{
			return;
		}
		for (int i = 0; i < iSize; i++)
		{
			int iGroup = RevRoot[i]["group"].asInt();
			std::string schlId = RevRoot[i]["chlId"].asString();
			std::string sName = RevRoot[i]["name"].asString();
			int iState = RevRoot[i]["state"].asInt();
			std::string sOnLine, sOffLine;
			WStrToMStr(_CS("OnLine"), sOnLine);
			WStrToMStr(_CS("OffLine"), sOffLine);
			std::string sState = iState == 2 ? sOnLine : sOffLine;

			char szlog[LOG_MAX_LEN] = {0};
			_snprintf(szlog, LOG_MAX_LEN-1, "(%s, %s)", schlId.c_str(), sState.c_str());
			wchar_t *p = UTF8ToUnicode(sName.c_str());
			CString strName(p);
			CString strIdState(szlog);
			CString csChlId(schlId.c_str());
			free(p);
			InsertTree(iGroup, iState, strIdState, strName, csChlId);
		}
	}
}

void CPlcTextDlg::InsertTree(int iGroup, int iState, CString strIdState, CString strName, CString schlId)
{
	std::map<int, HTREEITEM>::iterator iteItem = m_GroupItemMap.find(iGroup);
	if (iteItem != m_GroupItemMap.end())
	{
		HTREEITEM hChildItem = m_TreeIpcList.GetChildItem(iteItem->second);//遍历该子节点，修改状态或者添加设备
		while(hChildItem!=NULL)
		{
			TreeItemOgj *pItem = (TreeItemOgj*)m_TreeIpcList.GetItemData(hChildItem);
			if (pItem->csIpcID == schlId /*&& pItem->csIpcName == strName && pItem->iGroup == iGroup*/)
			{
				//已存在，修改状态，返回
				m_TreeIpcList.SetItemText(hChildItem, strName + strIdState);
				return;
			}
			hChildItem = m_TreeIpcList.GetNextItem(hChildItem, TVGN_NEXT);  
		} 
		HTREEITEM sub_son0=m_TreeIpcList.InsertItem(strName + strIdState, 0, 0, iteItem->second, TVI_LAST);
		TreeItemOgj *pItem = new TreeItemOgj(); 
		pItem->iState = iState;
		pItem->csIpcID = schlId;
		pItem->csIpcName = strName;
		pItem->iGroup = iGroup;
		m_TreeIpcList.SetItemData(sub_son0, (DWORD)pItem);
		m_TreeIpcList.Expand(iteItem->second, TVE_EXPAND); //只展开根结点下面一层.
	}
	else
	{
		if (0 == iGroup)
		{
			ParseAS300IpcList(iState, strIdState, strName, schlId);
		}
		else
		{
			CString strGroup;
			strGroup.Format(_CS("Group_%d"), iGroup);

			HTREEITEM root0=m_TreeIpcList.InsertItem(strGroup, 0, 1, TVI_ROOT, TVI_LAST);
			HTREEITEM sub_son0=m_TreeIpcList.InsertItem(strName + strIdState, 0, 0, root0, TVI_LAST);
			TreeItemOgj *pItem = new TreeItemOgj(); 
			pItem->iState = iState;
			pItem->csIpcID = schlId;
			pItem->csIpcName = strName;
			pItem->iGroup = iGroup;
			m_TreeIpcList.SetItemData(sub_son0, (DWORD)pItem);
			m_TreeIpcList.Expand( root0, TVE_EXPAND); //只展开根结点下面一层.

			m_GroupItemMap.insert(std::make_pair(iGroup, root0));
		}
	}
}
void CPlcTextDlg::ParseAS300IpcList(int iState, CString strIdState, CString strName, CString schlId)
{
	for (int iGroup = 1; iGroup <= m_nGroup; iGroup++)
	{
		std::map<int, HTREEITEM>::iterator iteItem = m_GroupItemMap.find(iGroup);
		if (iteItem != m_GroupItemMap.end())
		{
			HTREEITEM hChildItem = m_TreeIpcList.GetChildItem(iteItem->second);//遍历该子节点，修改状态或者添加设备
			while(hChildItem!=NULL)
			{
				TreeItemOgj *pItem = (TreeItemOgj*)m_TreeIpcList.GetItemData(hChildItem);
				if (pItem->csIpcID == schlId && pItem->csIpcName == strName && pItem->iGroup == iGroup)
				{
					//已存在，修改状态，返回
					m_TreeIpcList.SetItemText(hChildItem, strName + strIdState);
					//return;
					break;
				}
				hChildItem = m_TreeIpcList.GetNextItem(hChildItem, TVGN_NEXT);  
			} 
		}
	}
}

wchar_t * CPlcTextDlg::UTF8ToUnicode( const char* str )
{
	int textlen ;
	wchar_t * result;
	//textlen = MultiByteToWideChar( CP_UTF8, 0, str,-1, NULL,0 ); 
	//result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
	//memset(result,0,(textlen+1)*sizeof(wchar_t)); 
	//MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen ); 
	//return result; 

	textlen = MultiByteToWideChar( CP_ACP, 0, str,-1, NULL,0 ); 
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
	memset(result,0,(textlen+1)*sizeof(wchar_t)); 
	MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen ); 
	return result; 
}

void CPlcTextDlg::CleanTree()
{
	//m_TextDemoTree 先清理上次的先关资源
	HTREEITEM hItem;  
	hItem = m_TreeIpcList.GetRootItem();  //获得根目录节点  
	if(!m_TreeIpcList.ItemHasChildren(hItem))
	{
		return;
	}
	DeleteItemData(&m_TreeIpcList,hItem );
	m_TreeIpcList.DeleteAllItems();
}

void CPlcTextDlg::DeleteItemData(CTreeCtrl* pCtrl,HTREEITEM hItem)   
{      
	if(pCtrl->ItemHasChildren(hItem))       
	{   
		HTREEITEM   hChildItem = pCtrl->GetChildItem(hItem);       
		while(hChildItem!=NULL)       
		{   
			DeleteItemData(pCtrl,hChildItem); //递归遍历孩子节点       
			hChildItem  = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);       
		}       
	}  
	else
	{//删除绑定的相关变量 
		TreeItemOgj  *pItem = (TreeItemOgj*)pCtrl->GetItemData(hItem);
		delete pItem;
	}
}

BOOL CPlcTextDlg::SendHttpmMsgToRest(std::string str, const char *Ip, const int nPort)
{
	CCuHttpConfigClient* pClient = new CCuHttpConfigClient;
	pClient->SetIP(/*m_HttpIp.c_str()*/Ip, /*m_nHttpPort*/nPort);
	pClient->SetQueryStr("/api/v1/netkbd");
	pClient->SetChn(0, 0);
	pClient->SetJsonBody(str.c_str());

	std::string json_reponse;
	if(! pClient->Start(json_reponse))
	{
		char *pszlog = new char[LOG_MAX_LEN];
		memset(pszlog, LOG_MAX_LEN, 0);
		_snprintf(pszlog, LOG_MAX_LEN-1, "ip:%s; port:%d; error: send commend connect tv_wall server failed or timeout!", Ip, nPort);
		PostMessage(WM_LOG_INFO, 0, (LPARAM)(pszlog));

		delete pClient;
		return false;
	}
	delete pClient;
	return true;
}

	void CPlcTextDlg::OnBnClickedButtonLogIn()
{
	//   ///////////////////////////////////test写plc对应ipc状态

	//std::string str = "[{\"group\":1, \"chlId\":\"123\", \"name\":\"zs\", \"state\":1, \"local_id\":10003}]";
	//LoadInstance::Instance()->set_ipc_state(str.c_str());
	//////////////////////////////////////
	// TODO: 在此添加控件通知处理程序代码
	CString CstrIp, CstrPort;
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(CstrIp);
	//GetDlgItem(IDC_EDIT_PORT)->GetWindowText(CstrPort);

	std::string strIp;
	WStrToMStr(CstrIp, strIp);

	CString strRack, strSlot;
	//GetDlgItem(IDC_EDIT_RACK)->GetWindowText(strRack);
	//GetDlgItem(IDC_EDIT_SOLT)->GetWindowText(strSlot);
	int Rack = _ttoi(strRack);
	int Slot = _ttoi(strSlot);

	PLCADDRDataInfo GetInfo;
	m_lLoginId = LoadInstance::Instance()->login_plc_server(strIp.c_str(), _ttoi(CstrPort), GetInfo, 0, 2);
	if (m_lLoginId > 0)
	{
		m_bConnect = true;
		//GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_T("PLC服务已连接"));
	}
	else
	{
		//GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_T("PLC服务连接失败"));
	}
}

void CPlcTextDlg::OnBnClickedBtnDis()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bConnect)
	{
		LoadInstance::Instance()->logout_plc_server(m_lLoginId);
		m_bConnect = false;
		//GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_T("连接已断开"));
	}
}

HBRUSH CPlcTextDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	//if (pWnd->GetDlgCtrlID() == IDC_STATIC_SERVER)
	//{
	//	if (m_bConnect)
	//	{
	//		pDC->SetTextColor(RGB(0, 255, 0));
	//	}
	//} 
	return hbr;
}

void CPlcTextDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strGroup, strScreen, strMode, strSwitch;
	GetDlgItem(IDC_EDIT_GROUPNUM)->GetWindowText(strGroup);
	GetDlgItem(IDC_EDIT_SCREENNUM)->GetWindowText(strScreen);
	GetDlgItem(IDC_EDIT_MODENUM)->GetWindowText(strMode);
	GetDlgItem(IDC_EDIT_SWITCHNUM)->GetWindowText(strSwitch);

	LoadInstance::Instance()->set_group_switch(_ttoi(strGroup), _ttoi(strScreen), _ttoi(strMode), _ttoi(strSwitch));
}

void CPlcTextDlg::OnBnClickedBtnCleanLog()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Datalock.Lock();
	m_LogList.ResetContent();
	m_LogListFilter.ResetContent();
	m_Datalock.Unlock();
}
bool CPlcTextDlg::QueryRun()
{
	HKEY hKEY;
	LPCTSTR data_Set=_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	long ret=RegOpenKeyEx(HKEY_CURRENT_USER,data_Set, 0, KEY_READ, &hKEY);
	if (ERROR_SUCCESS != ret)
	{
		return false;
	}

	TCHAR szKeyName[MAX_PATH] = {0};
	DWORD dwMaxSubKey = MAX_PATH;
	DWORD i = 0;

	while (RegEnumValue(hKEY, i,szKeyName, &dwMaxSubKey, NULL, NULL,NULL,NULL) == ERROR_SUCCESS)
	{
		if(_tcscmp(szKeyName,_T("PlcConnect")) == 0)
		{
			RegCloseKey(hKEY);
			return true;
		}
		memset(szKeyName, 0, sizeof(szKeyName));
		dwMaxSubKey = 260;
		++i;
	}

	RegCloseKey(hKEY);
	return false;
}
//开机启动
int  CPlcTextDlg::CreateRun()
{
	//添加以下代码
	HKEY   hKey; 
	char pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//添加注册
		RegSetValueEx(hKey, _T("PlcConnect"), 0,REG_SZ,(const BYTE*)(LPCSTR)pFileName, MAX_PATH);
		RegCloseKey(hKey); 
	}
	return 0;
}
//取消开机启动
int CPlcTextDlg::DeleteRun()
{
	//添加以下代码
	HKEY   hKey; 
	char pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//删除注册
		RegDeleteValue(hKey,_T("PlcConnect"));
		RegCloseKey(hKey); 
	}
	return 0;
}
void CPlcTextDlg::OnBnClickedCheckRun()
{
	// TODO: 在此添加控件通知处理程序代码
	if(BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_RUN))
	{
		CreateRun();
	}
	else
	{
		DeleteRun();
	}
}

void CPlcTextDlg::OnBnClickedBtnSet()
{
	// TODO: 在此添加控件通知处理程序代码
	int iPtz,iZoom,iTvwall,iAutoClear,iTimeEnable, iSwitchGroup;
	int nEanbleWatchdog,nPLCHeartBeatTime;
	CString csTime;
	CString cstrPath;
	MStrToWStr(cstrPath, GetCurrentPath());

	CIniFile iniRestartFile(cstrPath + _T("\\softset.ini"));

	iniRestartFile.ReadString(_T("RESTART"), _T("time"), csTime);
	iniRestartFile.ReadInt(_T("RESTART"), _T("enable"), iTimeEnable);
	iniRestartFile.ReadInt(_T("LOG"), _T("ptz"), iPtz);
	iniRestartFile.ReadInt(_T("LOG"), _T("zoom"), iZoom);
	iniRestartFile.ReadInt(_T("LOG"), _T("tvwall"), iTvwall);
	iniRestartFile.ReadInt(_T("LOG"), _T("autoclear"), iAutoClear);
	iniRestartFile.ReadInt(_T("LOG"), _T("switchgroup"), iSwitchGroup);

	iniRestartFile.ReadInt(_T("PLCWatchdog"), _T("Enable"), nEanbleWatchdog);
	iniRestartFile.ReadInt(_T("PLCWatchdog"), _T("Timeout"), nPLCHeartBeatTime);

	CSoftSet softDlg;
	softDlg.m_iPtz				 = iPtz;
	softDlg.m_iTvwall			 = iTvwall;
	softDlg.m_iZoom				 = iZoom;
	softDlg.m_csTime			 = csTime;
	softDlg.m_iAutoClear		 = iAutoClear;
	softDlg.m_iTimeEnable		 = iTimeEnable;
	softDlg.m_iSwitchGroup		 = iSwitchGroup;
	softDlg.m_nEnableWatchdog	 = nEanbleWatchdog;
	softDlg.m_nPlCHeartbeatTimeout = nPLCHeartBeatTime;
	softDlg.fHandle = this->m_hWnd;

	INT_PTR nResponse = softDlg.DoModal();
	if (nResponse == IDOK)
	{
		iniRestartFile.WriteInt(_T("LOG"), _T("ptz"), softDlg.m_iPtz);
		iniRestartFile.WriteInt(_T("LOG"), _T("zoom"), softDlg.m_iZoom);
		iniRestartFile.WriteInt(_T("LOG"), _T("tvwall"), softDlg.m_iTvwall);
		iniRestartFile.WriteInt(_T("LOG"), _T("autoclear"), softDlg.m_iAutoClear);
		iniRestartFile.WriteInt(_T("LOG"), _T("autoclear"), softDlg.m_iSwitchGroup);
		iniRestartFile.WriteString(_T("RESTART"), _T("time"), softDlg.m_csTime);
		iniRestartFile.WriteInt(_T("RESTART"), _T("enable"), softDlg.m_iTimeEnable);
		iniRestartFile.WriteInt(_T("PLCWatchdog"), _T("Enable"), softDlg.m_nEnableWatchdog);
		iniRestartFile.WriteInt(_T("PLCWatchdog"), _T("Timeout"), softDlg.m_nPlCHeartbeatTimeout);
	}
}
void CPlcTextDlg::ClearLogFile(int iClear)
{
	if (iClear == 0)
	{
		return;
	}
	CString cstrPath;
	MStrToWStr(cstrPath, GetCurrentPath());

	CFileFind filefind;
	CString sFilePath(cstrPath + _T("\\log\\*.*"));
	if (filefind.FindFile(sFilePath, 0))
	{
		BOOL bBool = TRUE;
		while(bBool)
		{
			bBool = filefind.FindNextFile();
			if (filefind.IsDots())
			{
				continue;
			}
			CString sFileName = filefind.GetFileName();

			SYSTEMTIME systime;
			::GetLocalTime(&systime);
			CString csTime;
			if (systime.wDay <= 7*iClear)
			{
				int n = 7*iClear - systime.wDay;
				systime.wMonth = systime.wMonth-1;
				systime.wDay = 30 - n;
			}
			else
			{
				systime.wDay = systime.wDay-7*iClear;
			}
			csTime.Format(_T("%04d-%02d-%02d-%02d-%02d-%02d.txt"),  systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

			if (wcscmp(sFileName, csTime)< 0)
			{
				string strFile;
				WStrToMStr(filefind.GetFilePath(), strFile);
				remove(strFile.c_str());
			}
		}
	}
}
void CPlcTextDlg::OnBnClickedCheckListstop()
{
	// TODO: 在此添加控件通知处理程序代码
	if(IsDlgButtonChecked(IDC_CHECK_LISTSTOP)==BST_CHECKED)
		listStop=true;
	else
		listStop = false;
}

void CPlcTextDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (GetDlgItem(IDC_LIST_LOG)->GetSafeHwnd())
	{
		RECT rtDialog;
		GetClientRect(&rtDialog);
		RECT rt;
		if (nType == SIZE_MAXIMIZED)
		{
			rt = m_rtTree;
			rt.bottom = rtDialog.bottom - 5;
			rt.right = rt.right + 80;
			int nRight = rt.right;
			::MoveWindow(GetDlgItem(IDC_TREE1)->GetSafeHwnd(), rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,true);
			rt = m_rtLoglist;
			rt.left = nRight + 5;
			rt.bottom = rtDialog.bottom - 5;
			rt.right = rtDialog.right - 5;
			::MoveWindow(GetDlgItem(IDC_LIST_LOG)->GetSafeHwnd(), rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,true);
			::MoveWindow(GetDlgItem(IDC_LIST_LOG_FILTER)->GetSafeHwnd(), rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,true);

		}
		else if (nType == SIZE_RESTORED)
		{
			::MoveWindow(GetDlgItem(IDC_TREE1)->GetSafeHwnd(), m_rtTree.left, m_rtTree.top, m_rtTree.right - m_rtTree.left, m_rtTree.bottom - m_rtTree.top,true);
	
			::MoveWindow(GetDlgItem(IDC_LIST_LOG)->GetSafeHwnd(), m_rtLoglist.left, m_rtLoglist.top, m_rtLoglist.right - m_rtLoglist.left, m_rtLoglist.bottom - m_rtLoglist.top,true);
			::MoveWindow(GetDlgItem(IDC_LIST_LOG_FILTER)->GetSafeHwnd(), m_rtLoglist.left, m_rtLoglist.top, m_rtLoglist.right - m_rtLoglist.left, m_rtLoglist.bottom - m_rtLoglist.top,true);

		}
	}
}


void CPlcTextDlg::OnBnClickedCheckFilter()
{
	if (IsDlgButtonChecked(IDC_CHECK_FILTER) == BST_CHECKED)
	{
		m_bEnableFilter = true;
		m_LogListFilter.ShowWindow(SW_SHOW);
		m_LogList.ShowWindow(SW_HIDE);
		GetDlgItemText(IDC_EDIT_FILTER,m_strFilter);
		GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
	}
	else
	{
		m_bEnableFilter = false;
		m_LogListFilter.ShowWindow(SW_HIDE);
		m_LogList.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(TRUE);
	}
}

void CPlcTextDlg::dataMessage(const char* buffer)
{
	char dataBuffer[BLOCK_SIZE_COPY]={0};
	memcpy(dataBuffer,buffer,sizeof(dataBuffer));
	int itype =0;
	dataGet->SendMessage(WM_DATASHOW,itype, (LPARAM)(dataBuffer));
	return ;
}

void CPlcTextDlg::OnBnClickedButtonDataviewer()
{
	// TODO: 在此添加控件通知处理程序代码
	if(NULL==dataGet)
	{
		dataGet = new DataViewer();
		dataGet->Create(IDD_DIALOG_DATA);
		dataGet->fHandle = this->m_hWnd;
	}
	dataGet->ShowWindow(SW_SHOWNORMAL);
}

LRESULT CPlcTextDlg::OnDataViewerClose(WPARAM wParam, LPARAM lParam)
{
	delete dataGet;
	dataGet=NULL;
	return 0;
}

LRESULT CPlcTextDlg::OnGroupConfigureRefresh(WPARAM wParam, LPARAM lParam)
{
	LoadInstance::Instance()->group_configure_refresh();
	return 0;
}

LRESULT CPlcTextDlg::OnDataViewerClick(WPARAM wParam, LPARAM lParam)
{
	if(NULL==dataGet)
	{
		dataGet = new DataViewer();
		dataGet->Create(IDD_DIALOG_DATA);
		dataGet->fHandle = this->m_hWnd;
	}
	dataGet->ShowWindow(SW_SHOWNORMAL);
	return 0;
}

void CPlcTextDlg::mySqlConnect()
{
	CString cstrPath;
	MStrToWStr(cstrPath, GetCurrentPath());
	CIniFile iniFile(cstrPath+ _T("\\config\\")+_T("General.ini"));
	int enable=0;
	iniFile.ReadInt(_T("IPCINFOFROMAS300"),_T("Enable"),enable);
	if(enable)
	{
		CString strServerIp,strDBAccount,strDBPassword,strDatabase;
		int port=0;
		iniFile.ReadString(_T("AS300SERVER"),_T("ServerIP"),strServerIp);
		iniFile.ReadString(_T("AS300SERVER"),_T("UserName"),strDBAccount);
		iniFile.ReadString(_T("AS300SERVER"),_T("Password"),strDBPassword);
		iniFile.ReadString(_T("AS300SERVER"),_T("Database"),strDatabase);
		iniFile.ReadInt(_T("AS300SERVER"),_T("Port"),port);

		std::string m_strServerIp,m_strDBAccount,m_strDBPassword,m_strDatabase;
		WStrToMStr(strServerIp, m_strServerIp);
		WStrToMStr(strDBAccount,m_strDBAccount);
		WStrToMStr(strDBPassword,m_strDBPassword);
		WStrToMStr(strDatabase,m_strDatabase);
		int ret =0;
		try
		{
			m_pDBConnector = new CMySQLConnector(const_cast<char*>(m_strServerIp.c_str()),const_cast<char*>(m_strDBAccount.c_str()),
				const_cast<char*>(m_strDBPassword.c_str()),const_cast<char*>(m_strDatabase.c_str()),port);
		}
		catch (CMySQLException& e)
		{
			string strException = e.what_;
			//AfxMessageBox(_UnicodeString(strException.c_str(),CP_ACP));
			//cout << "Exceptions:"<<e.what_.c_str()<<endl;
			// 发生异常后编译器会自动回收对象所占内存，因此对象指针可置空
			m_pDBConnector = NULL;
			ret =1;
		}
		catch(std::exception &e)
		{
			const char *szError = e.what();
			//AfxMessageBox(_UnicodeString(e.what(),CP_ACP));
			//cout << "memory exceptions:"<< e.what()<<endl;
			// 发生异常后编译器会自动回收对象所占内存，因此对象指针可置空
			m_pDBConnector = NULL;
			ret =1;
		}
		if(ret==0)
			mAs300DatabaseConnect = true;
	}
}

void CPlcTextDlg::getIpcStatusMysql()
{
	if(!mAs300DatabaseConnect)
		mySqlConnect();
	else
	{
		try
		{
			// 执行一条SQL指令，结果集保存在CMyResult类对象res中
			// 查询所有离线的设备,设备离线时status值为2，初装设备status值为0

			//CMyResult res = m_pDBConnector->Query("SELECT `deviceid` from devices where `status` = 2 ORDER BY `deviceid`");
			CMyResult res = m_pDBConnector->Query("SELECT deviceid from devices where `status` = 1");
			do 
			{
				// CMyResult支持以字段名和列索引两种方式取得字段值，这里用的是字段名
				std::string strDeviceID = res["deviceid"];
				std::map<std::string,ipcInfo>::iterator iterOnline = mIpcInfoMap.find(strDeviceID);
				if(iterOnline!=mIpcInfoMap.end())
					iterOnline->second.ipcStatus=2;
			} while (++res);	//推荐使用这种循环方式
		}
		catch (CMySQLException& e)
		{
			string strException = e.what_;
			//AfxMessageBox(_UnicodeString(strException.c_str(),CP_ACP));
			//cout << "Exceptions:"<<e.what_.c_str()<<endl;
			// 发生异常后编译器会自动回收对象所占内存，因此对象指针可置空
			m_pDBConnector = NULL;
			mAs300DatabaseConnect = false;
		}
		catch(std::exception &e)
		{
			const char *szError = e.what();
			//AfxMessageBox(_UnicodeString(e.what(),CP_ACP));
			//cout << "memory exceptions:"<< e.what()<<endl;
			// 发生异常后编译器会自动回收对象所占内存，因此对象指针可置空
			m_pDBConnector = NULL;
			mAs300DatabaseConnect = false;
		}
		for(std::map<std::string,ipcInfo>::iterator ipcInfoIte=mIpcInfoMap.begin();ipcInfoIte!=mIpcInfoMap.end();ipcInfoIte++)
		{
			int iState = ipcInfoIte->second.ipcStatus;
			std::string sOnLine, sOffLine;
			WStrToMStr(_CS("OnLine"), sOnLine);
			WStrToMStr(_CS("OffLine"), sOffLine);
			std::string sState = iState == 2 ? sOnLine : sOffLine;
			char szlog[LOG_MAX_LEN] = {0};
			_snprintf(szlog, LOG_MAX_LEN-1, "(%s, %s)", ipcInfoIte->second.ipcId.c_str(), sState.c_str());
			wchar_t *p = UTF8ToUnicode(ipcInfoIte->second.ipcName.c_str());
			CString strName(p);
			CString strIdState(szlog);
			CString csChlId(ipcInfoIte->second.ipcId.c_str());
			free(p);
			InsertTree(ipcInfoIte->second.group, iState, strIdState, strName, csChlId);
			char ipcStateW[100]={0};
			sprintf_s(ipcStateW,sizeof(ipcStateW)-1,"%d,%d,%d",ipcInfoIte->second.group,ipcInfoIte->second.index,iState); //group index status
			LoadInstance::Instance()->set_ipc_state(ipcStateW);
			ipcInfoIte->second.ipcStatus=0;
		}
	}

}

LRESULT CPlcTextDlg::OnPtzTestClick(WPARAM wParam,LPARAM lParam)
{
	if(NULL==mPtzTest)
	{
		mPtzTest = new CPtzTestPanel();
		mPtzTest->Create(IDD_DIALOG_TESTPANEL);
		mPtzTest->fHandle = this->m_hWnd;
	}
	mPtzTest->ShowWindow(SW_SHOWNORMAL);
	return 0;
}

LRESULT CPlcTextDlg::OnPtzOperation(WPARAM wParam,LPARAM lParam)
{
	PtzTestParam* ptzConfig;
	ptzConfig = (PtzTestParam*)wParam;
	LoadInstance::Instance()->ptz_operation_test(ptzConfig->nGroup,ptzConfig->nIndex,ptzConfig->cmdIndex,ptzConfig->type);
	return 0;
}
