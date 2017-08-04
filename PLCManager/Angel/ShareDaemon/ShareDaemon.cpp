// ShareDaemon.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ShareDaemon.h"

#pragma data_seg("Daemon_Share")
BOOL	g_bDeamonRun = FALSE;
#pragma data_seg()
#pragma comment(linker,"/SECTION:Daemon_Share,RWS")

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


SHAREDAEMON_API int GetShareDaemon(void)
{
	return g_bDeamonRun;
}

SHAREDAEMON_API void SetShareDaemon(BOOL bValue)
{
	g_bDeamonRun = bValue;
}

