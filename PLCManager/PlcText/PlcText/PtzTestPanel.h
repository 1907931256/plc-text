#pragma once

#define WM_PTZOPERATIONTEST (WM_USER+301)
// CPtzTestPanel 对话框
typedef struct {
 int nGroup;
 int nIndex;
 int cmdIndex;
 int type;
}PtzTestParam;

//char* strPtzCmd[16]={"WIPER"/*, "ZOOM"*/,"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE",""};


class CPtzTestPanel : public CDialog
{
	DECLARE_DYNAMIC(CPtzTestPanel)

public:
	CPtzTestPanel(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPtzTestPanel();

// 对话框数据
	enum { IDD = IDD_DIALOG_TESTPANEL };
public:
	void buttonDownProc(MSG* pMsg);
	void buttonUpProc(MSG* pMsg);
	void ptzParamGet();
	HWND fHandle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	PtzTestParam mPtzOpeParam;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
