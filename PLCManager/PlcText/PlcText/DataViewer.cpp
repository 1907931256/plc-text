// DataViewer.cpp : 实现文件
//

#include "stdafx.h"
#include "PlcText.h"
#include "DataViewer.h"


// DataViewer 对话框

IMPLEMENT_DYNAMIC(DataViewer, CDialog)

DataViewer::DataViewer(CWnd* pParent /*=NULL*/)
	: CDialog(DataViewer::IDD, pParent)
{

}

DataViewer::~DataViewer()
{
}

void DataViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SHOW, mList);
}


BEGIN_MESSAGE_MAP(DataViewer, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON_ADD, &DataViewer::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &DataViewer::OnBnClickedButtonClear)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_SHOW, OnGetdispinfoList)
	ON_MESSAGE(WM_DATASHOW,&DataViewer::OnDataShow)
ON_WM_CLOSE()
END_MESSAGE_MAP()


// DataViewer 消息处理程序

int DataViewer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}
BOOL DataViewer::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CComboBox*)GetDlgItem(IDC_COMBO_TYPE))->AddString(_T("BOOL"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TYPE))->AddString(_T("Short Int"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TYPE))->SetCurSel(0);

	for(int i=0;i<12;i++)
	{
		CString str;
		str.Format(_T("DB5%02d"),i+1);
		((CComboBox*)GetDlgItem(IDC_COMBO_DB))->AddString(str);
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_DB))->SetCurSel(0);
	mList.ModifyStyle(LVS_EDITLABELS, 0);
	mList.ModifyStyle(0, LVS_REPORT);
	mList.ModifyStyle(0, LVS_SHOWSELALWAYS);
	mList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );
//	mList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | 
//	LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE | LVS_EX_MULTIWORKAREAS/*| LVS_EX_FLATSB|
//																	  LVS_NOSORTHEADER*/);//允许选择正行、报表头可以拖拽、绘制表格、单击激活选择条目、扁平滚动条
	//mList.SetExtendedStyle(mList.GetExtendedStyle()|LVS_EX_SUBITEMIMAGES);
	mList.SetTextBkColor(RGB(207,214,229));
	//添加列
	mList.InsertColumn(0,_T("TIME"));
	mList.InsertColumn(1, _T("ADDRESS"));
	mList.InsertColumn(2, _T("TYPE"));
	mList.InsertColumn(3, _T("DATA"));
	//设置列宽
	mList.SetColumnWidth(0, 100);
	mList.SetColumnWidth(1, 100);
	mList.SetColumnWidth(2, 100);
	mList.SetColumnWidth(3, 100);
	return 0;

}

void DataViewer::OnBnClickedButtonAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	int addrStart=0,addrEnd=0;
	BOOL errorFlag;
	addrStart=GetDlgItemInt(IDC_EDIT_START,&errorFlag,TRUE);
	addrEnd = GetDlgItemInt(IDC_EDIT_END,&errorFlag,TRUE);
	int type = ((CComboBox*)GetDlgItem(IDC_COMBO_TYPE))->GetCurSel();
	int db = ((CComboBox*)GetDlgItem(IDC_COMBO_DB))->GetCurSel()+1;
	if(addrStart<0||addrEnd<0||addrStart>addrEnd)
	{
		MessageBox(_T("Data Select Error！"),_T("ERROR"),MB_OK|MB_ICONERROR);
		return;
	}
	showInfo info;
	info.addressStart = addrStart;
	info.addressEnd = addrEnd;
	info.dataType = type;
	info.db = db;
	dataInfo.push_back(info);
	MessageBox(_T("Add Success！"),_T("INFORMATION"),MB_OK|MB_ICONINFORMATION);
}

void DataViewer::OnBnClickedButtonClear()
{
	// TODO: 在此添加控件通知处理程序代码
	dataSection.Lock();
	dataInfo.clear();
	dataSection.Unlock();	
	mList.DeleteAllItems();
}

LRESULT DataViewer::OnDataShow(WPARAM wparam,LPARAM lparam)
{
	char* pstr = (char*)lparam;
	BufferLock.Lock();
	memcpy(dataBufferG,pstr,BLOCK_SIZE_COPY);
	_beginthreadex(NULL,0,dataShowThread,(LPVOID)this,0,0);
	return 0;

}

void DataViewer::showBoolData(int db,int &count,int address)
{
	CString str;
	dataShow showStr;
	for(int k=0;k<8;k++)
	{	
		str.Format(_T("DB5%02d+%d.%d"),db,address,k);
		showStr.address = str;
		//mList.InsertItem(count,str);
		str=_T("BOOL");
	//	mList.SetItemText(count,1,str);
		showStr.type = str;
		switch (k)
		{
		case 0:
			str.Format(_T("%d"),dataS.bf.a);
			break;
		case 1:
			str.Format(_T("%d"),dataS.bf.b);
			break;
		case 2:
			str.Format(_T("%d"),dataS.bf.c);	
			break;
		case 3:
			str.Format(_T("%d"),dataS.bf.d);
			break;
		case 4:
			str.Format(_T("%d"),dataS.bf.e);
			break;
		case 5:
			str.Format(_T("%d"),dataS.bf.f);
			break;
		case 6:
			str.Format(_T("%d"),dataS.bf.g);
			break;
		case 7:
			str.Format(_T("%d"),dataS.bf.h);
			break;
		}
		showStr.value = str;
		showStr.time = timeGetStr;
		m_database.push_back(showStr);
		//mList.SetItemText(count,2,str);
		count++;
	}
}
void DataViewer::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	::PostMessage(fHandle,WM_DATAVIEWER_CLOSE,NULL,NULL);
	CDialog::OnClose();
}

UINT WINAPI DataViewer::dataShowThread(LPVOID pParam)
{
	DataViewer* This = reinterpret_cast<DataViewer*>(pParam);
	char dataBuf[BLOCK_SIZE_COPY]={0};
	memcpy(dataBuf,This->dataBufferG,BLOCK_SIZE_COPY);
	This->BufferLock.Unlock();
	return This->dataShowMain(dataBuf);
}

UINT DataViewer::dataShowMain(char* pBuffer)
{
	char data[BLOCK_SIZE_COPY];
	memcpy(data,pBuffer,sizeof(data));
	if(m_database.size()>10000)
		m_database.clear();
	dataSection.Lock();
	std::vector<showInfo> InfoShow = dataInfo;
	dataSection.Unlock();
	timeGet(timeGetStr);
	int itemCount =0;
	for(int i=0;i<InfoShow.size();i++)
	{	
		if(data[BLOCK_SIZE_COPY-1]!=InfoShow[i].db)
			continue;
		//if(mList.GetItemCount()>2048)
		//	mList.DeleteAllItems();
		int type = InfoShow[i].dataType;
		for(int j=InfoShow[i].addressStart;j<=InfoShow[i].addressEnd;j++)
		{
			//	mList.InsertItem(1000,_T(""));
			if(type==DATA_TYPPE_BOOL)
			{
				dataS.n = (unsigned char) data[j];
				showBoolData(InfoShow[i].db,itemCount,j);
			}
			else if(type == DATA_TYPE_INT)
			{
				dataShow  showStr;
				CString str;
				str.Format(_T("DB5%02d+%d"),InfoShow[i].db,j);
				//mList.InsertItem(itemCount,str);
				showStr.address = str;
				str=_T("INT");
				//mList.SetItemText(itemCount,1,str);
				showStr.type = str;
				int a=0;
				a = data[j];
				j++;
				a=(a<<8)+data[j];
				str.Format(_T("%d"),a);
				showStr.value = str;
				showStr.time = timeGetStr;
				m_database.push_back(showStr);
				//mList.SetItemText(itemCount,2,str);
				itemCount++;
			}
		}
	}
	//delete []pstr;
	//delete[]dataG;
	mList.SetItemCount( m_database.size() );

	return 0 ;
}
//This function is called when the list needs data. This is the most
//critical function when working with virtual lists.
void DataViewer::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	//Create a pointer to the item
	LV_ITEM* pItem= &(pDispInfo)->item;

	//Which item number?
	int itemid = pItem->iItem;

	//Do the list need text information?
	if (pItem->mask & LVIF_TEXT)
	{
		CString text;

		//Which column?
		if(pItem->iSubItem == 0)
		{
			//Text is name
			text = m_database[itemid].time;
		}
		else if(pItem->iSubItem == 1)
		{
			//Text is name
			text = m_database[itemid].address;
		}
		else if (pItem->iSubItem == 2)
		{
			//Text is slogan
			text = m_database[itemid].type;
		}
		else if (pItem->iSubItem == 3)
		{
			//Text is slogan
			text = m_database[itemid].value;
		}


		//Copy the text to the LV_ITEM structure
		//Maximum number of characters is in pItem->cchTextMax
		lstrcpyn(pItem->pszText, text, pItem->cchTextMax);
	}

	//Do the list need image information?
	//if( pItem->mask & LVIF_IMAGE) 
	//{
	//	//Set which image to use
	//	pItem->iImage=m_database[itemid].m_image;

	//	//Show check box?
	//	if(IsCheckBoxesVisible())
	//	{
	//		//To enable check box, we have to enable state mask...
	//		pItem->mask |= LVIF_STATE;
	//		pItem->stateMask = LVIS_STATEIMAGEMASK;

	//		if(m_database[itemid].m_checked)
	//		{
	//			//Turn check box on..
	//			pItem->state = INDEXTOSTATEIMAGEMASK(2);
	//		}
	//		else
	//		{
	//			//Turn check box off
	//			pItem->state = INDEXTOSTATEIMAGEMASK(1);
	//		}
	//	}
	//}

	*pResult = 0;
}
void DataViewer::timeGet(CString& timeStr)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	timeStr.Format(_T("%02d:%02d:%02d:%03d"),sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
}