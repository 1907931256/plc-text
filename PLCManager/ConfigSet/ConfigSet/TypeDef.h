#pragma once
#include <map>
#include <list>

static TCHAR GROUPID[] = L"GROUPID";
static TCHAR ID[] = L"id";
static TCHAR SGROUP[] = L"GROUP";
static TCHAR GROUPSCREEN[] = L"GroupScreen";
static TCHAR CARAMPTZ[] = L"CARAMPTZ";
static TCHAR SCREENCAMERA[] = L"SCREENCAMERA";
static TCHAR CAMERAZOOM[] = L"CAMERAZOOM";
static TCHAR CAMERAZOOMHEIGHT[] = L"CAMERAZOOMHEIGHT";
static TCHAR FREECUTSCREEN[] = L"FREECUTSCREEN";

#define MAXWNDNUM 16
#define  SIDE_WIDTH 2

typedef struct DeviceInfoZoom
{
	CString csIp;
    CString csIpcPuid;
	CString csZoomCfg;
	BOOL bBol;
	DeviceInfoZoom()
	{
		csIpcPuid = _T("");
		csZoomCfg = _T("");
		csIp = _T("");
		bBol = FALSE;
	}
}DeviceInfoZoom;

typedef struct DeviceItemInfo
{
    CString csDeviceId;
	CString csDeviceName;
	CString csDeviceIp;
	CString csDevicePort;
	CString csComponyType;
	CString csUserName;
	CString csUserPwd;
	CString csDeviceType;
	int iStreamType;
	DeviceItemInfo()
	{
		csDeviceId = _T("");
		csDeviceName = _T("");
		csDeviceIp = _T("");
		csDevicePort = _T("");
		csComponyType = _T("");
		csUserName = _T("");
		csUserPwd = _T("");
		csDeviceType = _T("");
		iStreamType = 1;
	}
}DeviceItemInfo, *pDeviceItemInfo;

typedef std::list<DeviceItemInfo> IpcList;
typedef std::map<int, IpcList> IpcGroupMap;

typedef std::list<int> TvWallScreenList;
typedef std::map<int, TvWallScreenList> TvWallIndex2Screen;
