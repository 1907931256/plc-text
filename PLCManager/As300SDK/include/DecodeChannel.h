#pragma  once
#include "typedef.h"
class CDecodeChannel
{
public:
	CDecodeChannel(int nChannnelID);
	~CDecodeChannel();

private:
	int		m_nChannelID; //½âÂëÍ¨µÀºÅ
	HWND    m_hwnd;
	DEC_STATE	  m_DecState;
	int		m_nDownData;
	int		m_nTotalData;
public:
	BOOL  Open(BOOL bStreamMode);
	BOOL  Close();
	BOOL  OpenFile(char *pchFileName);
	BOOL  CloseFile();
	BOOL  StartPreview(HWND hWnd);
	BOOL  StopPreview();
	BOOL  InputData(unsigned char *pData, unsigned long nLen, unsigned long ulParam);
	BOOL  SetPlayPose(float fpercent);
	float GetPlayPose();
	BOOL  ResetSourceBuffer();
	BOOL  ResetBuffer();
	BOOL  StartAudio();
	BOOL  StopAudio();
	BOOL  Pause(BOOL bpause);
	BOOL  Fast();
	BOOL  Slow();

	long  GetSourceBufferRemain();
	long  GetBufferValue();
	void  SetDownData(long ldata);
	long  GetDownData();
	

// 	BOOL GetCurrentPlayTime(RVNET_TIME& netTime);
// 	BOOL SnapPicture(char* szPictureName);
// 	BOOL StartRecord(char* szFileName);
// 	BOOL StopRecord();
};