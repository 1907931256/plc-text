// PtzTestPanel.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PlcText.h"
#include "PtzTestPanel.h"


enum ptzCmdIndex {WIPER=0, UP,DOWN,LEFT,RIGHT,ZOOM_ADD=5,
ZOOM_REDUCE,LEFT_UP,LEFT_DOWN,RIGHT_UP,RIGHT_DOWN=10,
STOP};
// CPtzTestPanel �Ի���

IMPLEMENT_DYNAMIC(CPtzTestPanel, CDialog)

CPtzTestPanel::CPtzTestPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CPtzTestPanel::IDD, pParent)
{

}

CPtzTestPanel::~CPtzTestPanel()
{
}

void CPtzTestPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPtzTestPanel, CDialog)
END_MESSAGE_MAP()


// CPtzTestPanel ��Ϣ�������

BOOL CPtzTestPanel::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(pMsg->message ==WM_LBUTTONDOWN)
	{
		buttonDownProc(pMsg);
	}
	else if(pMsg->message ==WM_LBUTTONUP)
	{
		buttonUpProc(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CPtzTestPanel::buttonDownProc(MSG* pMsg)
{
	ptzParamGet();
	mPtzOpeParam.cmdIndex=STOP;
	//===WIPER=0, UP,DOWN,LEFT,RIGHT,ZOOM_ADD=5,ZOOM_REDUCE,LEFT_UP,LEFT_DOWN,RIGHT_UP,RIGHT_DOWN=10,STOP
	int ptzButton[11]={0, IDC_BUTTON_UP_TP,IDC_BUTTON_DOWN_TP,IDC_BUTTON_LEFT_TP,IDC_BUTTON_RIGHT_TP,
		IDC_BUTTON_ZOOM_IN_TP,IDC_BUTTON_ZOOM_OUT_TP,IDC_BUTTON_LEFTUP_TP,IDC_BUTTON_LEFTDOWN_TP,
		IDC_BUTTON_RIGHTUP_TP,IDC_BUTTON_RIGHTDOWN_TP};
	bool rect = false;
	for(int j=1;j<11;j++)
	{
		if(pMsg->hwnd==GetDlgItem(ptzButton[j])->m_hWnd)
		{
			mPtzOpeParam.cmdIndex = j;
			rect = true;
		}
	}
	if(rect)
		::SendMessage(fHandle,WM_PTZOPERATIONTEST,reinterpret_cast<WPARAM>((void*)(&mPtzOpeParam)),NULL);

}

void CPtzTestPanel::buttonUpProc(MSG* pMsg)
{
	ptzParamGet();
	mPtzOpeParam.cmdIndex=STOP;
	int ptzButton[11]={0, IDC_BUTTON_UP_TP,IDC_BUTTON_DOWN_TP,IDC_BUTTON_LEFT_TP,IDC_BUTTON_RIGHT_TP,
		IDC_BUTTON_ZOOM_IN_TP,IDC_BUTTON_ZOOM_OUT_TP,IDC_BUTTON_LEFTUP_TP,IDC_BUTTON_LEFTDOWN_TP,
		IDC_BUTTON_RIGHTUP_TP,IDC_BUTTON_RIGHTDOWN_TP};
	bool rect = false;
	for(int j=1;j<11;j++)
	{
		if(pMsg->hwnd==GetDlgItem(ptzButton[j])->m_hWnd)
		{
			//mPtzOpeParam.cmdIndex = j;
			rect = true;
		}
	}
	if(rect)
		::SendMessage(fHandle,WM_PTZOPERATIONTEST,reinterpret_cast<WPARAM>((void*)(&mPtzOpeParam)),NULL);
}

void CPtzTestPanel::ptzParamGet()
{
	BOOL errorFlag;
	mPtzOpeParam.nGroup = GetDlgItemInt(IDC_EDIT_GROUP_TP,&errorFlag,TRUE);
	mPtzOpeParam.nIndex = GetDlgItemInt(IDC_EDIT_IPCINDEX_TP,&errorFlag,TRUE);
	mPtzOpeParam.type = ((CButton*)GetDlgItem(IDC_CHECK_CONNECTCAM_TP))->GetCheck(); //0δѡ�� 1ѡ��
}
