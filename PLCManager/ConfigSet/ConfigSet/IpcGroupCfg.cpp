// IpcGroupCfg.cpp : 实现文件

#include "stdafx.h"
#include "ConfigSet.h"
#include "IpcGroupCfg.h"
#include "ConfigSetDlg.h"

// IpcGroupCfg 对话框

IMPLEMENT_DYNAMIC(IpcGroupCfg, CDialog)

IpcGroupCfg::IpcGroupCfg(CWnd* pParent /*=NULL*/)
	: CDialog(IpcGroupCfg::IDD, pParent)
	,m_csIniPath(_T(""))
	,m_csDeviceFile(_T(""))
	,m_pMainCfgDlg(NULL)
{

}

IpcGroupCfg::~IpcGroupCfg()
{
}

void IpcGroupCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IPC, m_Devlist);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_GroupNum);
	DDX_Control(pDX, IDC_TREE_IPC_GROUP, m_GroupTree);
	DDX_Control(pDX, IDC_CHECK1, m_cCheckBox);
	DDX_Control(pDX, IDC_BTN_SET, m_btnset);
	DDX_Control(pDX, IDC_BTN_DEL, m_cBtnDel);
}


BEGIN_MESSAGE_MAP(IpcGroupCfg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SELECT, &IpcGroupCfg::OnBnClickedBtnSelect)
	ON_BN_CLICKED(IDC_BTN_SET, &IpcGroupCfg::OnBnClickedBtnSet)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_IPC, &IpcGroupCfg::OnNMDblclkListIpc)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &IpcGroupCfg::OnHdnItemclickListIpc)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_IPC, &IpcGroupCfg::OnLvnItemchangedListIpc)
	ON_BN_CLICKED(IDC_CHECK1, &IpcGroupCfg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BTN_DEL, &IpcGroupCfg::OnBnClickedBtnDel)
	ON_NOTIFY(NM_CUSTOMDRAW,IDC_LIST_IPC, OnCustomdrawMyList)
END_MESSAGE_MAP()


// IpcGroupCfg 消息处理程序
BOOL IpcGroupCfg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HBITMAP hBitMap;
	hBitMap = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ADD));
	((CButton*)GetDlgItem(IDC_BTN_SET))->SetBitmap(hBitMap);
	HBITMAP hBitMapDel;
	hBitMapDel = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_DEL));
	((CButton*)GetDlgItem(IDC_BTN_DEL))->SetBitmap(hBitMapDel);

	DWORD dwExtendedStyle = m_Devlist.GetExtendedStyle();
	dwExtendedStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES;

	m_Devlist.SetExtendedStyle( dwExtendedStyle);

	int nIndex = 0;
	m_Devlist.InsertColumn(nIndex++, _T("device id"), LVCFMT_LEFT,120);
	m_Devlist.InsertColumn(nIndex++, _T("device name"), LVCFMT_LEFT,130);
	m_Devlist.InsertColumn(nIndex++, _T("ip"), LVCFMT_LEFT, 110);
	m_Devlist.InsertColumn(nIndex++, _T("port"), LVCFMT_LEFT, 50);
	m_Devlist.InsertColumn(nIndex++, _T("company"), LVCFMT_LEFT, 60);
	m_Devlist.InsertColumn(nIndex++, _T("user"), LVCFMT_LEFT, 60);
	m_Devlist.InsertColumn(nIndex++, _T("password"), LVCFMT_LEFT, 70);
	m_Devlist.InsertColumn(nIndex++, _T("device type"), LVCFMT_LEFT, 60);

	m_pHeadCtrl =m_Devlist.GetHeaderCtrl();
	VERIFY(m_cImageList.Create(IDB_CHECKBOX, 13, 4, RGB(255,255,255)));

	int i = m_cImageList.GetImageCount();

	m_pHeadCtrl->SetImageList(&m_cImageList); 

	HDITEM hdItem;

	hdItem.mask= HDI_IMAGE | HDI_FORMAT;

	VERIFY(m_pHeadCtrl->GetItem(0, &hdItem) );

	hdItem.iImage= 1;
	hdItem.fmt|= HDF_IMAGE;
	
	VERIFY(  m_pHeadCtrl->SetItem(0, &hdItem) );
	
	for (int i = 1; i <= 16; i ++)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_GroupNum.AddString(str);
	}
	m_GroupNum.SetCurSel(0);
	return TRUE;
}

void IpcGroupCfg::OnBnClickedBtnSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	CleanDeviceList();
	m_mDeviceMap.clear();
	m_mSameDeviceID.clear();
	m_mSameDeviceName.clear();

	CString strFmt;
	if (strFmt.GetLength() == 0)
	{
		strFmt = "All Files (*.*)|*.*||";
	}
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, strFmt, NULL);
	dlgFile.m_ofn.lpstrInitialDir = m_csOpenPath;
	if (dlgFile.DoModal())
	{
		m_csDeviceFile = dlgFile.GetPathName();
	}
	if(!m_csDeviceFile.IsEmpty())
	{
		AddDeviceListFromExls();
	}

}

void IpcGroupCfg::AddDeviceListFromExls()
{
	//int nItem = m_Devlist.InsertItem(0,_T("qqqqqqqqqqq"));
	CStdioFile   sfile; 
	BasicExcel e;
	char buf[200] = {0};
	int k;
	BasicExcelWorksheet* sheet;
	
	std::string xlsFileName;
	CStrCode::WStrToMStr(m_csDeviceFile, xlsFileName);
	
	strncpy(buf, xlsFileName.c_str(), xlsFileName.length());	
	
	e.Load(buf);
	sheet = e.GetWorksheet("devices");
	if (sheet)
	{	
		USES_CONVERSION;  
		int iRows = sheet->GetTotalRows();
		int iCols = sheet->GetTotalCols();

		CString csExlType(sheet->Cell(0, 0)->GetString());
		if (csExlType == _T("deviceid"))		//dag导出
		{
			for (int i = 1; i < iRows; i++)
			{
				CString csRow;
				DeviceItemInfo *pItemInfo = new DeviceItemInfo();

				int nItem = m_Devlist.InsertItem(i - 1,_T(""));
				for (int j = 0; j< /*iCols*/8; j++)
				{
					CString csItem(sheet->Cell(i, j)->GetString());
					if (j == 0)
					{
						pItemInfo->csDeviceId = csItem;
					}
					else if (j == 1)
					{
						pItemInfo->csDeviceName = csItem;
					}
					else if (j == 2)
					{
						pItemInfo->csDeviceIp = csItem;
					}
					if (3 == j)//port
					{

						csItem.Format(_T("%d"), sheet->Cell(i, j)->GetInteger());
						pItemInfo->csDevicePort = csItem;
					}
					else if (j == 4)
					{
						//pItemInfo->csComponyType = csItem;		//由于TVWALL 需要用此字段，所以此字段改回原来
						if(_T("Axis") == csItem)
						{
							pItemInfo->csComponyType = "2";
						}
						else if (_T("HIK") == csItem)
						{
							pItemInfo->csComponyType = "4";	//HK
						}
						else
						{
							pItemInfo->csComponyType = "9";	//AP
						}
					}
					else if (j == 5)
					{
						pItemInfo->csUserName = csItem;
					}
					else if (j == 6)
					{
						pItemInfo->csUserPwd = csItem;
					}
					else if (j == 7)
					{
						pItemInfo->csDeviceType = csItem;
					}

					m_Devlist.SetItemText(i - 1, j, csItem);
				}
				m_Devlist.SetItemData(nItem, (DWORD)pItemInfo);
				
				if( i > 1)
				{
					int nbl = IsSameDevice(pItemInfo->csDeviceId, pItemInfo->csDeviceName , 1);
					if(-1 != nbl)
					{
						std::map<int , CString>::iterator it = m_mSameDeviceID.find(nbl);
						if (it != m_mSameDeviceID.end())
						{
							it->second = pItemInfo->csDeviceId;
						}
						else
						{
							m_mSameDeviceID.insert(std::make_pair(nbl, pItemInfo->csDeviceId));
						}

						m_mSameDeviceID.insert(std::make_pair(i-1, pItemInfo->csDeviceId));
					}
					int nb = IsSameDevice(pItemInfo->csDeviceId, pItemInfo->csDeviceName , 2);
					if(-1 != nb)
					{
						std::map<int , CString>::iterator it = m_mSameDeviceName.find(nb);
						if (it != m_mSameDeviceName.end())
						{
							it->second = pItemInfo->csDeviceName;
						}
						else
						{
							m_mSameDeviceName.insert(std::make_pair(nb, pItemInfo->csDeviceName));
						}
						m_mSameDeviceName.insert(std::make_pair(i-1, pItemInfo->csDeviceName));
					}
				}
				DEVICEINFO Info;
				Info.csDeviceId = pItemInfo->csDeviceId;
				Info.csDeviceName = pItemInfo->csDeviceName;
				m_mDeviceMap.insert(std::make_pair(i-1, Info));
			}
		}
		else	//300导出
		{
			for (int i = 1; i < iRows; i++)
			{
				CString csRow;
				CString csCompany;
				DeviceItemInfo *pItemInfo = new DeviceItemInfo();

				int nItem = m_Devlist.InsertItem(i - 1,_T(""));
				for (int j = 0; j< /*iCols*/14; j++)
				{
					CString csItem(sheet->Cell(i, j)->GetString());
					if (j == 13)
					{
						pItemInfo->csDeviceId = csItem;
					}
					else if (j == 0)
					{
						pItemInfo->csDeviceName = csItem;
					}
					else if (j == 3)
					{
						pItemInfo->csDeviceIp = csItem;
					}
					if (4 == j)//port
					{

						csItem.Format(_T("%d"), sheet->Cell(i, j)->GetInteger());
						pItemInfo->csDevicePort = csItem;
					}
					else if (j == 12)
					{
						//pItemInfo->csComponyType = csItem;		//由于TVWALL 需要用此字段，所以此字段改回原来
						if(_T("Axis") == csItem)
						{
							pItemInfo->csComponyType = "2";
							csCompany = _T("Axis");
						}
						else if (_T("HIK") == csItem)
						{
							pItemInfo->csComponyType = "4";	//HK
							csCompany = _T("HIK");
						}
						else
						{
							pItemInfo->csComponyType = "9";	//AP
							csCompany = _T("DEV 2.0");
						}
					}
					else if (j == 5)
					{
						pItemInfo->csUserName = csItem;
					}
					else if (j == 6)
					{
						pItemInfo->csUserPwd = csItem;
					}
					else if (j == 1)
					{
						pItemInfo->csDeviceType = csItem;
					}

					//m_Devlist.SetItemText(i - 1, j, csItem);
				}
				m_Devlist.SetItemText(i-1, 0, pItemInfo->csDeviceId);
				m_Devlist.SetItemText(i-1, 1, pItemInfo->csDeviceName);
				m_Devlist.SetItemText(i-1, 2, pItemInfo->csDeviceIp);
				m_Devlist.SetItemText(i-1, 3, pItemInfo->csDevicePort);
				m_Devlist.SetItemText(i-1, 4, csCompany);
				m_Devlist.SetItemText(i-1, 5, pItemInfo->csUserName);
				m_Devlist.SetItemText(i-1, 6, pItemInfo->csUserPwd);
				m_Devlist.SetItemText(i-1, 7, pItemInfo->csDeviceType);

				m_Devlist.SetItemData(nItem, (DWORD)pItemInfo);

				if( i > 1)
				{
					int nbl = IsSameDevice(pItemInfo->csDeviceId, pItemInfo->csDeviceName , 1);
					if(-1 != nbl)
					{
						std::map<int , CString>::iterator it = m_mSameDeviceID.find(nbl);
						if (it != m_mSameDeviceID.end())
						{
							it->second = pItemInfo->csDeviceId;
						}
						else
						{
							m_mSameDeviceID.insert(std::make_pair(nbl, pItemInfo->csDeviceId));
						}

						m_mSameDeviceID.insert(std::make_pair(i-1, pItemInfo->csDeviceId));
					}
					int nb = IsSameDevice(pItemInfo->csDeviceId, pItemInfo->csDeviceName , 2);
					if(-1 != nb)
					{
						std::map<int , CString>::iterator it = m_mSameDeviceName.find(nb);
						if (it != m_mSameDeviceName.end())
						{
							it->second = pItemInfo->csDeviceName;
						}
						else
						{
							m_mSameDeviceName.insert(std::make_pair(nb, pItemInfo->csDeviceName));
						}
						m_mSameDeviceName.insert(std::make_pair(i-1, pItemInfo->csDeviceName));
					}
				}
				DEVICEINFO Info;
				Info.csDeviceId = pItemInfo->csDeviceId;
				Info.csDeviceName = pItemInfo->csDeviceName;
				m_mDeviceMap.insert(std::make_pair(i-1, Info));
			}
		}


    }
}

int IpcGroupCfg::IsSameDevice(CString csDeviceId, CString csDeviceName , int num)
{
	if (0 == num)
	{
		for (std::map<int ,DEVICEINFO>::iterator ite = m_mDeviceMap.begin(); ite != m_mDeviceMap.end(); ite++)
		{
			if (ite->second.csDeviceId == csDeviceId || ite->second.csDeviceName == csDeviceName)
			{
				return ite->first;
			}
		}
	}
	else if (1 == num )
	{
		for (std::map<int ,DEVICEINFO>::iterator ite = m_mDeviceMap.begin(); ite != m_mDeviceMap.end(); ite++)
		{
			if (ite->second.csDeviceId == csDeviceId)
			{
				return ite->first;
			}
		}
	}
	else if (2 == num)
	{
		for (std::map<int ,DEVICEINFO>::iterator ite = m_mDeviceMap.begin(); ite != m_mDeviceMap.end(); ite++)
		{
			if (ite->second.csDeviceName == csDeviceName)
			{
				return ite->first;
			}
		}
	}
	
	return -1;
}
void IpcGroupCfg::OnBnClickedBtnSet()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strSelGroupNum;
	m_GroupNum.GetWindowText(strSelGroupNum);
	if (!PathFileExists(m_csIniPath))
	{
		if(!CreateDirectory(m_csIniPath, NULL))
		{
			MessageBox(_T("创建文件夹失败!"));
		}
	}
	IpcGroupMap ::iterator ite = m_pMainCfgDlg->m_IpcGroupMap.find(_ttoi(strSelGroupNum));
	if (ite != m_pMainCfgDlg->m_IpcGroupMap.end())
	{
		ite->second.clear();		//清楚指定组的IPC信息
	}

	CString csIniFileName = m_csIniPath + _T("Group_") + strSelGroupNum + _T(".ini");
	CIniFile iniFile(csIniFileName);
	iniFile.WriteString(GROUPID, ID, strSelGroupNum);

	HTREEITEM htrCurrent;
	HTREEITEM hItem = m_GroupTree.GetChildItem(TVI_ROOT);
	while (hItem) 
	{
		CString strText= m_GroupTree.GetItemText(hItem);
		if( _T("Group_") + strSelGroupNum == strText)
		{
			htrCurrent = hItem;
			break;
		}
		hItem = m_GroupTree.GetNextSiblingItem(hItem); 
	}
	if (NULL == hItem)
	{
		htrCurrent = m_GroupTree.InsertItem(_T("Group_") + strSelGroupNum, 0, 0, TVI_ROOT);
	}

	HTREEITEM er = m_GroupTree.GetChildItem(htrCurrent);
	while(er)
	{
		HTREEITEM Temp = er; 
		er = m_GroupTree.GetNextSiblingItem(er);
		m_GroupTree.DeleteItem(Temp);
	}
	iniFile.EraseSection(CARAMPTZ);		//清除CARAMPTZ，重新写

	int iRowCount = m_Devlist.GetItemCount();
	int iIndex = 0;
	IpcList IpcListItem;
	for(int i=0; i < iRowCount; i++)
	{
		CString csRowInfo;
		DeviceItemInfo ItemInfo;
		if(/* m_Devlist.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || */m_Devlist.GetCheck(i))
		{
			CString csKey;
			csKey.Format(_T("group%d"), ++iIndex);
			DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(i);
			CString csValue, csTreeValue;
			csValue.Format(_T("%s,%d,%s,%s,%s,%s,%s,%s,%s,%d"), strSelGroupNum, iIndex, pItemInfo->csDeviceId,
					pItemInfo->csDeviceIp, pItemInfo->csDevicePort,
					pItemInfo->csUserName, pItemInfo->csUserPwd, 
					pItemInfo->csComponyType, pItemInfo->csDeviceName, pItemInfo->iStreamType);
			
			iniFile.WriteString(CARAMPTZ, csKey, csValue);
			if ("2" == pItemInfo->csComponyType)
			{
				csTreeValue.Format(_T("%s, %s, Axis"), pItemInfo->csDeviceName, pItemInfo->csDeviceIp);
			}
			else if ("4" == pItemInfo->csComponyType)
			{
				csTreeValue.Format(_T("%s, %s, HIK"), pItemInfo->csDeviceName, pItemInfo->csDeviceIp);
			}
			else
			{
				csTreeValue.Format(_T("%s, %s, DEV 2.0"), pItemInfo->csDeviceName, pItemInfo->csDeviceIp);
			}
			ItemInfo = *pItemInfo;
			//delete pItemInfo;
			m_Devlist.SetCheck(i, FALSE);
			IpcListItem.push_back(ItemInfo);
			m_GroupTree.InsertItem(csTreeValue, 0, 0, htrCurrent);
			m_GroupTree.Expand(htrCurrent, TVE_EXPAND);
		}
	}
	//m_pMainCfgDlg->m_IpcGroupMap.insert(std::make_pair(_ttoi(strSelGroupNum), IpcListItem));
	m_pMainCfgDlg->m_IpcGroupMap[_ttoi(strSelGroupNum)] = IpcListItem;
}

void IpcGroupCfg::CleanDeviceList()
{
	int iRowCount = m_Devlist.GetItemCount();
	if (iRowCount > 0)
	{
		for(int i=0; i < iRowCount; i++)
		{
			DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(i);
			delete pItemInfo;
		}
		m_Devlist.DeleteAllItems();
	}
}

void IpcGroupCfg::OnNMDblclkListIpc(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POSITION p=m_Devlist.GetFirstSelectedItemPosition(); 
	if (p != NULL) 
	{ 
		//获取刚选取的位置的下标(从0开始的)
		int index = m_Devlist.GetNextSelectedItem(p);
		CString str;

		DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(index);

		CstreamSetDlg StreamTypeDlg;
		CString csDeviceID, csDeviceName, csIp, csPort, csCompany, csUser, csPassWord, csDeviceType;
		int iCompany;

		csDeviceID = StreamTypeDlg.m_csDeviceID = pItemInfo->csDeviceId;
	 	csDeviceName = StreamTypeDlg.m_csDeviceName = pItemInfo->csDeviceName;
	    csIp = StreamTypeDlg.m_csIp = pItemInfo->csDeviceIp;
		csPort = StreamTypeDlg.m_csPort = pItemInfo->csDevicePort;
		csCompany = StreamTypeDlg.m_csCompany = pItemInfo->csComponyType;
		csUser = StreamTypeDlg.m_csUser = pItemInfo->csUserName;
		csPassWord = StreamTypeDlg.m_csPassWord = pItemInfo->csUserPwd;
		csDeviceType = StreamTypeDlg.m_csDeviceType = pItemInfo->csDeviceType;

		StreamTypeDlg.DoModal();

		if (csDeviceID != StreamTypeDlg.m_csDeviceID)
		{
			if (-1 != IsSameDevice(StreamTypeDlg.m_csDeviceID, StreamTypeDlg.m_csDeviceName, 1))
			{
				MessageBox(_T("Device id不能相同"));
				return;
			}
			else
			{
				int num = 0;
				CString csName;

				std::map<int , CString>::iterator ite = m_mSameDeviceID.find(index);
				for (std::map<int , CString>::iterator it = m_mSameDeviceID.begin(); it != m_mSameDeviceID.end(); it++)
				{
					if (it->second == ite->second)
					{
						num++;
						csName = ite->second;
					}
				}
				if (ite != m_mSameDeviceID.end())
				{
					m_mSameDeviceID.erase(ite);
				}

				if (2 == num)
				{
					for (std::map<int , CString>::iterator ite = m_mSameDeviceID.begin(); ite != m_mSameDeviceID.end(); ite++)
					{
						if (ite->second == csName)
						{
							m_mSameDeviceID.erase(ite);
							break;
						}
					}

				}
			}
		}
		if (csDeviceName != StreamTypeDlg.m_csDeviceName)
		{
			if (-1 != IsSameDevice(StreamTypeDlg.m_csDeviceID, StreamTypeDlg.m_csDeviceName, 2))
			{
				MessageBox(_T("Device name不能相同"));
				return;
			}
			else
			{
				int num = 0;
				CString csName;

				std::map<int , CString>::iterator ite = m_mSameDeviceName.find(index);
				for (std::map<int , CString>::iterator it = m_mSameDeviceName.begin(); it != m_mSameDeviceName.end(); it++)
				{
					if (it->second == ite->second)
					{
						num++;
						csName = it->second;
					}
				}
				if (ite != m_mSameDeviceName.end())
				{
					m_mSameDeviceName.erase(ite);
				}

				if (2 == num)
				{
					for (std::map<int ,CString>::iterator ite = m_mSameDeviceName.begin(); ite != m_mSameDeviceName.end(); ite++)
					{
						if (ite->second == csName)
						{
							m_mSameDeviceName.erase(ite);
							break;
						}
					}					
				}	
			}
		}
		
		pItemInfo->csDeviceId = StreamTypeDlg.m_csDeviceID;
		pItemInfo->csDeviceName = StreamTypeDlg.m_csDeviceName;
		pItemInfo->csDeviceIp = StreamTypeDlg.m_csIp;
		pItemInfo->csDevicePort = StreamTypeDlg.m_csPort;
		if (_T("Axis") == StreamTypeDlg.m_csCompany)
		{
			pItemInfo->csComponyType = _T("2");
			iCompany = 2;
		}
		else if (_T("HIK") == StreamTypeDlg.m_csCompany)
		{
			pItemInfo->csComponyType = _T("4");
			iCompany = 4;
		}
		else
		{
			pItemInfo->csComponyType = _T("9");
			iCompany = 9;
		}
		pItemInfo->csUserName = StreamTypeDlg.m_csUser;
		pItemInfo->csUserPwd = StreamTypeDlg.m_csPassWord;
		pItemInfo->csDeviceType = StreamTypeDlg.m_csDeviceType;

		CString csStreamType = StreamTypeDlg.m_csStreamType;
		pItemInfo->iStreamType = _ttoi(csStreamType);
		
		m_Devlist.SetItemText(index, 0, StreamTypeDlg.m_csDeviceID);
		m_Devlist.SetItemText(index, 1, StreamTypeDlg.m_csDeviceName);
		m_Devlist.SetItemText(index, 2, StreamTypeDlg.m_csIp);
		m_Devlist.SetItemText(index, 3, StreamTypeDlg.m_csPort);
		m_Devlist.SetItemText(index, 4, StreamTypeDlg.m_csCompany);
		m_Devlist.SetItemText(index, 5, StreamTypeDlg.m_csUser);
		m_Devlist.SetItemText(index, 6, StreamTypeDlg.m_csPassWord);
		m_Devlist.SetItemText(index, 7, StreamTypeDlg.m_csDeviceType);

		BOOL bol = FALSE;
		if ( StreamTypeDlg.m_csDeviceID != csDeviceID || StreamTypeDlg.m_csDeviceName != csDeviceName ||
			StreamTypeDlg.m_csIp != csIp || StreamTypeDlg.m_csPort != csPort || StreamTypeDlg.m_csCompany != csCompany ||
			StreamTypeDlg.m_csUser != csUser || StreamTypeDlg.m_csPassWord != csPassWord || StreamTypeDlg.m_csDeviceType != csDeviceType)
		{
			bol = TRUE;
		}

		if (bol)
		{
			//将修改的值保存带Excel
			SaveToExcel(index);

			std::map<int , DEVICEINFO>::iterator it = m_mDeviceMap.find(index);
			if (it != m_mDeviceMap.end())
			{
				it->second.csDeviceId = StreamTypeDlg.m_csDeviceID;
				it->second.csDeviceName = StreamTypeDlg.m_csDeviceName;
			}
			else
			{
				DEVICEINFO info;
				info.csDeviceId = StreamTypeDlg.m_csDeviceID;
				info.csDeviceName =StreamTypeDlg.m_csDeviceName;
				m_mDeviceMap.insert(std::make_pair(index, info));
			}

			CFileFind filefind;
			CString sFilePath(m_csIniPath + _T("*.*"));

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

					if (sFileName != "General.ini")
					{
						CString csGroupNum(sFileName.GetAt(6));

						CIniFile inifile(m_csIniPath + sFileName);

						IpcList ipcList;
						CStringArray GroupArray;
						inifile.ReadSectionString(CARAMPTZ,GroupArray);

						for (int i = 0 ; i < GroupArray.GetSize(); i++)
						{
							std::vector<CString> arry;
							arry = Split(GroupArray[i]);

							if (csDeviceID == arry[2] && csDeviceName == arry[8] && csIp == arry[3] && csCompany == arry[7])
							{
								CString csKey,csValue;

								csKey.Format(_T("group%s"), arry[1]);
					
								csValue.Format(_T("%s,%s,%s,%s,%s,%s,%s,%d,%s,%s"), csGroupNum, arry[1], StreamTypeDlg.m_csDeviceID,
										StreamTypeDlg.m_csIp, StreamTypeDlg.m_csPort,
										StreamTypeDlg.m_csUser, StreamTypeDlg.m_csPassWord, 
										iCompany, StreamTypeDlg.m_csDeviceName, StreamTypeDlg.m_csStreamType);

								inifile.WriteString(CARAMPTZ, csKey, csValue);
								break;
							}
						}
					}
				}//if (sFileName != "General.ini")
			}
			m_Devlist.Invalidate(TRUE);
			CleanGroupTree();
			ReadGroupIPC();

		}//if (bol)
		
	}

	/*for (std::map<int , DEVICEINFO>::iterator it = m_mDeviceMap.begin(); it != m_mDeviceMap.end(); it++)
	{
		CString str;
		str.Format(_T("%d"), it->first);
		MessageBox(_T("MAP ,")+str+_T("-")+it->second.csDeviceId+_T("--")+it->second.csDeviceName);
	}
	for (std::map<int , CString>::iterator it = m_mSameDeviceID.begin(); it != m_mSameDeviceID.end(); it++)
	{
		CString str;
		str.Format(_T("%d"), it->first);
		MessageBox(_T("ID ,")+str+_T("-")+it->second);
	}
	for (std::map<int , CString>::iterator it = m_mSameDeviceName.begin(); it != m_mSameDeviceName.end(); it++)
	{
		CString str;
		str.Format(_T("%d"), it->first);
		MessageBox(_T("NAME ,")+str+_T("-")+it->second);
	}*/
	*pResult = 0;
}

void IpcGroupCfg::SaveToExcel(int iIndex)
{
	DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(iIndex);
	BasicExcelCell *exl;
	BasicExcel e;
	char buf[200];
	int k;
	BasicExcelWorksheet* sheet;
	CString FileName = /*_T("C:\\Users\\zdx\\Desktop\\output.xls")*/m_csDeviceFile;
	for(k=0;k<FileName.GetLength();k++)
		buf[k]=FileName.GetAt(k);
	buf[k]=0;

	e.Load(buf);
	//delete [] Buf;
	sheet = e.GetWorksheet("devices");
	if (sheet)
	{
		USES_CONVERSION;  
		int iCols = sheet->GetTotalCols();

		CString csExlType(sheet->Cell(0, 0)->GetString());
		if (csExlType == _T("deviceid"))		//dag导出
		{
			for (int j = 0; j< /*iCols*/8; j++)
			{
				char *buf = NULL;
				exl = sheet->Cell(iIndex+1, j);
				if (0 == j)
				{
					buf = WtoM(pItemInfo->csDeviceId);
					exl->SetString(buf);
					delete [] buf;
				}
				if (1 == j)
				{
					buf = WtoM(pItemInfo->csDeviceName);
					exl->SetString(buf);
					delete [] buf;
				}
				if (2 == j)
				{
					buf = WtoM(pItemInfo->csDeviceIp);
					exl->SetString(buf);
					delete [] buf;
				}
				if (3 == j)
				{
					buf = WtoM(pItemInfo->csDevicePort);
					exl->SetString(buf);
					delete [] buf;
				}
				if (4 == j)
				{
					if ("2" == pItemInfo->csComponyType)
					{
						exl->SetString("Axis");
					}
					else if ("4" == pItemInfo->csComponyType)
					{
						exl->SetString("HIK");
					}
					else
					{
						exl->SetString("DEV 2.0");
					}
					
				}
				if (5 == j)
				{
					buf = WtoM(pItemInfo->csUserName);
					exl->SetString(buf);
					delete [] buf;
				}
				if (6 == j)
				{
					buf = WtoM(pItemInfo->csUserPwd);
					exl->SetString(buf);
					delete [] buf;
				}
				if (7 == j)
				{
					buf = WtoM(pItemInfo->csDeviceType);
					exl->SetString(buf);
					delete [] buf;
				}
				e.Save();
			}
		}
		else
		{
			for (int j = 0; j< /*iCols*/14; j++)
			{
				char *buf = NULL;
				exl = sheet->Cell(iIndex+1, j);
				if (0 == j)
				{
					buf = WtoM(pItemInfo->csDeviceName);
					exl->SetString(buf);
					delete [] buf;
				}
				if (1 == j)
				{
					buf = WtoM(pItemInfo->csDeviceType);
					exl->SetString(buf);
					delete [] buf;
				}
				if (3 == j)
				{
					buf = WtoM(pItemInfo->csDeviceIp);
					exl->SetString(buf);
					delete [] buf;
				}
				if (4 == j)
				{
					buf = WtoM(pItemInfo->csDevicePort);
					exl->SetString(buf);
					delete [] buf;
				}
				if (12 == j)
				{
					if ("2" == pItemInfo->csComponyType)
					{
						exl->SetString("Axis");
					}
					else if ("4" == pItemInfo->csComponyType)
					{
						exl->SetString("HIK");
					}
					else
					{
						exl->SetString("DEV 2.0");
					}
				}
				if (5 == j)
				{
					buf = WtoM(pItemInfo->csUserName);
					exl->SetString(buf);
					delete [] buf;
				}
				if (6 == j)
				{
					buf = WtoM(pItemInfo->csUserPwd);
					exl->SetString(buf);
					delete [] buf;
				}
				if (13 == j)
				{
					buf = WtoM(pItemInfo->csDeviceId);
					exl->SetString(buf);
					delete [] buf;
				}
				e.Save();
			}		
		}

	}

}
char *IpcGroupCfg::WtoM(CString csStr)
{
	int ByteNum = ::WideCharToMultiByte(CP_ACP, NULL,csStr, -1, NULL, NULL, NULL, NULL);
	char *Buf = new char[ByteNum+1];
	::WideCharToMultiByte(CP_ACP, NULL, csStr, -1, Buf, ByteNum, NULL,NULL);
	Buf[ByteNum] = '\0';
	
	return Buf;
}
void IpcGroupCfg::CleanGroupTree()
{
	//m_TextDemoTree 先清理上次的先关资源
	HTREEITEM hItem;  
	hItem = m_GroupTree.GetRootItem();  //获得根目录节点  
	if(!m_GroupTree.ItemHasChildren(hItem))
	{
		return;
	}
	//DeleteItemData(&m_TextDemoTree,hItem );
	m_GroupTree.DeleteAllItems();
}
void IpcGroupCfg::ReadGroupIPC()
{
	CFileFind filefind;
	CString sFilePath(m_csIniPath + _T("*.*"));

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

			if (sFileName != "General.ini")
			{
				CString csGroupNum(sFileName.GetAt(6));
				HTREEITEM htrCurrent;
				HTREEITEM hItem = m_GroupTree.GetChildItem(TVI_ROOT);
				while (hItem) 
				{
					CString strText= m_GroupTree.GetItemText(hItem);
					if( _T("Group_") + csGroupNum == strText)
					{
						htrCurrent = hItem;
						break;
					}
					hItem = m_GroupTree.GetNextSiblingItem(hItem); 
				}
				if (NULL == hItem)
				{
					htrCurrent = m_GroupTree.InsertItem(_T("Group_") + csGroupNum , 0, 0, TVI_ROOT);
				}

				HTREEITEM er = m_GroupTree.GetChildItem(htrCurrent);
				while(er)
				{
					HTREEITEM Temp = er; 
					er = m_GroupTree.GetNextSiblingItem(er);
					m_GroupTree.DeleteItem(Temp);
				}

				CIniFile inifile(m_csIniPath + sFileName);

				IpcList ipcList;
				CStringArray GroupArray;
				inifile.ReadSectionString(CARAMPTZ,GroupArray);

				for (int i = 0 ; i < GroupArray.GetSize(); i++)
				{
					std::vector<CString> arry;
					arry = Split(GroupArray[i]);
					CString csTreeValue;
					if ("2" == arry[7])
					{
						csTreeValue.Format(_T("%s, %s, Axis"),arry[8], arry[3]);
					}
					else if ("4" == arry[7])
					{
						csTreeValue.Format(_T("%s, %s, HIK"),arry[8], arry[3]);
					}
					else
					{
						csTreeValue.Format(_T("%s, %s, DEV 2.0"),arry[8], arry[3]);
					}
					m_GroupTree.InsertItem(csTreeValue, 0, 0, htrCurrent);
					m_GroupTree.Expand(htrCurrent, TVE_EXPAND);
					DeviceItemInfo pItemInfo ;

					pItemInfo.csComponyType = arry[7];
					pItemInfo.csDeviceId = arry[2];
					pItemInfo.csDeviceIp = arry[3];
					pItemInfo.csDeviceName = arry[8];
					pItemInfo.csDevicePort = arry[4];
					pItemInfo.csUserName = arry[5];
					pItemInfo.csUserPwd = arry[6];
					pItemInfo.csDeviceType = _T("IPC");
					pItemInfo.iStreamType = _ttoi(arry[9])&0xff;
					
					ipcList.push_back(pItemInfo);
				}
				m_pMainCfgDlg->m_IpcGroupMap[_ttoi(csGroupNum)] = ipcList;
			}

		}
	}
}
std::vector<CString> IpcGroupCfg::Split(CString string, CString separator)
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
void IpcGroupCfg::OnHdnItemclickListIpc(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int nItem =phdr->iItem;

	if (0 != nItem)
		return;

	HDITEM hdItem;
	hdItem.mask= HDI_IMAGE | HDI_FORMAT;
	m_pHeadCtrl =m_Devlist.GetHeaderCtrl();

	ASSERT(m_pHeadCtrl->GetSafeHwnd());
	VERIFY(m_pHeadCtrl->GetItem(nItem, &hdItem) );

	if (hdItem.iImage == 1)
		hdItem.iImage = 3;
	else
		hdItem.iImage = 1;

	VERIFY(m_pHeadCtrl->SetItem(nItem, &hdItem) );

	BOOL bl =hdItem.iImage == 3 ? TRUE : FALSE;
	int nCount = m_Devlist.GetItemCount();   

	for(nItem = 0; nItem < nCount; nItem++)
	{
		ListView_SetCheckState(m_Devlist.GetSafeHwnd(), nItem, bl);
	}   

	*pResult = 0;
}

void IpcGroupCfg::OnLvnItemchangedListIpc(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	if (LVIF_STATE== pNMLV->uChanged)
	{
		BOOL blAllChecked = TRUE;
		int nCount =m_Devlist.GetItemCount();
		for(int nItem = 0; nItem < nCount; nItem++)
		{
			if (!ListView_GetCheckState(m_Devlist.GetSafeHwnd(), nItem) )
			{
				blAllChecked = FALSE;
				break;
			}
		}

		HDITEM hdItem;
		hdItem.mask = HDI_IMAGE;

		if (blAllChecked)
			hdItem.iImage = 3;
		else
			hdItem.iImage = 1;

		m_pHeadCtrl= m_Devlist.GetHeaderCtrl();

		ASSERT(m_pHeadCtrl->GetSafeHwnd());
		VERIFY(m_pHeadCtrl->SetItem(0, &hdItem) );
	}

	*pResult = 0;
}

void IpcGroupCfg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	 if (BST_CHECKED ==  m_cCheckBox.GetCheck())
	 {
		 int iRowCount = m_Devlist.GetItemCount();
		 if (iRowCount > 0)
		 {
			 for(int i=0; i < iRowCount; i++)
			 {
				DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(i);
				pItemInfo->csDeviceId += _T("01");
				m_Devlist.SetItemText(i, 0, pItemInfo->csDeviceId);
			 }
		 }
	 }
	 else if(BST_UNCHECKED == m_cCheckBox.GetCheck())
	 {
		 int iRowCount = m_Devlist.GetItemCount();
		 if (iRowCount > 0)
		 {
			 for(int i=0; i < iRowCount; i++)
			 {
				 DeviceItemInfo *pItemInfo = (DeviceItemInfo*)m_Devlist.GetItemData(i);
				 pItemInfo->csDeviceId = pItemInfo->csDeviceId.Mid(0, pItemInfo->csDeviceId.GetLength()-2);
				 m_Devlist.SetItemText(i, 0, pItemInfo->csDeviceId);
			 }
		 }
	 }
}

void IpcGroupCfg::OnBnClickedBtnDel()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strDeviceID;
	HTREEITEM htree;
	HTREEITEM hParent = NULL;
	htree = m_GroupTree.GetSelectedItem();
	HTREEITEM hRoot = htree ;
	while ( hParent = m_GroupTree.GetParentItem( hRoot ) )
		hRoot = hParent;

	CString csStrTemp = m_GroupTree.GetItemText(htree);
	m_GroupTree.DeleteItem(htree);

	CString csGroupNum = m_GroupTree.GetItemText(hRoot);
	csGroupNum = csGroupNum.Mid(6, 1);

	IpcList ipcListItem;
	ipcListItem = m_pMainCfgDlg->m_IpcGroupMap[_ttoi(csGroupNum)];
	
	for (IpcList::iterator ite = ipcListItem.begin(); ite != ipcListItem.end(); ite++)
	{
		CString str;
		if (_T("2") == ite->csComponyType)
		{
			str.Format(_T("%s, %s, Axis"), ite->csDeviceName, ite->csDeviceIp);
		}
		else if (_T("4") == ite->csComponyType)
		{
			str.Format(_T("%s, %s, HIK"), ite->csDeviceName, ite->csDeviceIp);
		}
		else
		{
			str.Format(_T("%s, %s, DEV 2.0"), ite->csDeviceName, ite->csDeviceIp);
		}
		if (str == csStrTemp)
		{
			strDeviceID = ite->csDeviceId;
			ipcListItem.erase(ite);
			break;
		}
	}

	IpcGroupMap ::iterator ite = m_pMainCfgDlg->m_IpcGroupMap.find(_ttoi(csGroupNum));
	if (ite != m_pMainCfgDlg->m_IpcGroupMap.end())
	{
		ite->second.clear();		//清除指定组的IPC信息
	}
	m_pMainCfgDlg->m_IpcGroupMap[_ttoi(csGroupNum)] = ipcListItem;

	CString csIniFileName = m_csIniPath + _T("Group_") + csGroupNum + _T(".ini");
	CIniFile iniFile(csIniFileName);

	//清除以前的值
	iniFile.EraseSection(CARAMPTZ);
	int iIndex = 0;

	CString csKey,csValue;
	for (IpcList::iterator it = ipcListItem.begin(); it != ipcListItem.end(); it++)
	{
		csKey.Format(_T("group%d"), ++iIndex);
		
		csValue.Format(_T("%s,%d,%s,%s,%s,%s,%s,%s,%s,%d"), csGroupNum, iIndex,it->csDeviceId,
				it->csDeviceIp,it->csDevicePort,
				it->csUserName, it->csUserPwd, 
				it->csComponyType, it->csDeviceName, it->iStreamType);
	
		iniFile.WriteString(CARAMPTZ, csKey, csValue);
	}

	CStringArray GroupArray;
	iniFile.ReadSectionString(SCREENCAMERA,GroupArray);

	for (int i = 0 ; i < GroupArray.GetSize(); i++)
	{
		std::vector<CString> arry;
		arry = Split(GroupArray[i]);
		std::vector<CString> ipcList;
		ipcList = Split(arry[5], _T("-"));
		CString strIPC;
		int ipcListSize = ipcList.size();
		for (int ii = 0; ii < ipcListSize; ii++)
		{
			if (ipcList[ii] == strDeviceID)
			{
				strIPC += _T("0");
			}
			else
			{
				strIPC += ipcList[ii];
			}		
			strIPC += _T("-");
		}
		CString csKeyEx,csValueEx;
		
		strIPC = strIPC.Mid(0, strIPC.GetLength()-1);
		csKeyEx.Format(_T("ScreenCamera%s_%s"), arry[1], arry[2]);
		csValueEx.Format(_T("%s,%s,%s,%s,%s,%s"), csGroupNum, arry[1], arry[2], arry[3], arry[4], strIPC);
		iniFile.WriteString(SCREENCAMERA, csKeyEx, csValueEx);
	}
}
void IpcGroupCfg::OnCustomdrawMyList(NMHDR* pNMHDR, LRESULT* pResult)  
{  
	//This code based on Michael Dunn's excellent article on  
	//list control custom draw at http://www.codeproject.com/listctrl/lvcustomdraw.asp  


	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );  


	// Take the default processing unless we set this to something else below.  
	*pResult = CDRF_DODEFAULT;  


	// First thing - check the draw stage. If it's the control's prepaint  
	// stage, then tell Windows we want messages for every item.  
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )  
	{  
		*pResult = CDRF_NOTIFYITEMDRAW;  
	}  
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )  
	{  
		// This is the notification message for an item.  We'll request  
		// notifications before each subitem's prepaint stage.  


		*pResult = CDRF_NOTIFYSUBITEMDRAW;  
	}  
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )  
	{  
		COLORREF clrNewTextColor, clrNewBkColor;  

		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );  

		for (std::map<int , CString>::iterator ite = m_mSameDeviceID.begin(); ite != m_mSameDeviceID.end(); ite++)
		{
			if (ite->first == nItem)
			{
				clrNewTextColor = RGB(0,0,0);       //text  
				clrNewBkColor = RGB(255,0,0);  
				pLVCD->clrText = clrNewTextColor;  
				pLVCD->clrTextBk = clrNewBkColor;  //红色
			}
		}
		for (std::map<int , CString>::iterator ite = m_mSameDeviceName.begin(); ite != m_mSameDeviceName.end(); ite++)
		{
			if (ite->first == nItem)
			{
				clrNewTextColor = RGB(0,0,0);       //text  
				clrNewBkColor = RGB(255,0,0);  
				pLVCD->clrText = clrNewTextColor;  
				pLVCD->clrTextBk = clrNewBkColor;  //红色
			}
		}
	
		
		// Tell Windows to paint the control itself.  
		*pResult = CDRF_DODEFAULT; 
	}  
}