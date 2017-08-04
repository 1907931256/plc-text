#ifndef VIXHZ_DEF_H
#define VIXHZ_DEF_H

#if defined(WIN32)
// For FILETIME in FromFileTime, until it moves to a new converter class.
// See TODO(iyengar) below.
//#include <windows.h>
#define CALLBACK __stdcall

#ifdef PLCHZ
# define VIXHZ_EXPORT __declspec(dllexport)
#else
# define VIXHZ_EXPORT __declspec(dllimport)
#endif

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
//一般情况下按照默认值
typedef struct MODBUSADDRDataInfo{
	int nOffSet1;	//地址10000
	int nByteNum1;	//长度
	int nOffSet2;	//30000
	int nByteNum2;
	int nOffSet3;	//40000
	int nByteNum3;

	int nModeMax;
	int nScreenNum;/*几个屏幕*/


	int nPtzNum;

	MODBUSADDRDataInfo()
	{
		nOffSet1 = 0;
		nByteNum1 = 130;
		nOffSet2 = 0;
		nByteNum2 = 7;
		nOffSet3 = 0;
		nByteNum3 = 30;

		nModeMax = 2;
		nScreenNum = 6;

		nPtzNum = 30;
	}
}MODBUSADDRDataInfo;

#endif
