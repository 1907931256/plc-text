// GeneralCfgSet.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ConfigSet.h"
#include "GeneralCfgSet.h"


// GeneralCfgSet �Ի���

IMPLEMENT_DYNAMIC(GeneralCfgSet, CDialog)

GeneralCfgSet::GeneralCfgSet(CWnd* pParent /*=NULL*/)
	: CDialog(GeneralCfgSet::IDD, pParent)
{

}

GeneralCfgSet::~GeneralCfgSet()
{
}

void GeneralCfgSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(GeneralCfgSet, CDialog)
END_MESSAGE_MAP()


// GeneralCfgSet ��Ϣ�������
