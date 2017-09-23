
// PlcText.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "PlcText.h"
#include "PlcTextDlg.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void SetupExceptionHandler()
{	
	BT_SetAppName(_T("Mago PlcText"));
	BT_SetActivityType(BTA_SAVEREPORT);
	TCHAR szReportPath[1024] = {0};
	GetAppPathW(szReportPath,1024);
	BT_SetReportFilePath(szReportPath);
	BT_SetFlags(BTF_DETAILEDMODE |BTF_RESTARTAPP);
	BT_InstallSehFilter();
}
// CPlcTextApp

BEGIN_MESSAGE_MAP(CPlcTextApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPlcTextApp ����

CPlcTextApp::CPlcTextApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPlcTextApp ����

CPlcTextApp theApp;


// CPlcTextApp ��ʼ��

BOOL CPlcTextApp::InitInstance()
{
	SetupExceptionHandler();
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CPlcTextDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
