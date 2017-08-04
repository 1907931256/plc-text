#ifndef _WINDOWS_PROCESS_H_
#define _WINDOWS_PROCESS_H_

#include <tchar.h>
#define		WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Tlhelp32.h>

#ifndef MAX_FILE_PATH
#define MAX_FILE_PATH   (1024)
#endif//MAX_FILE_PATH

class WindowsProcess
{
public:
    static TCHAR* ExeFileFullPath(TCHAR *sPath, int *pPathLength);
    static TCHAR* ExeFileDir(TCHAR *sFileDir, int *pFileDirLength);
    static BOOL FindProcess(TCHAR *sProcessName);
    static BOOL StartProcess(const TCHAR *sFileFullPath);
    static BOOL KillProcess(const TCHAR *sProcessName);
    static int AutoStart(bool bAutoStart);
};

#endif //_WINDOWS_PROCESS_H_
