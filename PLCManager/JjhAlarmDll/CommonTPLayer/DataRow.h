/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：TPTypedef.h
* 文件标识：参见配置管理计划书
* 摘　　要：传输层预定义
*
* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年4月28日

*
* 取代版本：1.0
* 原作者　：
* 完成日期：
* 修订记录：
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
