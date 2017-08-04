
#include "CGetInfoThread.h"
#include "Client.h"

CGetInfoThread::CGetInfoThread(CClientManager *cms)
:base::SimpleThread("CGetInfoThread")
,m_pClientManager(cms)
,m_nRefresh(50)
{

}

CGetInfoThread::~CGetInfoThread()
{

}

void CGetInfoThread::startSend()
{
	if (!HasBeenStarted())
	{
		Start();
	}
}

void CGetInfoThread::stopSend()
{
	if (HasBeenStarted())
	{
		Join();
	}
}
void CGetInfoThread::Run()
{
	while(m_pClientManager)
	{
		m_pClientManager->GetPlcDataInfo();
#ifdef WIN32
		Sleep(m_nRefresh);
#else
		usleep(10000);
#endif
	}

}
