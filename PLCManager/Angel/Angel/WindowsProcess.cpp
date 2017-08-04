#include "WindowsProcess.h"
#include <tchar.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

TCHAR* WindowsProcess::ExeFileFullPath(TCHAR *sPath, int *pPathLength)
{
    TCHAR szLocalPath[MAX_FILE_PATH];
    int nLength;

    memset(szLocalPath, 0, MAX_FILE_PATH*sizeof(TCHAR));
    nLength = ::GetModuleFileName(NULL, szLocalPath, MAX_FILE_PATH);
    if (nLength > 0 && nLength < MAX_FILE_PATH)
    {
        if (pPathLength != NULL)
        {
            *pPathLength = nLength;
        }
        if (sPath != NULL)
        {
            _tcscpy(sPath, szLocalPath);
        }
    }
    else
    {
        sPath = NULL;
    }

    return sPath;
}

TCHAR* WindowsProcess::ExeFileDir(TCHAR *sFileDir, int *pFileDirLength)
{
    TCHAR szLocalFileDir[MAX_FILE_PATH];
    int nFileDirLength = MAX_FILE_PATH;
    int nPos;

    memset(szLocalFileDir, 0, MAX_FILE_PATH*sizeof(TCHAR));
    ExeFileFullPath(NULL, &nFileDirLength);
    if (nFileDirLength > 0 && nFileDirLength < MAX_FILE_PATH)
    {
        nFileDirLength = MAX_FILE_PATH;
        ExeFileFullPath(szLocalFileDir, &nFileDirLength);
        nPos = nFileDirLength-1;
        while (
            (nPos > 0)
            && *(szLocalFileDir+nPos) != _T('\\')
            )
        {
            *(szLocalFileDir+nPos) = _T('\0');
            nPos --;
        }
        if (pFileDirLength != NULL)
        {
            *pFileDirLength = nPos+1;
        }
        if (sFileDir != NULL)
        {
            _tcscpy(sFileDir, szLocalFileDir);
        }

    }

    return NULL;
}

BOOL WindowsProcess::FindProcess(TCHAR *sProcessName)
{
    HANDLE hSnapshot;
    BOOL bFind = FALSE;
    BOOL bHaveNext;
    PROCESSENTRY32 processEntry;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != NULL)
    {
        bHaveNext = FALSE;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        bHaveNext = Process32First(hSnapshot, &processEntry);
        while(bHaveNext)
        {
            if (_tcscmp(sProcessName, processEntry.szExeFile) == 0)
            {
                bFind = TRUE;
                break;
            }
            bHaveNext = Process32Next(hSnapshot, &processEntry);
        }
        CloseHandle(hSnapshot);
    }

    return bFind;
}

BOOL WindowsProcess::StartProcess(const TCHAR *sFileFullPath)
{
    STARTUPINFO         startupInfo;
    PROCESS_INFORMATION processInfo;

    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    memset(&processInfo, 0, sizeof(processInfo));
    return CreateProcess(sFileFullPath, NULL, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,
        NULL, NULL, &startupInfo, &processInfo);
}

BOOL WindowsProcess::KillProcess(const TCHAR *sProcessName)
{
    HANDLE hSnapshot;
    BOOL bKill = FALSE;
    BOOL bHaveNext;
    PROCESSENTRY32 processEntry;
    HANDLE hProcess;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != NULL)
    {
        bHaveNext = FALSE;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        bHaveNext = Process32First(hSnapshot, &processEntry);
        while(bHaveNext)
        {
            if (_tcscmp(sProcessName, processEntry.szExeFile) == 0)
            {
                if (processEntry.th32ProcessID != NULL)
                {
                    hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,processEntry.th32ProcessID);
                    if (hProcess != NULL)
                    {
                        if (TerminateProcess(hProcess,0))
                        {
                            bKill = TRUE;
                        }
                    }
                }
            }
            bHaveNext = Process32Next(hSnapshot, &processEntry);
        }
        CloseHandle(hSnapshot);
    }

    return bKill;
}

int WindowsProcess::AutoStart(bool bAutoStart)
{
    HKEY    hKey;
    DWORD   dwDisposition;
    TCHAR   szKeyName[1024];
    TCHAR   szProgramPath[1024];

    GetModuleFileName(NULL, szKeyName, MAX_PATH);
    PathStripPath(szKeyName);
    GetModuleFileName(NULL, szProgramPath, MAX_PATH);
    if (
        ::RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS
        )
    {
        if (bAutoStart)
        {
            ::RegSetValueEx(hKey, szKeyName, 0, REG_SZ, (LPBYTE)szProgramPath, _tcslen(szProgramPath)*sizeof(TCHAR));
        }
        else
        {
            ::RegDeleteValue(hKey, szKeyName);
        }
        ::RegFlushKey(hKey);
        ::RegCloseKey(hKey);
    }

    return 0;
}
