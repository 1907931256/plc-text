
// PlcServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPlcServerApp:
// �йش����ʵ�֣������ PlcServer.cpp
//

class CPlcServerApp : public CWinAppEx
{
public:
	CPlcServerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPlcServerApp theApp;