
// PlcServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PlcServer.h"
#include "PlcServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma comment(lib, "Version.lib ")

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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPlcServerDlg 对话框

CPlcServerDlg::CPlcServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlcServerDlg::IDD, pParent)
	//,m_hServerThread(NULL)
	,m_hMainThread(NULL)
	,m_hLoopThread(NULL)
	,m_bStart(FALSE)
	,m_pLog(NULL)
	,m_sockSrv(INVALID_SOCKET)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CPlcServerDlg::~CPlcServerDlg()
{
	UInitData();
	if (m_pLog)
	{
		delete m_pLog;
		m_pLog = NULL;
	}
}

void CPlcServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_RevInfolist);
	DDX_Control(pDX, IDC_COMBO_DB, m_DBCombox);
}

BEGIN_MESSAGE_MAP(CPlcServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CPlcServerDlg::OnBtnStartServer)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_GETINFO, &CPlcServerDlg::OnBnClickedBtnGetinfo)
	ON_BN_CLICKED(IDC_BTN_SETINFO, &CPlcServerDlg::OnBnClickedBtnSetinfo)
	ON_BN_CLICKED(IDC_BIT7, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT6, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT5, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT4, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT3, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT2, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT1, &CPlcServerDlg::OnBnClickedBit)
	ON_BN_CLICKED(IDC_BIT0, &CPlcServerDlg::OnBnClickedBit)
	ON_EN_CHANGE(IDC_EDIT_VALUE, &CPlcServerDlg::OnEnChangeEditValue)
	ON_BN_CLICKED(IDC_BTN_BEGIN, &CPlcServerDlg::OnBnClickedBtnBegin)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CPlcServerDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CPlcServerDlg 消息处理程序

BOOL CPlcServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return FALSE;
	}
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			WSACleanup( );
			return FALSE; 
	}

	CString title = _T("MAGO控制系统模拟PLC服务     ") + GetVersionData() + _T("    ") + GetBuildTime();
	this->SetWindowText(title);

	m_RevInfolist.SetHorizontalExtent(2000);

	m_pLog = new Logger(GetCurrentPath().c_str(),LogLevelAll);

	InitDBData();
	UpdateDataBtnState(TRUE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

std::string CPlcServerDlg::GetCurrentPath()
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

CString CPlcServerDlg::GetBuildTime()
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
	return _T("编译日期: ") + fileWriteTime; 
}

CString CPlcServerDlg::GetVersionData()
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
		return _T("版本号: ") + strVersion;
	}
}

HBRUSH CPlcServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
		HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		// TODO:  在此更改 DC 的任何属性
		if (pWnd->GetDlgCtrlID() == IDC_PROMT)
		{
			if (m_hMainThread != NULL)
			{
				pDC->SetTextColor(RGB(0, 255, 0));
			}
		} 
		// TODO:  如果默认的不是所需画笔，则返回另一个画笔
		return hbr;
}
void CPlcServerDlg::InitDBData()
{
	//初始化原始map数据
	m_groupDataMap.empty();
	for(int i = 1; i < 4; i++)
	{
		unsigned char *pArray = new unsigned char[300];
		memset(pArray, 0, sizeof(300));
		for (int ii = 0; ii < 300; ii++)
		{
			pArray[ii] = 0;
			if (ii == 189)
			{
				pArray[ii] = i & 0xff;
			}
		}
		m_groupDataMap.insert(std::make_pair(i+500, pArray));
	}
/////////////////////用于测试的临时数据
	//std::map<int, unsigned char*>::iterator ite = m_groupDataMap.find(503);
	//int bb1 = ite->second[188]<<8 | ite->second[189];
	//int bb = ite->second[100];

	m_DBCombox.AddString(_T("501"));
	m_DBCombox.AddString(_T("502"));
	m_DBCombox.AddString(_T("503"));
	m_DBCombox.AddString(_T("504"));
	m_DBCombox.AddString(_T("505"));
	m_DBCombox.AddString(_T("506"));
	m_DBCombox.AddString(_T("507"));
	m_DBCombox.AddString(_T("508"));
	m_DBCombox.AddString(_T("509"));
	m_DBCombox.AddString(_T("510"));
	m_DBCombox.SetCurSel(0);

	GetDlgItem(IDC_EDIT_BYTE)->SetWindowText(_T("0"));
	GetDlgItem(IDC_EDIT_VALUE)->SetWindowText(_T("0"));
	GetDlgItem(IDC_EDIT_TIME)->SetWindowText(_T("5"));
}

void CPlcServerDlg::UInitData()
{
	for(std::map<int, unsigned char*>::iterator ite = m_groupDataMap.begin(); ite != m_groupDataMap.end(); ite++)
	{
		delete []ite->second;
	}
    m_groupDataMap.empty();
}

int CPlcServerDlg::SplitCString(CString &strIn, CStringArray& strAryRe, char str)
{
	strAryRe.RemoveAll();   
	for(int i=0;i< strIn.GetLength();i++)   
	{   
		if(strIn.GetAt(i)== str)   
		{   
			strAryRe.Add(strIn.Left(i));         //去掉右边   
			for(int j=0;j < (strAryRe.GetSize()-1); j++)
			{   
				strAryRe[strAryRe.GetSize()-1] = strAryRe[strAryRe.GetSize()-1].Right(strAryRe[strAryRe.GetSize()-1].GetLength()-strAryRe[j].GetLength()-1);  //去掉左边   
			}   
		}      
	}
	return strAryRe.GetSize();
}

void CPlcServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlcServerDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPlcServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlcServerDlg::ServerPort(int fd)
{
	_daveOSserialType s;
	daveInterface * di;
	daveConnection * dc;
	int waitCount,res/*,pcount*/;
	waitCount = 0;
	daveSetDebug(0);//daveDebugAll;
	//pcount=0;
	s.rfd=(HANDLE)fd;
	s.wfd=(HANDLE)fd;
	di =daveNewInterface(s,"IF1",0,daveProtoISOTCP,daveSpeed187k);
	di->timeout=900000;
	dc=daveNewConnection(di,2,0,2);
	while (m_bWorkThreadRun) 
	{
		Sleep(100);
		dc->AnswLen=_daveReadISOPacket(dc->iface, dc->msgIn);
		if (dc->AnswLen>0)
		{
			res=dc->AnswLen;    
			waitCount = 0;
			analyze(dc);
			//pcount++;
		}
		else
		{
			//waitCount++;
		}    
	}
	return;
}

//解析plcTest发送过来的数据请求包
void CPlcServerDlg::analyze(daveConnection * dc) 
{
	ISOpacket * p,* p2;
	uc resp[2000] ={0};
	//uc r5[]={0x03,0x00,0x00,0x16,0x11,0xd0,0,0,0,1,0,0xc0,1,9,0xc1,2,1,2,0xc2,2,1,0};
	uc r5[]={0x03, 0x00, 0x00, 0x16, 0x11, 0xD0, 0x00, 0x01, 0x00, 0x03, 0x00, 0xC0, 0x01, 0x09, 0xC1, 0x02, 0x01, 0x00, 0xC2, 0x02, 0x01, 0x02};
	uc r5Ex[]={0x03, 0x00, 0x00, 0x2F, 0x02, 0xF0, 0x80, 0x32, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x1A, 0x00, 0x00, 0x04, 0x01, 0xFF, 0x04, 0x00, 0xB0};
	uc data[]={0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
	int haveResp=0;
	PDU p1,pr;
	p= (ISOpacket*) dc->msgIn;
	dc->needAckNumber=-1;		// Assume no ack
   
	if (p->func==0xf0) 
	{
		//p1.header=((uc*)p)+sizeof(ISOpacket);
		dc->PDUstartI=sizeof(ISOpacket);
		_daveSetupReceivedPDU(dc, &p1);

		pr.header=resp+sizeof(ISOpacket);
		p2= (ISOpacket*) resp;
		p2->ch1=p->ch2;
		p2->ch2=p->ch1;
		if (p1.param[0]==daveFuncRead) 
		{
			//_daveHandleRead(&p1,&pr);
			haveResp=1;
			//p2->len=pr.hlen+pr.plen+pr.dlen+7;
			short iByteNum = p1.param[6]<<8 | p1.param[7];
			short iDB = p1.param[8]<<8 | p1.param[9];
			short iOffSet = (p1.param[12]<<8 | p1.param[13])/8;
		
			short ff =  htons(25 + iByteNum);
			char vv[2];
			memcpy(vv, &ff, sizeof(short));
			r5Ex[2] = vv[0]; 
			r5Ex[3] = vv[1];      //设置总长度

			ff =  htons(4 + iByteNum);
			memcpy(vv, &ff, sizeof(short));
			r5Ex[15] = vv[0]; 
			r5Ex[16] = vv[1];      //设置数据长度

			ff = htons(iByteNum*8);
			memcpy(vv, &ff, sizeof(short));
			r5Ex[23] = vv[0]; 
			r5Ex[24] = vv[1];

			memcpy(resp, r5Ex, sizeof(r5Ex));
			unsigned short iData = 0;
			CSingleLock singleLock(&m_Datalock);
			singleLock.Lock(); 
			std::map<int, unsigned char*>::iterator ite = m_groupDataMap.find(iDB);
			if (ite != m_groupDataMap.end())
			{
				if (iByteNum < 300)
				{
					int bb = ite->second[65];
					//GetDlgItem(IDC_EDIT_VALUE)->SetWindowText(strValue);

					memcpy(resp + sizeof(r5Ex), ite->second, iByteNum);
				}
			}
			singleLock.Unlock();
		} 
		else if (p1.param[0]==daveFuncWrite)
		{
			_daveHandleWrite(&p1,&pr);
			haveResp=1;
			p2->len=pr.hlen+pr.plen+pr.dlen+7;
		} 
		else if (p1.param[0]==240)
		{
			printf("PDU function code: %d, negociate PDU len\n",p1.param[0]);
			_daveDump("packet:",dc->msgIn,dc->msgIn[3]);
			memcpy(resp, dc->msgIn, dc->msgIn[3]);
			resp[23]=960 / 0x100;
			resp[24]=960 % 0x100;
			haveResp=1;
		}
		else if (p1.param[0]==0) 
		{
			printf("PDU function code: %d, system Message ?\n",p1.param[0]);
			//haveResp=handleSystemMessage(&p1,&pr);
			pr.header[4]=p1.header[4];		// give the PDU a number
			haveResp=1;
			p2->len=pr.hlen+pr.plen+pr.dlen+7;
		} 
		else 
		{
			printf("Unsupported PDU function code: %d\n",p1.param[0]);
		}
	}    
	if (p->func==0xe0) 
	{
		int rack=*(dc->msgIn+17)-1;
		int slot=*(dc->msgIn+18);
		printf("Connect to rack:%d slot:%d \n", rack, slot);

		*(r5+16)=rack+1;
		*(r5+17)=slot;    
		memcpy(resp, r5, sizeof(r5));
		haveResp=1;
	}    
	if (haveResp)
	{
		// simulating CP response delay:    
		//usleep(10000);
		//#define sim_broken_transport	
//#undef sim_broken_transport	
//#ifdef sim_broken_transport
//		double pr=random();
//		pr/=RAND_MAX;
//
//		if (pr>=0.90) {
//			LOG2("faking broken transport: %0.2f >= 0.95 \n",pr);
//		} else   
		send((SOCKET)dc->iface->fd.wfd,(const char*)resp,resp[3], 0);
		//_daveSendISOPacket(daveConnection * dc, int size) {
//#else	    
//		//write(dc->iface->fd.wfd,resp,resp[3]);
//#endif		    	
//
//		//aveDump("I send:",resp,resp[3]);    
//#endif	
	}	
}

//CString CPlcServerDlg::GetCurrentPath()
//{
//	HMODULE module = GetModuleHandle(0); 
//	char pFileName[MAX_PATH]; 
//	GetModuleFileNameA(module, pFileName, MAX_PATH); 
//
//	CString csFullPath(pFileName); 
//	int nPos = csFullPath.ReverseFind( _T('\\') );
//	if( nPos < 0 ) 
//	{
//		return CString(_T("")); 
//	}
//	else 
//	{
//		return csFullPath.Left(nPos); 
//	}
//}

void CPlcServerDlg::OnBtnStartServer()
{
	if (!m_hMainThread)
		StartServer();
	else
		StopServer();
	//MainFunction();
}

void CPlcServerDlg::StartServer()
{
	if (m_hMainThread)
	{
		StopMainThread();
	}
	DWORD dwThreadID;
	m_hMainThread = CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)WorkParseMainThread , this , 0, &dwThreadID);
	GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("停止SiemensPLC服务"));
	GetDlgItem(IDC_PROMT)->SetWindowText(_T("服务已开启!"));
}

void CPlcServerDlg::MainFunction()
{
	m_sockSrv=socket(AF_INET,SOCK_STREAM,0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(102);
	bind(m_sockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	listen(m_sockSrv,5);
	SOCKADDR_IN addrClient;
	int len=sizeof(SOCKADDR);
    ShowInfoToList(_T("服务已开启，正在监听端口102!"));
	m_bWorkThreadRun = true;
	while(1)
	{
		SOCKET sockConn=accept(m_sockSrv,(SOCKADDR*)&addrClient,&len);
		if (sockConn == INVALID_SOCKET)
			break;

		char * ipaddr=NULL;
		char addr[20];
		in_addr inaddr;
		ipaddr = inet_ntoa(addrClient.sin_addr);
		strcpy(addr,ipaddr);  
		CString strIp(addr);
		CString strPort;
		strPort.Format(_T("%d"), htons(addrClient.sin_port));
		ShowInfoToList(_T("设备已连接，ip:") + strIp + _T(" port:") + strPort);
		m_nSocket = sockConn;
		//char sendBuf[100];
		ThreadParam ThreadParamTmp;
		ThreadParamTmp._pthis = this;
		ThreadParamTmp._socket = sockConn;
		DWORD dwThreadID;
		HANDLE hServerThread = CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)WorkParseThread, &ThreadParamTmp, 0, &dwThreadID);
		m_vServerThread.push_back(hServerThread);
		//sprintf(sendBuf,"welcome %s to me",inet_ntoa(addrClient.sin_addr));
		//send(sockConn,sendBuf,strlen(sendBuf)+1,0);
		//break;
//		char recvBuf[100];
//		recv(sockConn,recvBuf,100,0);
//		printf("%s\n",recvBuf);
	//	closesocket(sockConn);
	}
	m_bWorkThreadRun = false;
}

void CPlcServerDlg::WorkParseMainThread(LPVOID lpParam, LPARAM lParam)
{
	CPlcServerDlg* pThis = (CPlcServerDlg*)lpParam;
	pThis->MainFunction();
}


void CPlcServerDlg::WorkParseThread(LPVOID lpParam, LPARAM lParam)
{
	ThreadParam* pThis = (ThreadParam*)lpParam;
	pThis->_pthis->ServerPort(pThis->_socket);
}

void CPlcServerDlg::StopServer()
{
	if(m_hMainThread && m_sockSrv != INVALID_SOCKET)
	{
		DisConnect(m_sockSrv);
		WaitForSingleObject(m_hMainThread, INFINITE);
		CloseHandle(m_hMainThread);
		m_hMainThread = NULL;
		m_sockSrv = INVALID_SOCKET;
		HANDLE hArray[64] = {0};
		int nCount = 0;
		for (list<HANDLE>::iterator it = m_vServerThread.begin(); it != m_vServerThread.end();it ++)
		{
			hArray[nCount ++] = *it;
			if (nCount>= 64)
			{
				WaitForMultipleObjects(nCount,hArray,true,INFINITE);
				nCount = 0;
			}
		}
		if (nCount > 0)
		{
			if (nCount>= 64)
			{
				WaitForMultipleObjects(nCount,hArray,true,INFINITE);
				nCount = 0;
			}
		}
		m_vServerThread.clear();
		GetDlgItem(IDC_PROMT)->SetWindowText(_T("服务已停止!"));
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("开启SiemensPLC服务"));
	}
}

void CPlcServerDlg::StopMainThread()
{
	if(m_hMainThread)
	{
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_hMainThread, 400))
		{
			TerminateThread(m_hMainThread,-1);
			m_hMainThread = NULL;
		}
	}	
}

void CPlcServerDlg::ShowInfoToList(CString str, BOOL bType)
{
	CTime CurTime = CTime::GetCurrentTime();
	CString StrTime;
	StrTime.Format(_T("%.4d-%.2d-%.2d %.2d:%.2d:%.2d  "), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
	StrTime += str;
	ShowInfo(StrTime, bType);
}

void CPlcServerDlg::ShowInfo(CString str, BOOL bType)
{
	if(bType)
	{
		m_RevInfolist.AddString(str, RGB(0,0,255));
		if (m_RevInfolist.GetCount() > 400)
		{
			m_RevInfolist.ResetContent();
		}
	}
	else
	{
		//m_SendInfoList.AddString(str, RGB(0,0,255));
		m_RevInfolist.AddString(str, RGB(0,0,255));
		if (m_RevInfolist.GetCount() > 400)
		{
			m_RevInfolist.ResetContent();
		}
	}
	m_pLog->TraceInfo((char*)str.GetBuffer(0));
}

void CPlcServerDlg::OnBnClickedBtnGetinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strDB, strByte, strValue;
	m_DBCombox.GetWindowText(strDB);
	GetDlgItem(IDC_EDIT_BYTE)->GetWindowText(strByte);
	CSingleLock singleLock(&m_Datalock);  
	singleLock.Lock();  
	std::map<int, unsigned char*>::iterator ite = m_groupDataMap.find(_ttoi(strDB));
	if (ite != m_groupDataMap.end())
	{
		if (_ttoi(strByte) < 300)
		{
			int iCurValue = ite->second[_ttoi(strByte)] & 0xff;
			strValue.Format(_T("%d"), iCurValue);
			GetDlgItem(IDC_EDIT_VALUE)->SetWindowText(strValue);
			Int2Bit(iCurValue);
		}
	}
	singleLock.Unlock(); 
}

void CPlcServerDlg::Int2Bit(int iValue)
{
	//1053为第0位的checkbox
    for (int i = 0; i <= 7; i++)
    {
		((CButton*)GetDlgItem(1053+i))->SetCheck((1<<i) & iValue);
    }
}

int CPlcServerDlg::bit2Int()
{
	int iValue = 0;
	for(int i = 0; i <= 7; i++)
	{
		int iBit = ((CButton*)GetDlgItem(1053+i))->GetCheck();
		iValue |= iBit<<i;
	}
	return iValue;
}

void CPlcServerDlg::OnBnClickedBtnSetinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	SetInfo();
}
void CPlcServerDlg::SetInfo()
{
	CString strDB, strByte, strValue;
	m_DBCombox.GetWindowText(strDB);
	GetDlgItem(IDC_EDIT_BYTE)->GetWindowText(strByte);
	CSingleLock singleLock(&m_Datalock);  
	singleLock.Lock();  
	std::map<int, unsigned char*>::iterator ite = m_groupDataMap.find(_ttoi(strDB));
	if (ite != m_groupDataMap.end())
	{
		if (_ttoi(strByte) < 300)
		{
			GetDlgItem(IDC_EDIT_VALUE)->GetWindowText(strValue);
			ite->second[_ttoi(strByte)] = _ttoi(strValue) & 0xff;
		}
	}
	singleLock.Unlock(); 
}
void CPlcServerDlg::OnBnClickedBit()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strValue;
	strValue.Format(_T("%d"), bit2Int());
	GetDlgItem(IDC_EDIT_VALUE)->SetWindowText(strValue);
}


void CPlcServerDlg::OnEnChangeEditValue()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString strValue;
	GetDlgItem(IDC_EDIT_VALUE)->GetWindowText(strValue);
	Int2Bit(_ttoi(strValue));
}

void CPlcServerDlg::OnBnClickedBtnBegin()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bStart)
	{
		return;
	}
	UINT iTime = GetDlgItemInt(IDC_EDIT_TIME);
	if(iTime <= 0)
	{
		MessageBox(_T("输入的值不合法"), _T("提示"));
		return;
	}
	std::string strPath = GetCurrentPath();
	CString csPath;
	MStrToWStr(csPath, strPath);
	CIniFile iniFile(csPath+_T("\\config.ini"));
	CStringArray DataArray;
	iniFile.ReadSectionString(_T("DBDATA"), DataArray);

	m_DbDataMap.clear();
	for(int i = 0; i < DataArray.GetSize(); i++)
	{
		std::vector<CString> vectorData;
		vectorData = Split(DataArray[i]);

		DBDATA stInfo;
		stInfo.iDB = _ttoi(vectorData[0]);
		stInfo.iBit = _ttoi(vectorData[1]);
		stInfo.iValue = _ttoi(vectorData[2]);
		
		m_DbDataMap.insert(std::make_pair(i,stInfo));
	}
	if (m_DbDataMap.empty())
	{
		MessageBox(_T("配置文件为空"), _T("提示"));
		return;
	}
	StartLoopThread();
}
void CPlcServerDlg::StartLoopThread()
{
	if (m_hLoopThread == NULL)
	{
		m_hLoopThread = (HANDLE)_beginthreadex(NULL, 0, &LoopThread, this, 0, 0);
	}
}
void CPlcServerDlg::StopLoopThread()
{
	if (m_hLoopThread != NULL)
	{
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_hLoopThread,1000))
		{
			TerminateThread(m_hLoopThread,-1);
			m_hLoopThread = NULL;
			m_bStart = FALSE;
			UpdateDataBtnState(TRUE);
		}
	}
}
unsigned __stdcall CPlcServerDlg::LoopThread( void* pArguments)
{
	CPlcServerDlg *pThis = (CPlcServerDlg *)pArguments;
	pThis->RunLoopThread();

	return 0;
}
void CPlcServerDlg::RunLoopThread()
{
	m_bStart = TRUE;
	UpdateDataBtnState(FALSE);
	while(1)
	{
		for (std::map<int, DBDATA>::iterator ite = m_DbDataMap.begin(); ite != m_DbDataMap.end(); ite++)
		{
			
			CString csDB,csByte,csValue;
			int iCurValue;
		
			csDB.Format(_T("%d"), ite->second.iDB);
			csByte.Format(_T("%d"), ite->second.iBit);
			iCurValue = pow((double)2, (double)ite->second.iValue);
			csValue.Format(_T("%d"), iCurValue);
			m_DBCombox.SetWindowText(csDB);
			GetDlgItem(IDC_EDIT_BYTE)->SetWindowText(csByte);
			GetDlgItem(IDC_EDIT_VALUE)->SetWindowText(csValue);
			Int2Bit(iCurValue);
			SetInfo();
			UINT iTime = GetDlgItemInt(IDC_EDIT_TIME);
			Sleep(iTime*1000);
		}
	}
}
void CPlcServerDlg::OnBnClickedBtnCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	StopLoopThread();
}
BOOL CPlcServerDlg::MStrToWStr(CStringW & strCS, std::string & sStr)
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

std::vector<CString> CPlcServerDlg::Split(CString string, CString separator)
{
	CString oriStr=string;
	std::vector<CString> strVec;
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
void CPlcServerDlg::UpdateDataBtnState(BOOL bBol)
{
	GetDlgItem(IDC_EDIT_BYTE)->EnableWindow(bBol);
	GetDlgItem(IDC_EDIT_VALUE)->EnableWindow(bBol);
	GetDlgItem(IDC_COMBO_DB)->EnableWindow(bBol);
	GetDlgItem(IDC_BTN_GETINFO)->EnableWindow(bBol);
	GetDlgItem(IDC_BTN_SETINFO)->EnableWindow(bBol);
	GetDlgItem(IDC_BTN_BEGIN)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT0)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT1)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT2)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT3)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT4)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT5)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT6)->EnableWindow(bBol);
	GetDlgItem(IDC_BIT7)->EnableWindow(bBol);
	GetDlgItem(IDC_EDIT_TIME)->EnableWindow(bBol);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(!bBol);
}
BOOL CPlcServerDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	StartLoopThread();
	return CDialog::DestroyWindow();
}
