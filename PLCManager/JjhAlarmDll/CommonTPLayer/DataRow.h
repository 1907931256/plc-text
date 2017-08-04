/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPTypedef.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ�������Ԥ����
*
* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��4��28��

*
* ȡ���汾��1.0
* ԭ���ߡ���
* ������ڣ�
* �޶���¼��
*/
#ifndef _DATAROW_H_
#define _DATAROW_H_

//#include <IAddRefAble.h>
#include <AX_IAddRefAble.h>
#include <ThreadMutex.h>
#include <deque>

class DataRow;

typedef std::deque<DataRow*> DataRow_Queue;

class CDataRowPool;

//class DataRow : public IAddRefAble
class DataRow : public AX_IAddRefAble
{
public:
	int len;
	unsigned int id;
	int socket;
	int sequence;
	int partDataSent;

	char* data;

	int SetPool(CDataRowPool* pool);

public:
	friend class CDataRowPool;
	virtual int release();
protected:

	DataRow(CDataRowPool* pool);
	CDataRowPool* _pool;
};

class CDataRowPool
{
public:
	CDataRowPool();
	~CDataRowPool();

	DataRow*			CreateDataRow();

	//user should not call this function
	int					Recycle(DataRow* dataRow);

protected:
	DataRow_Queue _pool;
	CThreadMutex _mutex;
private:
};


#endif
