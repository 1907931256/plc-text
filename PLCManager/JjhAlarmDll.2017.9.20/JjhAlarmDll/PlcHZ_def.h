#ifndef VIXHZ_DEF_H
#define VIXHZ_DEF_H

#if defined(WIN32)
// For FILETIME in FromFileTime, until it moves to a new converter class.
// See TODO(iyengar) below.
//#include <windows.h>
#define CALLBACK __stdcall
#define VIXHZ_EXPORT __declspec(dllexport)
//#ifdef PLCHZ
//# define VIXHZ_EXPORT __declspec(dllexport)
//#else
//# define VIXHZ_EXPORT __declspec(dllimport)
//#endif

#else
#define CALLBACK
#define HWND void*

#ifdef PLCHZ
# define VIXHZ_EXPORT __attribute__((visibility("default")))
#else
# define VIXHZ_EXPORT
#endif
#endif

#define CONNECT_PLC_SUC 1
#define CONNECT_PLC_ERR 2
#define CONNECT_PLC_REC 3
//一般情况下按照默认值
typedef struct PLCADDRDataInfo{
	int nDB;
	int nOffSet;
	int nByteNum;

	int nModeMax;
	int nScreenNum;/*几个屏幕*/

	
	int nPtzNum;
	
	int nZoomOffset;/*mode*/
	int nZoom20;
	int nZoom40;
	int nZoom45;

	int nSwitchGroup;/*switchgroup*/
	

	PLCADDRDataInfo()
	{
		nDB = 501;
		nOffSet = 0;
		nByteNum = 190;

		nModeMax = 4;
		nScreenNum = 6;
		
		nPtzNum = 33;
		
		nZoomOffset = 2;

		nZoom20 = 0;
	    nZoom40 = 0;
		nZoom45 = 0;

		nSwitchGroup = 0;
		
	}
}PLCADDRDataInfo;

#endif
