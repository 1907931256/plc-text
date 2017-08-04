#ifndef CGETINFOTHREAD_H
#define CGETINFOTHREAD_H

#include <Winsock2.h>
#include "base/threading/simple_thread.h"

class CClientManager;
class CGetInfoThread : public base::SimpleThread
{
public:
	CGetInfoThread(CClientManager* cms = NULL);
	~CGetInfoThread();
	void startSend();
	void stopSend();
	int  m_nRefresh;

protected:
	void Run();

private:
	CClientManager *m_pClientManager;

};

#endif // CGETINFOTHREAD_H
