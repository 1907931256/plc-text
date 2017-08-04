#if !defined(AFX_EXTENDLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_) 
#define AFX_EXTENDLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_ 
 
#if _MSC_VER >= 1000 
#pragma once 
#endif // _MSC_VER >= 1000 
 
// ExtendListBox.h : header file 
 
//------------------------------------------------------------------- 
// 
//	CExtendListBox class -  
//		A CListBox-derived class with optional colored items. 
// 
//		Version: 1.0	01/10/1998 Copyright ?Patrice Godard 
// 
//		Version: 2.0	09/17/1999 Copyright ?Paul M. Meidinger 
// 
//------------------------------------------------------------------- 
 
///////////////////////////////////////////////////////////////////////////// 
// CExtendListBox window 
 
class CExtendListBox : public CListBox 
{ 
// Construction 
public: 
	CExtendListBox(); 
 
// Attributes 
public: 
 
  virtual BOOL IsItemEnabled(UINT) const; 
// Operations 
public: 
	int AddString(LPCTSTR lpszItem);									// Adds a string to the list box 
	int AddString(LPCTSTR lpszItem, COLORREF rgb);						// Adds a colored string to the list box 
	int InsertString(int nIndex, LPCTSTR lpszItem);						// Inserts a string to the list box 
	int InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb);		// Inserts a colored string to the list box 
	void SetItemColor(int nIndex, COLORREF rgb);						// Sets the color of an item in the list box 
  int GetCurSel(); 
// Overrides 
	// ClassWizard generated virtual function overrides 
	//{{AFX_VIRTUAL(CExtendListBox) 
	public: 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct); 
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct); 
	//}}AFX_VIRTUAL 
 
// Implementation 
public: 
  void SetItemEnable(int nIndex,BOOL bEnable); 
	virtual ~CExtendListBox(); 
 
	// Generated message map functions 
protected: 
	//{{AFX_MSG(CExtendListBox) 
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags); 
	//}}AFX_MSG 
 
	DECLARE_MESSAGE_MAP() 
}; 
 
///////////////////////////////////////////////////////////////////////////// 
 
//{{AFX_INSERT_LOCATION}} 
// Microsoft Developer Studio will insert additional declarations immediately before the previous line. 
 
#endif // !defined(AFX_EXTENDLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_) 