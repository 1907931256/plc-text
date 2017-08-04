// GroupCfgSet.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "GroupCfgSet.h"
#include "ConfigSetDlg.h"

// GroupCfgSet 对话框

IMPLEMENT_DYNAMIC(GroupCfgSet, CDialog)

GroupCfgSet::GroupCfgSet(CWnd* pParent /*=NULL*/)
	: CDialog(GroupCfgSet::IDD, pParent)
	,m_pMainCfgDlg(NULL)
	,m_nCurIndex(0)
	,m_nMode(0)
	,m_nTvwallScreen(1)
	,m_bCheckBol(FALSE)
{
	m_brushSel.CreateSolidBrush(RGB(0, 255, 255));
}

GroupCfgSet::~GroupCfgSet()
{
}

void GroupCfgSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GROUP_NUM, m_ComBoxGroup);
	DDX_Control(pDX, IDC_TVWALL_TOTLE_SCREEN, m_ComBoxScreenIndex);
	DDX_Control(pDX, IDC_TVWALL_SCREEN_NUM, m_ScreenSplitNum);
	DDX_Control(pDX, IDC_EDITTV_PORT, m_TvWallPort);
	DDX_Control(pDX, IDC_IPADDRESS, m_TvWallIp);
	DDX_Control(pDX, IDC_LIST_IPC_GROUP, m_Ipclist);
	DDX_Control(pDX, IDC_TVWALL_MODE, m_ComBoxMode);
	DDX_Control(pDX, IDC_LIST_IPC_ZOOM, m_IpcListZoom);
	DDX_Control(pDX, IDC_CHECK_FREECUT, m_cCheckFreeCut);
}


BEGIN_MESSAGE_MAP(GroupCfgSet, CDialog)
	ON_BN_CLICKED(IDC_BTN_TVWALL_CROUP, &GroupCfgSet::OnBnClickedBtnTvwallCroup)
	ON_CBN_SELCHANGE(IDC_GROUP_NUM, &GroupCfgSet::OnCbnSelchangeGroupNum)
	ON_CBN_SELCHANGE(IDC_TVWALL_SCREEN_NUM, &GroupCfgSet::OnCbnSelchangeTvwallScreenNum)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_IPC_GROUP, &GroupCfgSet::OnNMDblclkListIpcGroup)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_IPC_ZOOM, &GroupCfgSet::OnNMDblclkListIpcZoom)
	ON_BN_CLICKED(IDC_BTN_ADD, &GroupCfgSet::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_SET_ZOOM, &GroupCfgSet::OnBnClickedBtnSetZoom)
	ON_CBN_SELCHANGE(IDC_TVWALL_TOTLE_SCREEN, &GroupCfgSet::OnCbnSelchangeTvwallTotleScreen)
	ON_CBN_SELCHANGE(IDC_TVWALL_MODE, &GroupCfgSet::OnCbnSelchangeTvwallMode)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_BUTTON1, &GroupCfgSet::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK_FREECUT, &GroupCfgSet::OnBnClickedCheckFreecut)
	ON_BN_CLICKED(IDC_BUTTON2, &GroupCfgSet::OnBnClickedButton2)
END_MESSAGE_MAP()

BOOL GroupCfgSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i = 1; i <= 128; i++)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_ComBoxGroup.AddString(str);
	}
	m_ComBoxGroup.SetCurSel(0);

	for (int i = 1; i <= 4; i ++)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_ComBoxScreenIndex.AddString(str);
	}
	m_ComBoxScreenIndex.SetCurSel(0);

	for (int i = 0; i <= 4; i ++)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_ComBoxMode.AddString(str);
	}
	m_ComBoxMode.SetCurSel(0);

	CString csScreen[8] = {_T("1"),_T("2"),_T("3"),_T("4"),_T("6"),_T("8"),_T("9"),_T("16"),};
	for (int i = 0; i < 8; i ++)
	{
		m_ScreenSplitNum.AddString(csScreen[i]);
		m_ScreenSplitNum.SetItemData(i, _ttoi(csScreen[i]));
	}
	m_ScreenSplitNum.SetCurSel(0);

	m_TvWallIp.SetWindowText(_T("192.168.0.0"));
    m_TvWallPort.SetWindowText(_T("33445"));

	DWORD dwExtendedStyle = m_Ipclist.GetExtendedStyle();
	dwExtendedStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	
	m_Ipclist.SetExtendedStyle(dwExtendedStyle);
	int nIndex = 0;

	m_Ipclist.InsertColumn(nIndex++, _T("device id"), LVCFMT_LEFT,120);
	m_Ipclist.InsertColumn(nIndex++, _T("device name"), LVCFMT_LEFT,130);
	m_Ipclist.InsertColumn(nIndex++, _T("ip"), LVCFMT_LEFT, 110);
	m_Ipclist.InsertColumn(nIndex++, _T("port"), LVCFMT_LEFT, 50);
	m_Ipclist.InsertColumn(nIndex++, _T("company"), LVCFMT_LEFT, 60);
	m_Ipclist.InsertColumn(nIndex++, _T("user"), LVCFMT_LEFT, 60);
	m_Ipclist.InsertColumn(nIndex++, _T("password"), LVCFMT_LEFT, 70);
	m_Ipclist.InsertColumn(nIndex++, _T("device type"), LVCFMT_LEFT, 60);

	m_IpcListZoom.SetExtendedStyle(dwExtendedStyle);
	nIndex = 0;
	m_IpcListZoom.InsertColumn(nIndex++, _T("device id"), LVCFMT_LEFT,120);
	m_IpcListZoom.InsertColumn(nIndex++, _T("device name"), LVCFMT_LEFT,130);
	m_IpcListZoom.InsertColumn(nIndex++, _T("ip"), LVCFMT_LEFT, 110);
	m_IpcListZoom.InsertColumn(nIndex++, _T("port"), LVCFMT_LEFT, 50);
	m_IpcListZoom.InsertColumn(nIndex++, _T("company"), LVCFMT_LEFT, 60);
	m_IpcListZoom.InsertColumn(nIndex++, _T("user"), LVCFMT_LEFT, 60);
	m_IpcListZoom.InsertColumn(nIndex++, _T("password"), LVCFMT_LEFT, 70);
	m_IpcListZoom.InsertColumn(nIndex++, _T("device type"), LVCFMT_LEFT, 60);

	InitWind();
	
	return TRUE;
}

void GroupCfgSet::RefrushIpc()
{
   CleanDeviceList();
   RenewSplitScreen(1, 0);

   CString csGroupIndex;
   csGroupIndex.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);

   IpcGroupMap::iterator ite = m_pMainCfgDlg->m_IpcGroupMap.find(_ttoi(csGroupIndex));
   if (ite != m_pMainCfgDlg->m_IpcGroupMap.end())
   {
	   int iRow = 0;
	   for (IpcList::iterator it = ite->second.begin(); it != ite->second.end(); it++)
	   {
		   DeviceItemInfo *pItemInfo = new DeviceItemInfo();
		   int iCom = 0;
		   int nItem = m_Ipclist.InsertItem(iRow, _T(""));
		   m_Ipclist.SetItemText(iRow, iCom++, it->csDeviceId);
		   m_Ipclist.SetItemText(iRow, iCom++, it->csDeviceName);
		   m_Ipclist.SetItemText(iRow, iCom++, it->csDeviceIp);
		   m_Ipclist.SetItemText(iRow, iCom++, it->csDevicePort);
		   if (_T("2") == it->csComponyType)
		   {
			    m_Ipclist.SetItemText(iRow, iCom++, _T("Axis"));
		   }
		   else if (_T("4") == it->csComponyType)
		   {
			    m_Ipclist.SetItemText(iRow, iCom++, _T("HIK"));
		   }
		   else
		   {
			    m_Ipclist.SetItemText(iRow, iCom++, _T("DEV 2.0"));
		   }
		   m_Ipclist.SetItemText(iRow, iCom++, it->csUserName);
		   m_Ipclist.SetItemText(iRow, iCom++, it->csUserPwd);
		   m_Ipclist.SetItemText(iRow, iCom++, it->csDeviceType);
		   iRow++;
		   pItemInfo->csDeviceId = it->csDeviceId;
		   pItemInfo->csDeviceName = it->csDeviceName;
		   pItemInfo->csDeviceIp = it->csDeviceIp;
		   pItemInfo->csDevicePort = it->csDevicePort;
		   pItemInfo->csComponyType = it->csComponyType;
		   pItemInfo->csUserName = it->csUserName;
		   pItemInfo->csUserPwd = it->csUserPwd;
		   pItemInfo->csDeviceType = it->csDeviceType;
		   m_Ipclist.SetItemData(nItem, (DWORD)pItemInfo);
	   }
   }
}

void GroupCfgSet::CleanIpcZoom()
{
	int iRowCount = m_IpcListZoom.GetItemCount();
	for(int i=0; i < iRowCount; i++)
	{
		DeviceInfoZoom *pItemInfo = (DeviceInfoZoom*)m_IpcListZoom.GetItemData(i);
		delete pItemInfo;
	}
	m_IpcListZoom.DeleteAllItems();
}
void GroupCfgSet::CleanDeviceList()
{
	int iRowCount = m_Ipclist.GetItemCount();
	for(int i=0; i < iRowCount; i++)
	{
		DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Ipclist.GetItemData(i);
		delete pItemInfo;
	}
	m_Ipclist.DeleteAllItems();
}

// GroupCfgSet 消息处理程序

void GroupCfgSet::OnBnClickedBtnTvwallCroup()
{
	// TODO: 在此添加控件通知处理程序代码
	//CString strSelGroupNum;
	//m_ComBoxGroup.GetWindowText(strSelGroupNum);
	for (int i = 0; i < m_Ipclist.GetItemCount(); i++)
	{
		m_Ipclist.SetCheck(i, FALSE);
	}
	CString strSelGroupNum;
	strSelGroupNum.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);
	if (!PathFileExists(m_csIniPath))
	{
		if(!CreateDirectory(m_csIniPath, NULL))
		{
			MessageBox(_T("创建文件夹失败!"));
		}
	}
	CString csIniFileName = m_csIniPath + _T("Group_") + strSelGroupNum + _T(".ini");
	CIniFile iniFile(csIniFileName);
	iniFile.WriteString(GROUPID, ID, strSelGroupNum); //写组号
	//写TVWALL分组信息
   // 分组序号1,组内TV_WALL序号,ip,port,tvsscreen
   //;GROUP,SCREENID,IP,PORT,TVSSCREENID  //GroupScreen1=1,1,172.16.35.11,33445,1
	CString csTvWallIp, csTvWallPort, csScreenIndex, csSplitNum, csMode;
	m_TvWallIp.GetWindowText(csTvWallIp);
	m_TvWallPort.GetWindowText(csTvWallPort);

	csScreenIndex.Format(_T("%d"), m_ComBoxScreenIndex.GetCurSel()+1);
	csMode.Format(_T("%d"),m_ComBoxMode.GetCurSel());
	
	m_ScreenSplitNum.GetWindowText(csSplitNum);

	//GroupScreen1=1,1,172.16.35.11,33445,1
	CString csValue, csKey;

	csKey.Format(_T("GroupScreen%d_%d"), _ttoi(strSelGroupNum), _ttoi(csScreenIndex));
	csValue.Format(_T("%d,%d,%s,%d,%d"), _ttoi(strSelGroupNum), _ttoi(csScreenIndex), csTvWallIp, _ttoi(csTvWallPort), _ttoi(csScreenIndex));
	iniFile.WriteString(SGROUP, csKey, csValue);  //写tv_wall和屏幕对应信息


    CString csKeyEx, csValueEx, csIpcIdList;

	for (int i = 0; i < _ttoi(csSplitNum); i++)
	{
		CString str = m_videoWnd[i].m_csIpcPuid + _T("-");
		csIpcIdList += str;
	}
	csIpcIdList = csIpcIdList.Left(csIpcIdList.GetLength() - 1);

	csKeyEx.Format(_T("ScreenCamera%s_%s"), csScreenIndex, csMode);
	csValueEx.Format(_T("%s,%s,%s,%s,%s,%s"), strSelGroupNum, csScreenIndex, csMode, csSplitNum, _T("1"), csIpcIdList);
	iniFile.WriteString(SCREENCAMERA, csKeyEx, csValueEx);//写tv_wall和屏幕的分屏和上墙信息
	//CleanIpcScreenlist();
}

void GroupCfgSet::CleanIpcScreenlist()
{
	for (int i = 0; i < MAXWNDNUM; i++)
	{
		m_videoWnd[i].SetIpcPuid(_T("0"));
		m_videoWnd[i].SetOutText(_T("0.0.0.0"));
		m_videoWnd[i].Invalidate(TRUE);
	}
}

BOOL GroupCfgSet::CheckTvWallScreenIndex(int iTvWallIndex, int iScreenIndex, int & iTotleIndex)
{
	TvWallIndex2Screen::iterator ite = m_MapTvWallIndex2ScreenIndex.find(iTvWallIndex);
	if(ite != m_MapTvWallIndex2ScreenIndex.end())
	{
		list<int>::iterator it = find(ite->second.begin(), ite->second.end(), iScreenIndex);
		if (it != ite->second.end())
		{
			int mm = 12;
		}
	}
	//   m_MapTvWallIndex2ScreenIndex[_ttoi(csTvWallIndex)] = 1;
	//int mm = m_MapTvWallIndex2ScreenIndex[_ttoi(csTvWallIndex)];
	return TRUE;
}

void GroupCfgSet::OnCbnSelchangeGroupNum()
{
	// TODO: 在此添加控件通知处理程序代码
	RefrushIpc();

	CleanIpcZoom();

	m_cCheckFreeCut.SetCheck(FALSE);
	m_bCheckBol = FALSE;

	m_ComBoxScreenIndex.SetCurSel(0);
	m_ComBoxMode.SetCurSel(0);
	m_ScreenSplitNum.SetCurSel(0);
	m_ComBoxScreenIndex.SetCurSel(0);
	m_nTvwallScreen = m_ComBoxScreenIndex.GetCurSel()+1;
	m_nMode = m_ComBoxMode.GetCurSel();
	
	RenewSplitScreen(1,0);	//初始化分组默认屏幕分割状态
}

void GroupCfgSet::InitWind()
{
	for (int i = 0; i < 16; i++)
	{
		m_videoWnd[i].Create(CScreenDlg::IDD, this);
		m_videoWnd[i].SetWndIndex(i);
		m_videoWnd[i].SetParent(this);
	}

	CRect rect(0, 0, 0, 0);
	for (int j = 0; j < 4; ++j)
	{
		CStatic *pCtrl = new CStatic();
		pCtrl->Create(_T(""), WS_CHILD | SS_NOTIFY | SS_CENTERIMAGE, rect, this, N_SIDE_WND_ID);
		pCtrl->ShowWindow(TRUE);
		m_wndSelect[j] = pCtrl;
	}
	CRect rectGroup;
	GetDlgItem(IDC_STATIC_WIND)->GetWindowRect(&rectGroup);
	ScreenToClient(rectGroup);
	ArrayWindow(1, rectGroup);
}

void GroupCfgSet::OnCbnSelchangeTvwallScreenNum()
{
	// TODO: 在此添加控件通知处理程序代码
	int iScreenNum = m_ScreenSplitNum.GetItemData(m_ScreenSplitNum.GetCurSel());
	CRect rect;
	GetDlgItem(IDC_STATIC_WIND)->GetWindowRect(&rect);
	ScreenToClient(rect);
	ArrayWindow(iScreenNum, rect);
	CleanIpcScreenlist();
}

void GroupCfgSet::ArrayWindow(WORD iNumber, CRect Rect)
{
	//MFCAUTOLOCK(_vwLock);
	//InvalidWindow();
	int i = 0;
	int iWidth=Rect.Width();
	int iHeight=Rect.Height();

	for (i = 0;i < MAXWNDNUM;i++)
	{
		m_videoWnd[i].ShowWindow(SW_HIDE);
	}
	int nNull = 2;	
//iNumber = 2;
	switch(iNumber)
	{
	case 1:
		m_videoWnd[0].MoveWindow(SIDE_WIDTH + Rect.left, SIDE_WIDTH + Rect.top, iWidth - SIDE_WIDTH*2, iHeight - SIDE_WIDTH*2);
		m_videoWnd[0].ShowWindow(SW_SHOW);
		break;
	case 2:
		for(i = 0; i < 2; i++)
		{
			m_videoWnd[i].MoveWindow( Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*3) / 2) + i * nNull, Rect.top + SIDE_WIDTH+0, ((iWidth-SIDE_WIDTH*3) / 2), iHeight-SIDE_WIDTH*2);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		break;
	case 3:
		for(i = 0; i < 3; i++)
		{
			if (0 == i)
			{
				m_videoWnd[i].MoveWindow( Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*3) / 2) + i * nNull, Rect.top + SIDE_WIDTH+0, ((iWidth-SIDE_WIDTH*3) / 2), iHeight-SIDE_WIDTH*2);
				m_videoWnd[i].ShowWindow(SW_SHOW);
			}
			else if(1 == i)
			{
				m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*3) / 2) + i * nNull, Rect.top + SIDE_WIDTH+0, ((iWidth-SIDE_WIDTH*3) / 2), (iHeight-SIDE_WIDTH*3) / 2);
				m_videoWnd[i].ShowWindow(SW_SHOW);
			}
			else
			{
				int j = 3;
				m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + (j - 2) * ((iWidth-SIDE_WIDTH*3) / 2) + (j - 2) * nNull, Rect.top + (iHeight-SIDE_WIDTH*3) / 2 + 2*nNull, ((iWidth-SIDE_WIDTH*3) / 2), (iHeight-SIDE_WIDTH*3) / 2);
				m_videoWnd[i].ShowWindow(SW_SHOW);
			}
		}
		break;
	case 4:
		for(i = 0; i < 2; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*3) / 2) + i * nNull, Rect.top + SIDE_WIDTH+0, ((iWidth-SIDE_WIDTH*3) / 2), (iHeight-SIDE_WIDTH*3) / 2);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for(i = 2; i < 4; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + (i - 2) * ((iWidth-SIDE_WIDTH*3) / 2) + (i - 2) * nNull, Rect.top + (iHeight-SIDE_WIDTH*3) / 2 + 2*nNull, ((iWidth-SIDE_WIDTH*3) / 2), (iHeight-SIDE_WIDTH*3) / 2);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		break;
	case 6:
		{
			m_videoWnd[0].MoveWindow(Rect.left + SIDE_WIDTH, Rect.top + SIDE_WIDTH, 2*(iWidth-SIDE_WIDTH*4) / 3 + nNull, 2*(iHeight-SIDE_WIDTH*4) / 3 + nNull);
			m_videoWnd[0].ShowWindow(SW_SHOW);
			for (i = 1; i < 3; i++)
			{
				m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH*3 + 2*(iWidth-SIDE_WIDTH*4) / 3, Rect.top + (i-1)*(iHeight-SIDE_WIDTH*4)/3 + i*SIDE_WIDTH, ((iWidth-SIDE_WIDTH*4) / 3), (iHeight-SIDE_WIDTH*4) / 3);
				m_videoWnd[i].ShowWindow(SW_SHOW);
			}
			for (i = 3; i < 6; i++)
			{
				m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH*(i-2) + (i-3)*(iWidth-SIDE_WIDTH*4) / 3, Rect.top + 2*(iHeight-SIDE_WIDTH*4)/3 + 3*SIDE_WIDTH, ((iWidth-SIDE_WIDTH*4) / 3), (iHeight-SIDE_WIDTH*4) / 3);
				m_videoWnd[i].ShowWindow(SW_SHOW);
			}
		}
		break;
	case 8:
		m_videoWnd[0].MoveWindow(Rect.left + SIDE_WIDTH, Rect.top + SIDE_WIDTH, SIDE_WIDTH*2 + ((iWidth-SIDE_WIDTH*5)*3 / 4), SIDE_WIDTH*2 + (iHeight-SIDE_WIDTH*5)*3 / 4);
		m_videoWnd[0].ShowWindow(SW_SHOW);
		for(i = 1; i < 4; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + 4*SIDE_WIDTH +3*((iWidth-SIDE_WIDTH*5) / 4), Rect.top + SIDE_WIDTH*i + (i-1)*(iHeight-SIDE_WIDTH*5)/4, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for(i = 4; i < 8; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + (i - 3)*SIDE_WIDTH +(i - 4) * ((iWidth-SIDE_WIDTH*5) / 4), Rect.top + 3*(iHeight-SIDE_WIDTH*5)/4 + SIDE_WIDTH*4, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		break;
	case 9:
		for (i=0;i<3;i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*4) / 3) + i * nNull, Rect.top + SIDE_WIDTH+0, ((iWidth-SIDE_WIDTH*4) / 3), (iHeight-SIDE_WIDTH*4) / 3);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for (i=3;i<6;i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH +(i - 3) * ((iWidth-SIDE_WIDTH*4) / 3) + (i - 3) * nNull, Rect.top + (iHeight-SIDE_WIDTH*4) / 3 + 2*nNull, ((iWidth-SIDE_WIDTH*4) / 3), (iHeight-SIDE_WIDTH*4) / 3);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for (i=6;i<9;i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + (i - 6) * ((iWidth-SIDE_WIDTH*4) / 3) + (i - 6) * nNull, Rect.top + 2*(iHeight-SIDE_WIDTH*4) / 3 + 3*nNull, ((iWidth-SIDE_WIDTH*4) / 3), (iHeight-SIDE_WIDTH*4) / 3);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		break;
	case 16:
		for(i = 0; i < 4; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH + i * ((iWidth-SIDE_WIDTH*5) / 4) + (i) * nNull, Rect.top + SIDE_WIDTH, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for(i = 4; i < 8; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH +(i - 4) * ((iWidth-SIDE_WIDTH*5) / 4) + (i - 4) * nNull, Rect.top + (iHeight-SIDE_WIDTH*5) / 4 + 2*nNull, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for(i = 8; i < 12; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH +(i - 8) * ((iWidth-SIDE_WIDTH*5) / 4) + (i - 8) * nNull, Rect.top + 2*(iHeight-SIDE_WIDTH*5) / 4 + 3*nNull, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		for(i = 12; i < 16; i++)
		{
			m_videoWnd[i].MoveWindow(Rect.left + SIDE_WIDTH +(i - 12) * ((iWidth-SIDE_WIDTH*5) / 4) + (i - 12) * nNull, Rect.top + 3 * (iHeight-SIDE_WIDTH*5) / 4 + 4 * nNull, ((iWidth-SIDE_WIDTH*5) / 4), (iHeight-SIDE_WIDTH*5) / 4);
			m_videoWnd[i].ShowWindow(SW_SHOW);
		}
		break;
	default:
		break;
	}
	m_nCurIndex = m_nCurIndex >= iNumber ? 0 : m_nCurIndex;
	//m_nScreenTotle = iNumber;
	DrawActivePage(TRUE);
}

void GroupCfgSet::DrawActivePage(BOOL bActive)
{
	if (bActive)
	{
		CRect rc;
		m_videoWnd[m_nCurIndex].GetWindowRect(&rc);
		ScreenToClient(&rc);

		m_wndSelect[0]->MoveWindow(rc.left - SIDE_WIDTH, rc.top - SIDE_WIDTH, rc.Width() + 2 * SIDE_WIDTH, SIDE_WIDTH);
		m_wndSelect[1]->MoveWindow(rc.left - SIDE_WIDTH, rc.bottom, rc.Width() + 2 * SIDE_WIDTH, SIDE_WIDTH);
		m_wndSelect[2]->MoveWindow(rc.left - SIDE_WIDTH, rc.top - SIDE_WIDTH, SIDE_WIDTH, rc.Height() + 2 * SIDE_WIDTH);
		m_wndSelect[3]->MoveWindow(rc.right, rc.top - SIDE_WIDTH, SIDE_WIDTH, rc.Height() + 2 * SIDE_WIDTH);
		for (int i = 0; i < 4; ++i)
		{
			m_wndSelect[i]->Invalidate();
		}
	}
}

HBRUSH GroupCfgSet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC:
		{
			int id = pWnd->GetDlgCtrlID();
			if(id == N_SIDE_WND_ID)
			{
				hbr = (HBRUSH)m_brushSel.GetSafeHandle();
			}
		}
		break;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void GroupCfgSet::OnNMDblclkListIpcZoom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//m_Devlist.GetCheck(i)
	POSITION p=m_IpcListZoom.GetFirstSelectedItemPosition(); 
	if (p != NULL) 
	{ 
		//获取刚选取的位置的下标(从0开始的)
		int index = m_IpcListZoom.GetNextSelectedItem(p);
		DeviceInfoZoom *pItemInfo = (DeviceInfoZoom*)m_IpcListZoom.GetItemData(index);

		ZoomCfgDlg ZoomCfgDlg;

		ZoomCfgDlg.m_csIp = pItemInfo->csIp;
		ZoomCfgDlg.m_csPuid = pItemInfo->csIpcPuid;
		ZoomCfgDlg.m_csDefaultZoom = _T("1000");

		ZoomCfgDlg.DoModal();

        pItemInfo->csZoomCfg = ZoomCfgDlg.m_csIpcZoomCfg;
		ZoomCfgDlg.UnInitData();
		//m_videoWnd[m_nCurIndex].SetOutText(pItemInfo->csDeviceIp);
		//m_videoWnd[m_nCurIndex].SetIpcPuid(pItemInfo->csDeviceId);
		//m_videoWnd[m_nCurIndex].Invalidate(TRUE);
		////移动选中的焦点
		//m_nCurIndex++;
		//int iSplitNum = m_ScreenSplitNum.GetItemData(m_ScreenSplitNum.GetCurSel());
		//m_nCurIndex = m_nCurIndex >= iSplitNum ? 0 : m_nCurIndex;
		//DrawActivePage(TRUE);
	} 
	*pResult = 0;
}

void GroupCfgSet::OnNMDblclkListIpcGroup(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//m_Devlist.GetCheck(i)
	POSITION p=m_Ipclist.GetFirstSelectedItemPosition(); 
	if (p != NULL) 
	{ 
		//获取刚选取的位置的下标(从0开始的)
		int index = m_Ipclist.GetNextSelectedItem(p);
		m_Ipclist.SetCheck(index, TRUE);
		DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Ipclist.GetItemData(index);
		m_videoWnd[m_nCurIndex].SetOutText(pItemInfo->csDeviceIp);
		m_videoWnd[m_nCurIndex].SetIpcPuid(pItemInfo->csDeviceId);
		m_videoWnd[m_nCurIndex].Invalidate(TRUE);
		//移动选中的焦点
		m_nCurIndex++;
		int iSplitNum = m_ScreenSplitNum.GetItemData(m_ScreenSplitNum.GetCurSel());
		m_nCurIndex = m_nCurIndex >= iSplitNum ? 0 : m_nCurIndex;
		DrawActivePage(TRUE);
	} 
	*pResult = 0;
}

void GroupCfgSet::OnBnClickedBtnAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	for(int i=0; i<m_Ipclist.GetItemCount(); i++)
	{
		if( m_Ipclist.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED )
		{
			DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Ipclist.GetItemData(i);
			int iCom = 0;
			int iRow = m_IpcListZoom.GetItemCount();
			int nItem = m_IpcListZoom.InsertItem(iRow, _T(""));
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csDeviceId);
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csDeviceName);
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csDeviceIp);
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csDevicePort);
			if (_T("2") == pItemInfo->csComponyType)
			{
				m_Ipclist.SetItemText(iRow, iCom++, _T("Axis"));
			}
			else if (_T("4") == pItemInfo->csComponyType)
			{
				m_Ipclist.SetItemText(iRow, iCom++, _T("HIK"));
			}
			else
			{
				m_Ipclist.SetItemText(iRow, iCom++, _T("DEV 2.0"));
			}
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csUserName);
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csUserPwd);
			m_IpcListZoom.SetItemText(iRow, iCom++, pItemInfo->csDeviceType);

			DeviceInfoZoom *pIpcZoomItem = new DeviceInfoZoom();
			pIpcZoomItem->csIpcPuid = pItemInfo->csDeviceId;
			pIpcZoomItem->csIp = pItemInfo->csDeviceIp;
			m_IpcListZoom.SetItemData(nItem, (DWORD)pIpcZoomItem);
		}
	}
}

void GroupCfgSet::OnBnClickedBtnSetZoom()
{
	// TODO: 在此添加控件通知处理程序代码
	//CString strSelGroupNum;
	//m_ComBoxGroup.GetWindowText(strSelGroupNum);
	CString strSelGroupNum;
	strSelGroupNum.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);
	CString csIniFileName = m_csIniPath + _T("Group_") + strSelGroupNum + _T(".ini");
	CIniFile iniFile(csIniFileName);

	//清除以前的值
	iniFile.EraseSection(CAMERAZOOMHEIGHT);
	iniFile.EraseSection(CAMERAZOOM);

	CString csPuidTemp, csCameraKey, csCameraValue;
	for(int i=0; i<m_IpcListZoom.GetItemCount(); i++)
	{
		CString csZoomCameraKey, csZoomCameraValue;

		DeviceInfoZoom *pItemInfo = (DeviceInfoZoom*)m_IpcListZoom.GetItemData(i);
			
		if (pItemInfo->csZoomCfg.GetLength() == 0)	//没有设置ZOOM值，使用默认 
		{
			csZoomCameraKey.Format(_T("ZoomCamera%d_%d"),_ttoi(strSelGroupNum), i);	
			csZoomCameraValue.Format(_T("%s,1000,"),pItemInfo->csIpcPuid);
			csZoomCameraValue+=_T("20-0-5-1200,20-5-13-3000,");
			csZoomCameraValue+=_T("20-13-19-4100,20-19-25-5500,20-25-32-6100,30-0-5-1200,30-5-13-2800,");
			csZoomCameraValue+=_T("30-13-19-4600,30-19-25-5100,30-25-32-6300,40-0-5-2300,40-5-13-3200,");
			csZoomCameraValue+=_T("40-13-19-4200,40-19-25-5000,40-25-32-6300,45-0-5-2400,45-5-13-3100,");
			csZoomCameraValue+=_T("45-13-19-4600,45-19-25-5700,45-25-32-6100");
		}
		else
		{
			csZoomCameraKey.Format(_T("ZoomCamera%d_%d"),_ttoi(strSelGroupNum), i);
			csZoomCameraValue = pItemInfo->csZoomCfg;

		}
		iniFile.WriteString(CAMERAZOOMHEIGHT, csZoomCameraKey, csZoomCameraValue);
		csPuidTemp += (pItemInfo->csIpcPuid + _T("-"));
	}
	if (m_IpcListZoom.GetItemCount() > 0)
	{
		csCameraKey.Format(_T("zoom%s"), strSelGroupNum);
		csPuidTemp = csPuidTemp.Left(csPuidTemp.GetLength() - 1);
		csCameraValue.Format(_T("%s,%s"), strSelGroupNum, csPuidTemp);
		iniFile.WriteString(CAMERAZOOM, csCameraKey, csCameraValue);
	}
}

void GroupCfgSet::OnCbnSelchangeTvwallTotleScreen()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cCheckFreeCut.SetCheck(FALSE);
	m_bCheckBol = FALSE;

	m_nTvwallScreen = m_ComBoxScreenIndex.GetCurSel()+1;
	
	RenewSplitScreen(m_nTvwallScreen, m_nMode);
}

void GroupCfgSet::OnCbnSelchangeTvwallMode()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nMode = m_ComBoxMode.GetCurSel();

	RenewSplitScreen(m_nTvwallScreen, m_nMode);
}
void GroupCfgSet::RenewSplitScreen(int nTvwallScreen, int nMode)
{
	BOOL bol = FALSE;
	CString csGroupNum;
	csGroupNum.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);
	CString csFileName = m_csIniPath+_T("Group_")+csGroupNum+_T(".ini");
	CIniFile infile(csFileName);

	CStringArray GroupTvwall;
	infile.ReadSectionString(SGROUP, GroupTvwall);		//改变TVWALL IP
	for (int i = 0; i < GroupTvwall.GetSize(); i++)	
	{
		std::vector<CString> arry;
		arry = Split(GroupTvwall[i]);

		int nGroup,nTvScreen;

		nGroup = _ttoi(arry[0])&0xff;
		nTvScreen = _ttoi(arry[1])&0xff;
		if (nGroup == _ttoi(csGroupNum) && nTvScreen == nTvwallScreen)
		{
			bol = TRUE;
			CString strIP = arry[2];
			m_TvWallIp.SetWindowText(strIP);
		}
	}
	if (!bol)
	{
		//m_TvWallIp.SetWindowText(_T("192.168.0.0"));
	}
	bol = FALSE;

	CStringArray GroupArray;
	infile.ReadSectionString(_T("SCREENCAMERA"), GroupArray);		//改变屏幕分割状态和IPC信息
	for (int i = 0; i < GroupArray.GetSize(); i++)
	{
		std::vector<CString> arry;
		arry = Split(GroupArray[i]);
		int nGroup, nTvGroupScreen, nTvMode;
		nTvMode= _ttoi(arry[2])&0xff;
		nTvGroupScreen = _ttoi(arry[1])&0xff;
		nGroup = _ttoi(arry[0])&0xff;
		int nGroupNum = _ttoi(csGroupNum);
		int iSplitNum = _ttoi(arry[3])&0xff;

		if (nTvMode == nMode && nTvGroupScreen == nTvwallScreen && nGroup == nGroupNum)
		{
			bol = TRUE;
			CRect rect;
			GetDlgItem(IDC_STATIC_WIND)->GetWindowRect(&rect);
			ScreenToClient(rect);
			ArrayWindow(iSplitNum, rect);

			if (1 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(0);
			}
			else if (2 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(1);
			}
			else if (4 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(2);
			}
			else if (6 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(3);
			}
			else if (8 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(4);
			}
			else if (9 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(5);
			}
			else if (16 == iSplitNum)
			{
				m_ScreenSplitNum.SetCurSel(6);
			}
			std::vector<CString> sIpcList;
			sIpcList = Split(arry[5], _T("-"));
			int nsize = sIpcList.size();
			for (int i = 0; i < nsize; i++)
			{
				CString strPUID = sIpcList[i];
				CString strIP = FindIPC(strPUID);
				if (strIP != _T(""))
				{
					m_videoWnd[i].SetOutText(strIP);
					m_videoWnd[i].SetIpcPuid(strPUID);
					m_videoWnd[i].Invalidate(TRUE);
				}
				else
				{
					m_videoWnd[i].SetOutText(_T("0.0.0.0"));
					m_videoWnd[i].SetIpcPuid(_T("0"));
					m_videoWnd[i].Invalidate(TRUE);
				}
			}

		}
	}
	if (!bol)		//如果配置文件没有关于此组的配置，还原为一窗口
	{
		CRect rectGroup;
		GetDlgItem(IDC_STATIC_WIND)->GetWindowRect(&rectGroup);
		ScreenToClient(rectGroup);
		ArrayWindow(1, rectGroup);
		m_ScreenSplitNum.SetCurSel(0);
		CleanIpcScreenlist();
	}
}
std::vector<CString> GroupCfgSet::Split(CString string, CString separator)
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
CString GroupCfgSet::FindIPC(CString strPUID)
{
	CString csGroupNum;
	csGroupNum.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);
	CString csFileName = m_csIniPath+_T("Group_")+csGroupNum+_T(".ini");
	CIniFile infile(csFileName);

	CStringArray GroupArray;
	infile.ReadSectionString(_T("CARAMPTZ"), GroupArray);
	for (int i = 0; i < GroupArray.GetSize(); i++)
	{
		std::vector<CString> sIpcList;
		sIpcList = Split(GroupArray[i]);
		CString sIPCPuid = sIpcList[2];
		if (sIPCPuid == strPUID)
		{
			return sIpcList[3];
		}
	}
	return _T("");
}
void GroupCfgSet::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nItem =m_IpcListZoom.GetSelectionMark();
	m_IpcListZoom.DeleteItem(nItem);
}

void GroupCfgSet::OnBnClickedCheckFreecut()
{
	// TODO: 在此添加控件通知处理程序代码
	if (BST_CHECKED ==  m_cCheckFreeCut.GetCheck())
	{
		m_bCheckBol = TRUE;
	}
	else if(BST_UNCHECKED == m_cCheckFreeCut.GetCheck())
	{
		m_bCheckBol = FALSE;
	}
}

void GroupCfgSet::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strSelGroupNum,csScreenIndex;

	strSelGroupNum.Format(_T("%d"), m_ComBoxGroup.GetCurSel()+1);
	csScreenIndex.Format(_T("%d"), m_ComBoxScreenIndex.GetCurSel()+1);
	CString csIniFileName = m_csIniPath + _T("Group_") + strSelGroupNum + _T(".ini");
	CIniFile iniFile(csIniFileName);

	if (m_bCheckBol)
	{
		int addr = 176;
		CString csKeyFree, csValueFree;
		for (int i = 1; i < 5; i++)
		{
			csKeyFree.Format(_T("Windows%d"), i);
			csValueFree.Format(_T("%s,%s,%d,%d"),strSelGroupNum, csScreenIndex, i, addr);
			iniFile.WriteString(FREECUTSCREEN, csKeyFree, csValueFree);//写tv_wall和屏幕的分屏和上墙信息
			addr += 2;
		}
		m_cCheckFreeCut.SetCheck(FALSE);
		m_bCheckBol = FALSE;
	}
}
