#include <Windows.h>
#include "NetUDP.h"

CNetUDP::CNetUDP(void)
{
	m_skClient = INVALID_SOCKET;
	m_skServer = INVALID_SOCKET;
	for (int i = 0; i < MAX_IPADDRESS_NUMBER; ++i)
	{
		m_skSendClient[i] = INVALID_SOCKET;
	}
}

CNetUDP::~CNetUDP(void)
{
	WSACleanup();
	DisConnect();
}

SOCKET CNetUDP::CreateUDPSocket(EUDPTYPE type)
{
	int nRet = -1;
	SOCKET skt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == skt)
	{
		goto END;
	}

	BOOL b = TRUE;
	BOOL ba = TRUE;
	if (0 != ::setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (const char *)&ba, sizeof(ba)))
	{
		goto END;
	}

	switch (type)
	{
	case EUDP_Broadcast:
		{
			if (0 != ::setsockopt(skt, SOL_SOCKET, SO_BROADCAST, (const char *)&b, sizeof(b)))
			{
				goto END;
			}
		}
		break;
	case EUDP_Multcast:
		{
			int mc_loop = 0;
			if( setsockopt(skt, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&mc_loop, sizeof(int) ) < 0 )
			{
				goto END;
			}
		}
		break;
	}
	nRet = 0;

END:
	if (nRet != 0)
	{
		closesocket(skt);
		skt = INVALID_SOCKET;
	}
	return skt;
}

int CNetUDP::SetSendSocket(int nSendPort, const char *szIP, EUDPTYPE type)
{
	int nRet = -1;
	if (m_skClient != INVALID_SOCKET)
	{
		closesocket(m_skClient);
		m_skClient = INVALID_SOCKET;
	}

	m_skClient = CreateUDPSocket(type);
	if (m_skClient == INVALID_SOCKET)
	{
		goto END;
	}
	/*
	//得到本机所有IP地址
	std::list<std::string> IPList;
	WSADATA wd;
	WSAStartup(MAKEWORD(2, 2), &wd);
	struct hostent* pHost = gethostbyname(NULL);
	if(pHost == NULL)
	{
		return nRet;
	}
	for (int i = 0;pHost->h_addr_list[i] != NULL;i++)
	{
		std::string  IPAddress = inet_ntoa(*(struct in_addr*)pHost->h_addr_list[i]);
		IPList.push_back(IPAddress);
	}
	int i = 0;	
	int j = 0;
	std::list<std::string>::iterator it = IPList.begin();
	while((j!=MAX_IPADDRESS_NUMBER) && (it!=IPList.end()))
	{
		const char *szIP = (*it).c_str();
		sockaddr_in addClient; 
		addClient.sin_family = AF_INET;
		addClient.sin_addr.s_addr = inet_addr(szIP);
		SOCKET m_skClient = CreateUDPSocket(type);
		if (m_skClient == INVALID_SOCKET)
		{
			goto END;
		}

		for (i = 7000+j; i < 8000; ++i)
		{
			addClient.sin_port = htons(i);
			if (SOCKET_ERROR != bind(m_skClient, (SOCKADDR*)&addClient, sizeof(addClient)))
			{
				m_skSendClient[j++] = m_skClient;
				break;
			}
		}

		if (i == 8000)
		{
			goto END;
		}
		++it;
	}
	*/
	/*sockaddr_in addClient;
	addClient.sin_family = AF_INET;
	addClient.sin_addr.s_addr = INADDR_ANY;

	int i = 0;
	for (i = 7000; i < 8000; ++i)
	{
		addClient.sin_port = htons(i);
		if (SOCKET_ERROR != bind(m_skClient, (SOCKADDR*)&addClient, sizeof(addClient)))
		{
			break;
		}
	}

	if (i == 8000)
	{
		goto END;
	}*/

	m_sendAddr.sin_family = AF_INET;
	m_sendAddr.sin_port = htons(nSendPort);
	m_sendAddr.sin_addr.s_addr = inet_addr(szIP);
	nRet = 0;
	
END:
	if (nRet != 0)
	{
		DisConnect();
		return INVALID_SOCKET;
	}

	return 0;
}

int CNetUDP::SetRecvSocket(int nRecvPort, const char *szIP, EUDPTYPE type)
{
	int nRet = -1;
	if (m_skServer != INVALID_SOCKET)
	{
		closesocket(m_skServer);
		m_skServer = INVALID_SOCKET;
	}

	m_skServer = CreateUDPSocket(type);
	if (m_skServer == INVALID_SOCKET)
	{
		goto END;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htons(INADDR_ANY);
	service.sin_port = htons(nRecvPort);
	if (SOCKET_ERROR == bind(m_skServer, (SOCKADDR*)&service, sizeof(service)))
	{
		goto END;
	}

	switch (type)
	{
	case EUDP_Multcast:
		{
			struct ip_mreq mreq;
			memset(&mreq, 0, sizeof(struct ip_mreq));
			mreq.imr_multiaddr.s_addr = inet_addr(szIP);	// 设置组地址 
			mreq.imr_interface.s_addr = htons(INADDR_ANY);	// 设置发送组播消息的源主机的地址信息
			
			if (setsockopt(m_skServer, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(struct ip_mreq)) != 0) 
			{
				goto END;
			}

			int mc_loop = 0;
			if( setsockopt(m_skServer, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&mc_loop, sizeof(int) ) != 0)
			{
				goto END;
			}
		}
		break;
	}

	// 设置为非阻塞
	DWORD l = 1;
	if (ioctlsocket(m_skServer, FIONBIO, &l) != 0)
	{
		goto END;
	}
	nRet = 0;
	
END:
	if (nRet != 0)
	{
		DisConnect();
		return INVALID_SOCKET;
	}

	return nRet;
}

int CNetUDP::RecvFrom(char *pReadBuf, int nRSize, sockaddr_in &senderAddr)
{
	if (m_skServer != INVALID_SOCKET)
	{
		int fromLen = sizeof(sockaddr_in);
		return recvfrom(m_skServer, pReadBuf, nRSize, 0, (SOCKADDR *)&senderAddr, &fromLen);
	}
	return 0;
}

int CNetUDP::SendTo(const char *pWriteBuf, int nWSize)
{
	bool bRet = true;
	for(int i = 0; i < MAX_IPADDRESS_NUMBER; ++i)
	{
		bRet &= bRet;
		//SOCKET skClient = m_skSendClient[i];
		SOCKET skClient = m_skClient;
		if (skClient != INVALID_SOCKET)
		{
			bRet = sendto(skClient, pWriteBuf, nWSize, 0, (SOCKADDR *)&m_sendAddr, sizeof(m_sendAddr));
		}
	}
	return bRet;
	/*if (m_skClient)
	{
		return sendto(m_skClient, pWriteBuf, nWSize, 0, (SOCKADDR *)&m_sendAddr, sizeof(m_sendAddr));
	}*/

	return 0;
}

void CNetUDP::DisConnect()
{
	if (INVALID_SOCKET != m_skClient)
	{
		closesocket(m_skClient);
		m_skClient = INVALID_SOCKET;
	}

	if (INVALID_SOCKET != m_skServer)
	{
		closesocket(m_skServer);
		m_skServer = INVALID_SOCKET;
	}

	for(int i = 0; i < MAX_IPADDRESS_NUMBER; ++i)
	{
		if(INVALID_SOCKET != m_skSendClient[i])
		{
			closesocket(m_skSendClient[i]);
			m_skSendClient[i] = INVALID_SOCKET;
		}
	}
}
