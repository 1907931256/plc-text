/*
* Copyright (c) 2006, �㽭�󻪰����Ƽ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�ThreadMutex.h
* �ļ���ʶ��������������
* ժҪ���ṩ��������װ��Ϊ�ϲ��ṩƽ̨�޹ص������ࡣ
*		ʹ��ʱ��ֻ��Ҫ����Ҫ�����ĵط�����һ����̬����
*		�Ϳ����ˣ����캯���Զ���ͼ���������ʹ�����
*		���������Զ��ͷ������ͷ�ʹ����Դ��
*
*
* ��ʷ��������2006��7��26�գ�������(li_mingjiang@dahuatech.com)
*
*/
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREADMUTEX_H__51105975_377E_4223_970C_574251793AE2__INCLUDED_)
#define AFX_THREADMUTEX_H__51105975_377E_4223_970C_574251793AE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(WIN32)||defined(WINCE)
//#include <windows.h>
	#include <winsock2i.h>
#else
	#include <pthread.h> 

	#ifdef LINUX
		#include <AX_Atomic.h>
	#else
		#include <asm/asm-armnommu/atomic.h>
	#endif
#endif

//#include <IAddRefAble.h>
#include <AX_IAddRefAble.h>

//class CNewMutex:public IAddRefAble
class CNewMutex:public AX_IAddRefAble
{
public:
	CNewMutex();
	virtual ~CNewMutex();

	virtual int lock(void) = 0;
	virtual int unlock(void) = 0;
};

class CNullMutex : public CNewMutex
{
public:
	CNullMutex();
	virtual ~CNullMutex();

	virtual int lock(void);
	virtual int unlock(void);
};


class CThreadMutex : public CNewMutex
{
public:
	CThreadMutex (void);
	virtual ~CThreadMutex (void);

	virtual int lock(void);
	virtual int unlock(void);
#ifdef WIN32
	CRITICAL_SECTION mutex_;
#else
	pthread_mutex_t mutex_;
#endif
};


#endif // !defined(AFX_THREADMUTEX_H__51105975_377E_4223_970C_574251793AE2__INCLUDED_)
