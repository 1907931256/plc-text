#pragma once
#define MAX_IPADDRESS_NUMBER	1
typedef enum
{
	EUDP_Normal,
	EUDP_Broadcast,
	EUDP_Multcast,
}EUDPTYPE;

class CNetUDP
{
public:
	CNetUDP();
	~CNetUDP(void);

	int SetSendSocket(int nSendPort, const char *szIP, EUDPTYPE type);
	int SetRecvSocket(int nRecvPort, const char *szIP, EUDPTYPE type);

	int RecvFrom(char *pReadBuf, int nRSize, sockaddr_in &senderAddr);
	int SendTo(const char *pWriteBuf, int nWSize);
	void DisConnect();

private:
	SOCKET CreateUDPSocket(EUDPTYPE type);

private:
	sockaddr_in m_sendAddr;
	SOCKET m_skClient;
	SOCKET m_skServer;
	SOCKET m_skSendClient[MAX_IPADDRESS_NUMBER];
};
