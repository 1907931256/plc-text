
#include "qdatetime.h"
#include "ITPObject.h"

QAtomicInt ITPObject::_newClientId = 0;

ITPObject::ITPObject(ITPListener* instance, int engineId)
{
	this->_listener = instance;

	_buffer = new char[BUF_SIZE];
	_localBuffer = 1;

    _socket = INVALID_SOCKET;
    _ip     = INADDR_ANY;
    _port   = 0;

	_localPort = 0;
	_localIp = INADDR_ANY;

	_engineId = engineId;
	_sequenceNo = 0;

	_recvBuffSize = 64 * 1024; //16 * 1024;
	_sendBufferSize = 64 * 1024; //16 * 1024;

	_timeout.tv_usec = 1;
	_timeout.tv_sec = 0;

	_nodelay = 0;

	_tpRecvBuffSize = BUF_SIZE;
	_sendQueueThreshold = 0;
	_lastAnouncedThreshold = 0;

	_newTimerId = 0;
}

ITPObject::~ITPObject()
{
#ifdef WIN32
	OutputDebugStringA("ITPObject:destroyed\n");
#endif
	if(_buffer != NULL)
	{
		delete []_buffer;
		_buffer = NULL;
	}

	for(CONN_MAP::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		client_list *conn = it->second;
		CloseClient(conn->socket);
		delete conn;
	}

	_clients.clear();

	int queueSize = _queue.size();
	for(int i=0; i<queueSize; i++)
	{
		DataRow *data = _queue.front();
		_queue.pop();
		data->release();
	}

	//clear queueJitt
	Queue_Jitt::iterator itJitt = _queueJitt.begin();
	while (itJitt != _queueJitt.end())
	{
		Queue_List* ql = itJitt->second;
		if (ql != NULL)
		{
			int queueSize = ql->size();
			for(int i=0; i<queueSize; i++)
			{
				DataRow *data = ql->front();
				ql->pop();
				data->release();
			}

			delete ql;
		}

		itJitt++;
	}
	_queueJitt.clear();

	//clear timer
	TIMER_MAP::iterator itTimer = _timerMap.begin();
	while (itTimer != _timerMap.end())
	{
		tp_timer* tmp = itTimer->second;
		delete tmp;

		itTimer++;
	}
	_timerMap.clear();
}
int ITPObject::clearSendBuffer()
{
	_mutex->lock();

	int queueSize = _queue.size();
	for(int i=0; i<queueSize; i++)
	{
		DataRow *data = _queue.front();
		_queue.pop();

		if(data != NULL)
			data->release();
	}

	_mutex->unlock();
	return 0;
}
void ITPObject::Startup(void)
{
#ifdef WIN32
	WORD version_requested = MAKEWORD (2, 2);
	WSADATA wsa_data;
	int error = WSAStartup (version_requested, &wsa_data);
#endif
}

void ITPObject::Cleanup(void)
{
#ifdef WIN32
	WSACleanup ();
#endif
}

int ITPObject::Close(void)
{
	return 0;
}
	
int ITPObject::CloseClient(int id)
{
    return TP_ERROR_UNSUPPORT;
}

int ITPObject::Listen(char* ip, int port)
{
    return TP_ERROR_UNSUPPORT;
}
	
int ITPObject::Connect(const char* ip, int port)
{
	return TP_ERROR_UNSUPPORT;
}		

int ITPObject::Connect(const char* localIp, int localPort, const char* remoteIp, int remotePort)
{
	return TP_ERROR_UNSUPPORT;
}
        
void ITPObject::SetListener(ITPListener* listener)
{
	assert(listener != NULL);
	this->_listener = listener;
}

int ITPObject::getSequence(void)
{
/*	if (_sequenceNo++ >= 65535)
	{
		_sequenceNo = 0;
	}

	return _sequenceNo;
	*/
	return (int)++_sequenceNo;
}

int ITPObject::SetSocketBufferSize(TPType type, int size)
{
	_mutex->lock();

	int ret = 0;

	if(size >= 0)
	{
		switch(type)
		{
		case TP_SEND:
			_sendBufferSize = size;
			break;
		case TP_RECEIVE:
			_recvBuffSize = size;
			break;
		default:
			ret = -1;
		}
	}
	else
	{
		ret = -2;
	}

	_mutex->unlock();
	return ret;
}

int ITPObject::GetSocketBufferSize(TPType type)
{
	/*int optval = -1;
	int int_len = sizeof(int_len);
	if (getsockopt(_socket, SOL_SOCKET, SO_RCVBUF,&optval, (socklen_t*)&int_len) == -1) {
			printf("Error when getting socket option.\n");
		}
	else
	{
		printf("rcv buffer size=%d\n", optval);
	}
*/
	switch(type)
	{
	case TP_SEND:
		return _sendBufferSize;
	case TP_RECEIVE:
		return _recvBuffSize;
	default:
		return -1;
	}
}

int ITPObject::SetSelectTimeout(long sec, long usec)
{
	_mutex->lock();
	int ret = 0;

	if(sec >=0 && usec >= 0)
	{
		_timeout.tv_usec = usec;
		_timeout.tv_sec = sec;
	}
	else
	{
		ret = -1;
	}

	_mutex->unlock();
	return ret;
}

int ITPObject::SetRecvTPBuffSize(int size)
{
	_mutex->lock();

	int ret = 0;

	if(size >0 && size < 1024*1024)
	{
		_tpRecvBuffSize = size;
		char* p = new char[_tpRecvBuffSize];

		if (NULL != p)
		{
			delete _buffer;
			_buffer = p;
			ret = 0;
		}
		else
		{
			ret = -2;
		}
	}
	else
	{
		ret = -1;
	}

	_mutex->unlock();
	return ret;
}

int ITPObject::SetTPRecvBuffer(char* buff, int size)
{
	_mutex->lock();

	if (_localBuffer == 1)
	{
		delete _buffer;
		_localBuffer = 0;
	}


	_buffer = buff;
	_tpRecvBuffSize = size;

	_mutex->unlock();

	return 0;
}

client_list* ITPObject::GetConnection(int id)
{
	client_list* conn = _clients[(unsigned int)id];

	if (conn == NULL)
	{
		return NULL;
	}
	else
		return conn;
}

int ITPObject::SetConnection(client_list* conn)
{
	client_list* tmp = _clients[conn->id];
	if (tmp == NULL)
	{
		client_list *newConn = new client_list;
	//	memcpy(newConn, conn, sizeof(newConn));
		newConn->id		= conn->id;
		newConn->ip		= conn->ip;
		newConn->port	= conn->port;
		newConn->socket = conn->socket;
		_clients[conn->id] = newConn;
	}
	else
	{
		return -1;
	}
	return 0;
}

DataRow* ITPObject::createDataRow()
{
	return _dataRowPool.CreateDataRow();
}

int ITPObject::SetNodelayFlag(int flag)
{
	_mutex->lock();
	_nodelay = flag;
	_mutex->unlock();
	return 0;
}

long ITPObject::GetNewClientId()
{
	_newClientId.ref();
    return _newClientId;
}


int ITPObject::SetSendQueueThreshold(int threshold)
{
	_mutex->lock();
	_sendQueueThreshold = threshold>=0 ? threshold : _sendQueueThreshold;
	_mutex->unlock();

	return 0;
}

Queue_List* ITPObject::GetSendQueue(int connId)
{
	return &_queue;
}


long ITPObject::SchedureTimer(int delay, int context)
{
	if (delay < 0)
	{
		return -1;
	}

	_mutex->lock();
	long newTimerId = this->getTimerId();
	tp_timer* newTimer = new tp_timer;
	newTimer->delay = delay;
	newTimer->interval = 0;
	newTimer->repeat = 0;	//no repeat
	newTimer->context = context;
	newTimer->timeBegin = QDateTime::currentMSecsSinceEpoch();
	newTimer->lastTimeout = 0;

	_timerMap[newTimerId] = newTimer;
	_mutex->unlock();

	return newTimerId;
}

long ITPObject::SchedureRepeatTimer(int interval, int delay, int context)
{
	if (interval < 0 || delay < 0)
	{
		return -1;
	}

	_mutex->lock();
	long newTimerId = this->getTimerId();
	tp_timer* newTimer = new tp_timer;
	newTimer->delay = delay*1000;
	newTimer->interval = interval*1000;
	newTimer->repeat = 1;	//repeat
	newTimer->context = context;
	newTimer->timeBegin = QDateTime::currentMSecsSinceEpoch();
	newTimer->lastTimeout = 0;

	_timerMap[newTimerId] = newTimer;
	_mutex->unlock();

	return newTimerId;
}

int ITPObject::CancelTimer(long timerId)
{
	_mutex->lock();
	TIMER_MAP::iterator it = _timerMap.find(timerId);
	if (it == _timerMap.end())
	{
		_mutex->unlock();
		return -1;
	}

	tp_timer* tmp = it->second;
	_timerMap.erase(it);
	delete tmp;

	_mutex->unlock();
	return 0;
}

long ITPObject::getTimerId(void)
{
	_newClientId.ref();
    return _newClientId;
}

int ITPObject::checkTimer()
{
	if (_listener != NULL)
	{
		int64 timeNow = _newClientId;
		std::vector<timeout_cb_param> tempVec;	//临时存储已超时需要回调的参数，在出锁之后再一一回调，从而避免死锁的可能。
		//printf("getTime = %d \n", timeNow);
		_mutex->lock();
		TIMER_MAP::iterator it = _timerMap.begin();
		while (it != _timerMap.end())
		{
			tp_timer* tm = it->second;
			if ((tm->lastTimeout != 0 && timeNow - tm->lastTimeout >= tm->interval)
				|| (tm->lastTimeout == 0 && timeNow - tm->timeBegin >= tm->delay))
			{
				timeout_cb_param temp = {0};
				temp.id = it->first;
				temp.context = tm->context;
				tempVec.push_back(temp);

				if (tm->repeat == 0)
				{
					//cancel it
					_timerMap.erase(it++);
					delete tm;
					continue;
				}

				tm->lastTimeout = timeNow;
			}
			it++;
		}
		_mutex->unlock();

		for (std::vector<timeout_cb_param>::iterator it = tempVec.begin(); it != tempVec.end(); it++)
		{
			timeout_cb_param& temp = (*it);

			int ret = _listener->onTimeout(temp.id, temp.context);
			if (ret == 1)
			{
				//cancel it
				_mutex->lock();
				TIMER_MAP::iterator it = _timerMap.find(temp.id);
				if (it != _timerMap.end())
				{
					tp_timer* tm = it->second;
					_timerMap.erase(it);
					delete tm;
				}
				_mutex->unlock();
			}
		}
	}

	return 0;
}

int ITPObject::fillFds(int& maxfd, fd_set& readfds, fd_set& writefds)
{
	if(INVALID_SOCKET!=_socket)
	{
		FD_SET(_socket, &readfds);
		FD_SET(_socket, &writefds);
		if(_socket>maxfd)
			maxfd=_socket;
	}
	return 0;
}
int ITPObject::dealFDResult(int& fds,fd_set& readfds, fd_set& writefds,bool& fdsChange)
{
	if(INVALID_SOCKET!=_socket && 0<fds)
	{
		if(0<fds && FD_ISSET(_socket,&readfds))
			--fds;
		if(0<fds && FD_ISSET(_socket,&writefds))
			--fds;
	}
	return 0;
}
