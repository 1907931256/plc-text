// ScreenDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "ScreenDlg.h"
#include "GroupCfgSet.h"
#include <vector>

// CScreenDlg �Ի���

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


// CScreenDlg ��Ϣ�������

HBRUSH CScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
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

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CScreenDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	m_brushVideo.DeleteObject();
}

void CScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_pGroupCfgSetDlg->m_nCurIndex = m_iIndex;
	m_pGroupCfgSetDlg->DrawActivePage(TRUE);
	CDialog::OnLButtonDown(nFlags, point);
}

void CScreenDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: �ڴ˴������Ϣ����������
	CDC *pDC = GetDC();
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(rect);
	
	pDC->TextOut(2, (rect.Height()-15)/2, m_csText);
	// ��Ϊ��ͼ��Ϣ���� CDialog::OnPaint()
}

void CScreenDlg::OnContextMenu(CWnd* , CPoint point)
{
//	CMenu   menu;   //��������Ҫ�õ���cmenu����
//	menu.LoadMenu(IDR_MENU2); //װ���Զ�����Ҽ��˵� 
//	CMenu   *pContextMenu=menu.GetSubMenu(0); //��ȡ��һ�������˵������Ե�һ���˵��������Ӳ˵� 
//	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd()); //��ָ��λ����ʾ�����˵�

	CMenu popMenu;
	popMenu.LoadMenu(IDR_MENU2);		//����˵�
	CMenu *pPopup;
	pPopup=popMenu.GetSubMenu(0);	//����Ӳ˵�ָ��

	pPopup->EnableMenuItem(ID_CLEAN,MF_BYCOMMAND|MF_ENABLED);	//����˵���ʹ��
	//pPopup->EnableMenuItem(ADD,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);	//������˵���ʹ��

	//ClientToScreen(&point);		//���ͻ�������ת������Ļ����
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);			//��ʾ�����˵�����������Ϊ(����ڲ˵����|�����Ҽ���x��y��this)
	pPopup->Detach();
	popMenu.DestroyMenu();
}


void CScreenDlg::OnClean()
{
	// TODO: �ڴ���������������
	int mm = 12;
}

void CScreenDlg::On32774()
{
	// TODO: �ڴ���������������
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
	//		infile.WriteString(SCREENCAMERA, csKeyEx, csValueEx);//дtv_wall����Ļ�ķ�������ǽ��Ϣ
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
