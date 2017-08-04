#include <stdio.h>
#include <string.h>
#include <tchar.h>
#define		WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "WindowsProcess.h"
#include "..\ShareDaemon\ShareDaemon.h"
#pragma comment(lib,"../release/ShareDaemon")

#ifdef _UNICODE
#pragma comment(linker, "/subsystem:\"windows\"   /entry:\"wmainCRTStartup\"")
#else
#pragma comment(linker, "/subsystem:\"windows\"   /entry:\"mainCRTStartup\"")
#endif
//////////////////////////////////////////////////////////////////////////
#define DAEMON_SELF_NAME                _T("MYEXE_DAEMOND")
#define BE_DAEMONED_PROCESS_NAME        _T("PlcText.exe")

int _tmain(int argc, _TCHAR* argv[])
{
	//设置程序开机自启动
    WindowsProcess::AutoStart(TRUE);
    //ensure only one daemon
    HANDLE hMutex = CreateMutex(NULL, FALSE, DAEMON_SELF_NAME);
    if ((hMutex != NULL) && (::GetLastError() != ERROR_ALREADY_EXISTS))
    {
		SetShareDaemon(TRUE);
        while(GetShareDaemon())
        {
			BOOL bFind = WindowsProcess::FindProcess(BE_DAEMONED_PROCESS_NAME);
			if (!bFind)
			{
				TCHAR sExePath[MAX_FILE_PATH];
				int   nExePathLength = MAX_FILE_PATH;

				memset(sExePath, 0, sizeof(TCHAR)*MAX_FILE_PATH);
				WindowsProcess::ExeFileDir(NULL, &nExePathLength);
				if (nExePathLength > 0 && nExePathLength < MAX_FILE_PATH)
				{
					nExePathLength = MAX_FILE_PATH;
					WindowsProcess::ExeFileDir(sExePath, &nExePathLength);
					_tcscat(sExePath, BE_DAEMONED_PROCESS_NAME);
					if (WindowsProcess::StartProcess(sExePath))
					{
						Sleep(1000);
					}
					else
					{
						printf("Start Error!\r\n");
					}
				}
			}
            Sleep(500);
        }
		CloseHandle(hMutex);
    }
    else
    {
        printf("Daemon is already running.\r\n");
    }

    return 0;
}
