#include "DataRow.h"

DataRow::DataRow(CDataRowPool* pool)
{
	this->_pool = pool;
	partDataSent=0;
}


int DataRow::SetPool(CDataRowPool* pool)
{
	this->_pool = pool;
	return 0;
}

int DataRow::release()
{
	if (RefCountedBase::Release()) {
		if(_pool != NULL)
		{
			partDataSent=0;
			_pool->Recycle(this);
		}
		else
		{
			delete this;
		}
		return 0;
	}

	return 1;
}

//data row pool

CDataRowPool::CDataRowPool()
{

}

CDataRowPool::~CDataRowPool()
{
	_mutex.lock();
	int size = _pool.size();
	for (int i=0; i<size; i++)
	{
		DataRow* dataRow = _pool.front();
		_pool.pop_front();
		delete dataRow;
		//这里还有没有必要releast？
	}
	_mutex.unlock();
}

DataRow* CDataRowPool::CreateDataRow()
{
	DataRow* dataRow = NULL;

	_mutex.lock();
	if (_pool.size() == 0)
	{
		dataRow = new DataRow(this);
		dataRow->SetPool(this);
		//printf("new CRTPPacket! ++! \n");
	}
	else
	{
		dataRow = _pool.front();
		_pool.pop_front();
	}

	_mutex.unlock();

	return dataRow;
}

int CDataRowPool::Recycle(DataRow* packet)
{
	_mutex.lock();
	_pool.push_back(packet);
	_mutex.unlock();

	return 0;
}