#include "TPTCPClient.h"

TPTCPClient::TPTCPClient(ITPListener * tcpclientapp, int engineId)
	:ITPObject(tcpclientapp, engineId)
{
	_mutex = new QMutex(QMutex::Recursive);

	_maxDataQueueLength = 0;
}

//TPTCPClient::TPTCPClient(int engineId, ITPListener *tcpclientapp, base::internal::LockImpl* mutex):ITPObject(tcpclientapp, engineId)
//{
//	if (mutex != NULL)
//		_mutex = mutex;
//	else
//		_mutex = new base::internal::LockImpl();
//
//	_maxDataQueueLength = 0;
//}

TPTCPClient::~TPTCPClient()
{
	Close();

	if (_mutex)
	{
		delete _mutex;
		_mutex = NULL;
	}
}

int TPTCPClient::Connect(const char* ip, int port)
{
	_mutex->lock();

    _ip = inet_addr(ip);
    _port = htons(port);

	if (INVALID_SOCKET == _socket)
	{
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (_socket == SOCKET_ERROR)
		{
            //int i = GetLastError();
			return -1;
		}

		if(/*(_localIp != INADDR_ANY) &&*/(_localPort != 0))
		{
		    struct sockaddr_in local_addr;
			memset(&local_addr, 0, sizeof(local_addr));
			local_addr.sin_family = AF_INET;
			local_addr.sin_port = _localPort;
			local_addr.sin_addr.s_addr = _localIp;//MY IP
   
			if (INVALID_SOCKET == bind(_socket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) )
			{
				printf("bind local address error\n");
				closeInside();
				_mutex->unlock();
                //int i = GetLastError();
				return INVALID_LOCAL_ADDRESS;
			}
		}
	}

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = _ip;
    my_addr.sin_port = _port;

    //将socket设置为非阻塞
	int flags = 0;
#ifdef WIN32
	unsigned long l = 1;
	int n = ioctlsocket(_socket, FIONBIO, &l);
	if (n != 0)
	{
		int errcode = WSAGetLastError();
		_mutex->unlock();
		//return errcode;
		return TP_ERROR_SET_NOBLOCKING_FAILED;
	}
#else
	if ((flags = fcntl(_socket, F_GETFL, 0)) == -1)
	{
		printf("fcntl(F_GETFL, O_NONBLOCK)");

		_mutex->unlock();
		return TP_ERROR_SET_NOBLOCKING_FAILED;
	}

	if (fcntl(_socket, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		printf("fcntl(F_SETFL, O_NONBLOCK)");
		_mutex->unlock();
		return TP_ERROR_SET_NOBLOCKING_FAILED;
	}

#endif


	if(_nodelay == 1)
	{
		const int optval = 1;
#ifdef WIN32
		if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof(optval)) < 0)
#else
		if (setsockopt(_socket, SOL_SOCKET, TCP_NODELAY, (char *)&optval, sizeof(optval)) < 0)
#endif
		{
			printf("error setsockopt nodelay");
		}
	}

	if(_recvBuffSize > 0)
		setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&_recvBuffSize, sizeof(_recvBuffSize));
	if(_sendBufferSize > 0)
		setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&_sendBufferSize, sizeof(_sendBufferSize));

    //connect
    int ret = connect(_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

    if(INVALID_SOCKET == ret)
    {
        fd_set wds;
        FD_ZERO(&wds);
        FD_SET(_socket,&wds);

		if (_socket<0)
		{
			int i =0;
		}

		timeval timeo;
		timeo.tv_sec = _timeout.tv_sec;
		timeo.tv_usec = _timeout.tv_usec;
        
        int iRet = select(_socket + 1, NULL, &wds, NULL, &timeo);
        
        if(iRet>0 && FD_ISSET(_socket, &wds))
        {
			int error = -1;
			int llen = sizeof(int);
#ifdef WIN32
			getsockopt(_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &llen);
#else
			getsockopt(_socket, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&llen);
#endif

			if(error == 0)
				ret = _socket;
			else
			{
				ret = INVALID_SOCKET;
				closeInside();
			}

			_mutex->unlock();
			return ret;
        }
        else
        {
            closeInside();
			_mutex->unlock();
            return INVALID_SOCKET;
        }
    }

	_mutex->unlock();

    return _socket;
}

int TPTCPClient::Connect(const char* localIp, int localPort, const char* remoteIp, int remotePort)
{
	_mutex->lock();

    _localIp = inet_addr(localIp);
    _localPort = htons(localPort);

	_mutex->unlock();
	return Connect(remoteIp, remotePort);
}

int TPTCPClient::Close()
{
	_mutex->lock();

    int iRet = closeInside();

	_mutex->unlock();
    return iRet;
}

int TPTCPClient::closeInside()
{
	_mutex->lock();

	int iRet = 0;

	if(_socket != INVALID_SOCKET)
	{
		iRet = closesocket(_socket);
		_socket = INVALID_SOCKET;
	}

	int queueSize = _queue.size();
	for(int i=0; i<queueSize; i++)
	{
		DataRow *data = _queue.front();
		_queue.pop();
		data->release();
	}
	_mutex->unlock();

	return iRet;
}

int TPTCPClient::Heartbeat()
{
	int retValue = TP_NORMAL_RET;
	_mutex->lock();

    if(INVALID_SOCKET == _socket)
    {
		retValue = TP_ERROR_BASE;
		_mutex->unlock();
		this->checkTimer();
		return retValue;
    }

	int iRevLen = 0;
	int queueSize = 0;
    
    fd_set readfds;
	fd_set writefds;

    FD_ZERO(&readfds);
	FD_ZERO(&writefds);
    FD_SET(_socket,&readfds);

	if(_queue.size() > 0)
		FD_SET(_socket,&writefds);

	struct timeval timeo;
	timeo.tv_sec = _timeout.tv_sec;
	timeo.tv_usec = _timeout.tv_usec;

    int fdnums = select(_socket + 1, &readfds, &writefds, NULL, &timeo);
    
	if(fdnums > 0)
	{
		//处理读事件
		if(fdnums>0 && FD_ISSET(_socket,&readfds))
		{
			fdnums--;	//将处理掉的事件减去

			iRevLen = recv(_socket,_buffer,_tpRecvBuffSize, 0);
			if(iRevLen <= 0)
			{
				//是先通知上层还是先关闭需要再研究,因为可能会影响上层应用程序.
				if(_listener != NULL)
				{
					_mutex->unlock();
					_listener->onClose(_engineId, _socket);
					_mutex->lock();
				}
				closeInside();

				retValue = TP_NORMAL_RET;
				goto _out;
			}
			else
			{
				if(_listener != NULL)
				{
					//_mutex->unlock();
					this->_listener->onData(_engineId, _socket,_buffer,iRevLen);
					//_mutex->lock();
				}
			}
		}

		//处理写事件
		if(fdnums>0 && FD_ISSET(_socket, &writefds))
		{
			int stop = _queue.size();
			for(int i=0; i < stop; i++)
			{
				DataRow* data = _queue.front();

				int iSend = sendInside(data->id, data->data, data->len);
				if (iSend < 0)
				{
					//连接应该已断，这里不用处理，下次heartbeat会检测到断线。
					break;
				}
				else
				{
					if (iSend < data->len)
					{
					//	_sendStatistic++;
						//未全部发送成功
						//_mutex->unlock();
						int ret = _listener->onSendDataAck(_engineId, data->id, data->sequence, iSend);
						//_mutex->lock();

						//根据回调的返回值做不同处理
						if (0 == ret)
						{
							//继续投递请求，直至全部发送成功
							data->len = data->len - iSend;
							data->data = data->data + iSend;
						}
						else if (1 == ret)
						{
							//清空内部缓冲
							queueSize = _queue.size();
							for(int i=0; i<queueSize; i++)
							{
								data = _queue.front();
								_queue.pop();
								data->release();
							}
						}
						break;
					}
					else
					{
					//	_sendStatistic++;
						//发送完成，回调通知上层
						if(_listener != NULL)
						{
							//_mutex->unlock();
							_listener->onSendDataAck(_engineId, data->id, data->sequence, 0);
							//_mutex->lock();
						}
					}
				}

				//delete data;
				data->release();
				_queue.pop();
			}
		}
	}
	else if(fdnums < 0) //有错误发生，错误的原因有非常多，需要由应用层决定如何处理
	{
	    printf("client select error\n");
		retValue = TP_ERROR_BASE;
	}
    else if(fdnums == 0) //无事件
    {
        retValue = TP_NOTHING_IS_DONE_RET;
    }

	//检查是否需要回调内部状态
	queueSize = _queue.size();
	if (_sendQueueThreshold > 0 && abs(queueSize-_lastAnouncedThreshold) > _sendQueueThreshold)
	{
		//通知上层缓冲长度已过了阀值
		//_mutex->unlock();
		_listener->onSendStatus(_engineId, 0, ss_sendQueueThreshold, queueSize);
		_lastAnouncedThreshold = queueSize;
		//_mutex->lock();
	}

_out:
	_mutex->unlock();

	this->checkTimer();

	return retValue;
}

int TPTCPClient::SetMaxDataQueueLength(int maxDataQueueLength)
{
	_maxDataQueueLength = maxDataQueueLength;
	return 0;
}

int TPTCPClient::Send(int id, const char *pBuf, unsigned int iBufLen)
{
	int ret = 0;

	_mutex->lock();
	if (_maxDataQueueLength > 0)
	{
		//检查是否已到达最大缓冲长度
		if (_queue.size() >= _maxDataQueueLength)
		{
			_mutex->unlock();
			return -1;
		}
	}

	DataRow* row = createDataRow();
	row->AddRef();

	row->partDataSent=0;
	row->id = id;
	row->data = pBuf;
	row->len = iBufLen;
	row->socket = _socket;
	row->sequence = getSequence();

	_queue.push(row);
	
	ret = row->sequence;

	_mutex->unlock();


	return ret;
}

inline int TPTCPClient::sendInside(int id, const char *pBuf, unsigned int iBufLen)
{
    if (INVALID_SOCKET == _socket)
    {
        printf("socket invalid\n");
        return TP_ERROR_BASE;
    }

    if ((0 == iBufLen) || (NULL == pBuf))
    {
        return 0;
    }

    int iSend = send(_socket, pBuf, iBufLen, 0);

	return iSend;
	/*
    unsigned int iSended = 0;

    while (iSended < iBufLen)
    {
        iSend = send(_socket, pBuf+iSended, iBufLen-iSended, 0);

        if (iSend < 0)
        {
            break;
        }

        iSended += iSend;

        continue;
    }

    return (iBufLen);
	*/
}

int TPTCPClient::dealFDResult(int& fds,fd_set& readfds, fd_set& writefds,bool& fdsChange)
{
	int dealCount = 0;
	_mutex->lock();
	if(INVALID_SOCKET!=_socket && 0<fds)
	{
		bool needRead=false;
		bool needWrite=false;
		if(0<fds && FD_ISSET(_socket,&readfds))
		{
			--fds;
			needRead=true;
		}
		if(0<fds && FD_ISSET(_socket,&writefds))
		{
			--fds;
			needWrite=true;
		}

		//处理读事件
		if(needRead)
		{
			++dealCount;

			int iRevLen = recv(_socket,_buffer,_tpRecvBuffSize, 0);
			if(iRevLen <= 0)
			{
				//是先通知上层还是先关闭需要再研究,因为可能会影响上层应用程序.
				if(_listener != NULL)
				{
					_mutex->unlock();
					_listener->onClose(_engineId, _socket);
					_mutex->lock();
				}
				closeInside();
				fdsChange=true;


				goto _out;
			}
			else
			{
				if(_listener != NULL)
				{
					//_mutex->unlock();
					this->_listener->onData(_engineId, _socket,_buffer,iRevLen);
					//_mutex->lock();
				}
			}
		}

		//处理写事件
		if(needWrite)
		{
			while(!_queue.empty())
			{
				++dealCount;
				DataRow* data = _queue.front();

				int iSend = sendInside(data->id, data->data, data->len);
				if (iSend < 0)
				{
					//连接应该已断，这里不用处理，下次heartbeat会检测到断线。
					break;
				}
				else
				{
					if (iSend < data->len)
					{
						//	_sendStatistic++;
						//未全部发送成功
						//_mutex->unlock();
						int ret = _listener->onSendDataAck(_engineId, data->id, data->sequence, iSend);
						//_mutex->lock();

						//根据回调的返回值做不同处理
						if (0 == ret)
						{
							//继续投递请求，直至全部发送成功
							data->len = data->len - iSend;
							data->data = data->data + iSend;
						}
						else if (1 == ret)
						{
							//清空内部缓冲							
							while(!_queue.empty())
							{
								data=_queue.front();
								_queue.pop();
								data->release();
							}
						}
						break;
					}
					else
					{
						//	_sendStatistic++;
						//发送完成，回调通知上层
						if(_listener != NULL)
						{
							//_mutex->unlock();
							_listener->onSendDataAck(_engineId, data->id, data->sequence, 0);
							//_mutex->lock();
						}
					}
				}

				//delete data;
				data->release();
				_queue.pop();
			}
		}
	}
_out:
	_mutex->unlock();
	return dealCount;
}
