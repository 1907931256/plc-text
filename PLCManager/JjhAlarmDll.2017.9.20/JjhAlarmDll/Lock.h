#pragma once

#include <Winsock2.h>
#include "JjhAlarmDll.h"

class Lock  
{  
private:      
	CRITICAL_SECTION m_cs;  
public:  
	Lock(CRITICAL_SECTION  cs) : m_cs(cs)  
	{  
		EnterCriticalSection(&m_cs);  
	}  
	~Lock()  
	{  
		EnterCriticalSection(&m_cs);
	}  
};  

