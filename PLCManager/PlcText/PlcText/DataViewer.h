#pragma once

#include <vector>
#include "afxcmn.h"

#define DATA_TYPPE_BOOL 0
#define DATA_TYPE_INT 1
#define BLOCK_SIZE_COPY 256
#define WM_DATASHOW (WM_USER+100)
#define WM_DATAVIEWER_CLOSE (WM_USER+101)

// DataViewer 对话框
typedef struct{
	CString time;
	CString address;
	CString type;
	CString value;
}dataShow;
class DataViewer : public CDialog
{
	DECLARE_DYNAMIC(DataViewer)

public:
	DataViewer(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DataViewer();

public:
	typedef struct{
		int addressStart;
		int addressEnd;
		int dataType;
		int db;
	}showInfo;
	std::vector<showInfo> dataInfo;
	CCriticalSection dataSection;
	afx_msg LRESULT OnDataShow(WPARAM wparam,LPARAM lparam);
	virtual BOOL OnInitDialog();
	struct BitField
	{
		bool a:1;
		bool b:1;
		bool c:1;
		bool d:1;
		bool e:1;
		bool f:1;
		bool g:1;
		bool h:1;
	};
	union Union
	{
		struct BitField bf;
		unsigned char n;
	};
	Union dataS;
	void showBoolData(int db,int& count,int address);
	// 对话框数据
	enum { IDD = IDD_DIALOG_DATA };
	HWND fHandle;
	static UINT WINAPI dataShowThread(LPVOID pParam);
	UINT dataShowMain(char* pBuffer);
	char dataBufferG[BLOCK_SIZE_COPY];
	CCriticalSection BufferLock;
	CString timeGetStr;
	void timeGet(CString& timeStr);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	std::vector<dataShow>m_database;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl mList;
	afx_msg void OnClose();
};
