// ColorListBox.cpp : 实现文件

#include "stdafx.h"
//#include "ListBoxtest.h"
#include "fontListBox.h"
// CColorListBox

IMPLEMENT_DYNAMIC(CColorListBox, CListBox)

CColorListBox::CColorListBox()
{

}

CColorListBox::~CColorListBox()
{
}

BEGIN_MESSAGE_MAP(CColorListBox, CListBox)
END_MESSAGE_MAP()
// CColorListBox 消息处理程序
void CColorListBox::DrawItem(LPDRAWITEMSTRUCT  lpDrawItemStruct)
{
	// TODO:  添加您的代码以绘制指定项
	if (lpDrawItemStruct->itemID == -1) 
	{
		DrawFocusRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);
		return;
	}
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	COLORREF clrOld;
	CString sText;
	// get color info from item data
	COLORREF clrNew = (COLORREF)(lpDrawItemStruct->itemData);
	// item selected ?
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush brush(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush);
	}
	// item deselected ?
	if (!(lpDrawItemStruct->itemState & ODS_SELECTED) &&	
		(lpDrawItemStruct->itemAction & ODA_SELECT))
	{
		CBrush brush(::GetSysColor(COLOR_WINDOW));
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush);
	}	 	
	// item has focus ?
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && 
		(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem); 
	}
	// lost focus ?
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) &&	
		!(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem); 
	}
	// set the background mode to TRANSPARENT
	int nBkMode = pDC->SetBkMode(TRANSPARENT);

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		clrOld = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	else 
		if (lpDrawItemStruct->itemState & ODS_DISABLED)
			clrOld = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			clrOld = pDC->SetTextColor(clrNew);

	// get item text
	GetText(lpDrawItemStruct->itemID, sText);
	CRect rect = lpDrawItemStruct->rcItem;

	// text format
	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;

	// draw the text
	pDC->DrawText(sText, -1, &rect, nFormat);

	// restore old values
	pDC->SetTextColor(clrOld); 
	pDC->SetBkMode(nBkMode);
}


void CColorListBox::MeasureItem(LPMEASUREITEMSTRUCT  lpMeasureItemStruct)
{

	// TODO:  添加您的代码以确定指定项的大小
	lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK);
}

int CColorListBox::AddString(LPCTSTR lpszItem, COLORREF itemColor)
{
	// Add the string to the list box
	int nIndex = CListBox::AddString(lpszItem);
	// save color data
	if (nIndex >= 0)
		SetItemData(nIndex, itemColor);

	RefushHorizontalScrollBar();
	return nIndex;
}

void CColorListBox::RefushHorizontalScrollBar( void )
{
	CDC *pDC = this->GetDC();
	if ( NULL == pDC )
	{
		return;
	}



	int nCount = this->GetCount();
	if ( nCount < 1 )
	{
		this->SetHorizontalExtent( 0 );
		return;
	}



	int nMaxExtent = 0;
	CString szText;
	for ( int i = 0; i < nCount; ++i )
	{
		this->GetText( i, szText );
		CSize &cs = pDC->GetTextExtent( szText );
		if ( cs.cx > nMaxExtent )
		{
			nMaxExtent = cs.cx;
		}
	}
}

int CColorListBox::InsertString( int nIndex, LPCTSTR lpszItem )
{
	int nResult = CListBox::InsertString( nIndex, lpszItem );
	RefushHorizontalScrollBar();
	return nResult;
}
