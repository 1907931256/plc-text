
// PlcText.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPlcTextApp:
// �йش����ʵ�֣������ PlcText.cpp
//

class CPlcTextApp : public CWinAppEx
{
public:
	CPlcTextApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPlcTextApp theApp;