#pragma once
// CColorListBox

class CColorListBox : public CListBox
{
	DECLARE_DYNAMIC(CColorListBox)

public:
	CColorListBox();
	virtual ~CColorListBox();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);

	int AddString(LPCTSTR lpszItem, COLORREF itemColor = RGB(0,0,0));
	void RefushHorizontalScrollBar( void );
	int InsertString( int nIndex, LPCTSTR lpszItem );
};
