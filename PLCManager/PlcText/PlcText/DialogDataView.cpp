// DialogDataView.cpp : implementation file
//

#include "stdafx.h"
#include "PlcText.h"
#include "DialogDataView.h"


// CDialogDataView dialog

IMPLEMENT_DYNAMIC(CDialogDataView, CDialog)

CDialogDataView::CDialogDataView(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogDataView::IDD, pParent)
	, m_nDataType(0)
	, m_nDataType1(0)
{

}

CDialogDataView::~CDialogDataView()
{
}

void CDialogDataView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_BYTE, m_nDataType);
	DDV_MinMaxInt(pDX, m_nDataType, 0, 3);
}


BEGIN_MESSAGE_MAP(CDialogDataView, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE, &CDialogDataView::OnBnClickedCheckEnable)
END_MESSAGE_MAP()


// CDialogDataView message handlers

void CDialogDataView::OnBnClickedCheckEnable()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_CHECK_ENABLE) == BST_CHECKED;
	
	//GetDlgItem(IDC_EDIT_BLOCKID)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_OFFSET)->EnableWindow(bEnable);
	
}
