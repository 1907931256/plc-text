
// ConfigSet.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CConfigSetApp:
// �йش����ʵ�֣������ ConfigSet.cpp
//

class CConfigSetApp : public CWinAppEx
{
public:
	CConfigSetApp();

// ��д
	public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CConfigSetApp theApp;