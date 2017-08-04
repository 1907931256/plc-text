// ScreenDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "ScreenDlg.h"
#include "GroupCfgSet.h"
#include <vector>

// CScreenDlg 对话框

IMPLEMENT_DYNAMIC(CScreenDlg, CDialog)

CScreenDlg::CScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenDlg::IDD, pParent)
	,m_pGroupCfgSetDlg(NULL)
	,m_iIndex(0)
	,m_csText(_T("0.0.0.0"))
	,m_csIpcPuid(_T("0"))
{
	m_brushVideo.CreateSolidBrush(RGB(/*60, 60, 60*/208, 226, 237));

}

CScreenDlg::~CScreenDlg()
{
}

void CScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScreenDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
    ON_WM_CONTEXTMENU()  
	ON_COMMAND(ID_CLEAN, &CScreenDlg::OnClean)
	ON_COMMAND(ID_32774, &CScreenDlg::On32774)
END_MESSAGE_MAP()


// CScreenDlg 消息处理程序

HBRUSH CScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	switch(nCtlColor)
	{
	case CTLCOLOR_DLG:
		{
			int id = pWnd->GetDlgCtrlID();
			//if (id == IDD_VIDEO_WND)
			//{
			hbr = (HBRUSH)m_brushVideo.GetSafeHandle();
			//}
		}
		//pDC->SetBkColor(RGB(180,180,180));
		break;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CScreenDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	m_brushVideo.DeleteObject();
}

void CScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pGroupCfgSetDlg->m_nCurIndex = m_iIndex;
	m_pGroupCfgSetDlg->DrawActivePage(TRUE);
	CDialog::OnLButtonDown(nFlags, point);
}

void CScreenDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	CDC *pDC = GetDC();
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(rect);
	
	pDC->TextOut(2, (rect.Height()-15)/2, m_csText);
	// 不为绘图消息调用 CDialog::OnPaint()
}

void CScreenDlg::OnContextMenu(CWnd* , CPoint point)
{
//	CMenu   menu;   //定义下面要用到的cmenu对象
//	menu.LoadMenu(IDR_MENU2); //装载自定义的右键菜单 
//	CMenu   *pContextMenu=menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单 
//	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd()); //在指定位置显示弹出菜单

	CMenu popMenu;
	popMenu.LoadMenu(IDR_MENU2);		//载入菜单
	CMenu *pPopup;
	pPopup=popMenu.GetSubMenu(0);	//获得子菜单指针

	pPopup->EnableMenuItem(ID_CLEAN,MF_BYCOMMAND|MF_ENABLED);	//允许菜单项使用
	//pPopup->EnableMenuItem(ADD,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);	//不允许菜单项使用

	//ClientToScreen(&point);		//将客户区坐标转换成屏幕坐标
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);			//显示弹出菜单，参数依次为(鼠标在菜单左边|跟踪右键，x，y，this)
	pPopup->Detach();
	popMenu.DestroyMenu();
}


void CScreenDlg::OnClean()
{
	// TODO: 在此添加命令处理程序代码
	int mm = 12;
}

void CScreenDlg::On32774()
{
	// TODO: 在此添加命令处理程序代码
	m_pGroupCfgSetDlg->m_videoWnd[m_iIndex].SetOutText(_T("0.0.0.0"));
	m_pGroupCfgSetDlg->m_videoWnd[m_iIndex].m_csIpcPuid = _T("0");
	m_pGroupCfgSetDlg->m_videoWnd[m_iIndex].Invalidate(TRUE);
	
	//CString csGroupNum, csScreenNum, csMode, csSplitNum;

	//csGroupNum.Format(_T("%d"),  m_pGroupCfgSetDlg->m_ComBoxGroup.GetCurSel()+1);
	//csScreenNum.Format(_T("%d"),  m_pGroupCfgSetDlg->m_ComBoxScreenIndex.GetCurSel()+1);
	//csMode.Format(_T("%d"),  m_pGroupCfgSetDlg->m_ComBoxMode.GetCurSel());
	//m_pGroupCfgSetDlg->m_ScreenSplitNum.GetWindowText(csSplitNum);
	//
	//CString csFileName =  m_pGroupCfgSetDlg->m_csIniPath+_T("Group_")+csGroupNum+_T(".ini");
	//CIniFile infile(csFileName);

	//CStringArray GroupArray;
	//infile.ReadSectionString(_T("CARAMPTZ"), GroupArray);
	//for (int i = 0; i < GroupArray.GetSize(); i++)
	//{
	//	std::vector<CString> sIpcList;
	//	sIpcList = Split(GroupArray[i]);
	//	if (sIpcList[0] == csGroupNum && sIpcList[1] == csScreenNum && sIpcList[2] == csMode && sIpcList[3] == csSplitNum)
	//	{
	//		CString csKeyEx, csValueEx, csIpcIdList;

	//		for (int i = 0; i < _ttoi(csSplitNum); i++)
	//		{
	//			CString str =  m_pGroupCfgSetDlg->m_videoWnd[i].m_csIpcPuid + _T("-");
	//			csIpcIdList += str;
	//		}
	//		csIpcIdList = csIpcIdList.Left(csIpcIdList.GetLength() - 1);

	//		csKeyEx.Format(_T("ScreenCamera%s_%s"), csScreenNum, csMode);
	//		csValueEx.Format(_T("%s,%s,%s,%s,%s,%s"), csGroupNum, csScreenNum, csMode, csSplitNum, _T("1"), csIpcIdList);
	//		infile.WriteString(SCREENCAMERA, csKeyEx, csValueEx);//写tv_wall和屏幕的分屏和上墙信息
	//	}
	//}

}
std::vector<CString> CScreenDlg::Split(CString string, CString separator)
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
