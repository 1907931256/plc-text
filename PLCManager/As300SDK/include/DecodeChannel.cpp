#include "stdafx.h"
#include "DecodeChannel.h"
#include "vnplayer.h"


CDecodeChannel::CDecodeChannel(int nChannelID)
{
	memset(&m_DecState,0,sizeof(m_DecState));
	m_hwnd = NULL;
	m_nChannelID = nChannelID;
	m_nDownData = 0; //下载的数据
	m_nTotalData = 0;
}

CDecodeChannel::~CDecodeChannel()
{

}
BOOL CDecodeChannel::Open(BOOL bStreamMode)
{
	if (m_DecState.nState!=DSRC_NOPLAY)
	{
		return FALSE;
	}
	BYTE byFileHeadBuf;
	if (VN_PLAY_OpenStream(m_nChannelID, &byFileHeadBuf, 1, SOURCE_BUF_MIN*150))
	{	
		VN_PLAY_SetStreamOpenMode(m_nChannelID, bStreamMode?STREAME_REALTIME:STREAME_FILE);	

		m_DecState.bStreamMode = bStreamMode;
		m_DecState.bOpen = TRUE;
		m_DecState.ulParam = NULL;
		//m_bPause = FALSE;
		return TRUE;
	}
	return FALSE;
}
BOOL CDecodeChannel::Close()
{
	if (m_DecState.bOpen)
	{
		VN_PLAY_CloseStream(m_nChannelID);
		m_DecState.bOpen = FALSE;
		m_DecState.ulParam = NULL;
		return TRUE;
	}
	return FALSE;
}
BOOL CDecodeChannel::OpenFile(char *pchFileName)
{
	VN_PLAY_SetFileRefCallBack(m_nChannelID, NULL, NULL);

	if (VN_PLAY_OpenFile(m_nChannelID, pchFileName))
	{
		m_DecState.nState = DSRC_RECORDFILE;
		return TRUE;
	}
	return FALSE;
}
BOOL CDecodeChannel::StartPreview(HWND hWnd)
{
	BOOL bRet = FALSE;
	bRet = VN_PLAY_Play(m_nChannelID, hWnd);
	if (bRet)
	{
		m_DecState.bPreview = TRUE;
		m_DecState.ulParam = NULL;
		m_hwnd = hWnd;
	}
	return bRet;
}
BOOL CDecodeChannel::StopPreview()
{
	BOOL bRet = VN_PLAY_Stop(m_nChannelID);
	if (bRet)
	{
		m_DecState.bPreview = FALSE;
		m_DecState.ulParam = NULL;
		m_hwnd = NULL;
	}
	return bRet;
}
BOOL  CDecodeChannel::CloseFile()
{
	if (VN_PLAY_CloseFile(m_nChannelID))
	{
		m_DecState.nState = DSRC_NOPLAY;
		return TRUE;
	}
	return FALSE;
}
BOOL CDecodeChannel::InputData(unsigned char *pData, unsigned long nLen, unsigned long ulParam)
{
	if (!m_DecState.bPreview)
	{
		return TRUE;
	}
//	m_DecState.ulParam = ulParam;
	BOOL bRet = VN_PLAY_InputData( m_nChannelID ,(PBYTE)pData, nLen );
	return bRet;
}
BOOL CDecodeChannel::SetPlayPose(float fpercent)
{
	return VN_PLAY_SetPlayPos(m_nChannelID, fpercent);
}
float CDecodeChannel::GetPlayPose()
{
	return VN_PLAY_GetPlayPos(m_nChannelID);
}
BOOL CDecodeChannel::ResetSourceBuffer()
{
	return VN_PLAY_ResetSourceBuffer(m_nChannelID);
}
BOOL CDecodeChannel::ResetBuffer()
{
	return (VN_PLAY_ResetBuffer(m_nChannelID, BUF_VIDEO_RENDER)
		&&  VN_PLAY_ResetBuffer(m_nChannelID, BUF_VIDEO_SRC));
}

BOOL CDecodeChannel::StartAudio()
{
	BOOL bResult = VN_PLAY_PlaySoundShare(m_nChannelID);
	return bResult;
}
BOOL CDecodeChannel::StopAudio()
{
	BOOL bResult = VN_PLAY_StopSoundShare(m_nChannelID);
	return bResult;
}
// BOOL CDecodeChannel::SnapPicture(char* szPictureName)
// {
// 	return VN_PLAY_CatchPic(m_nChannelID, szPictureName);
// }
// 
// BOOL CDecodeChannel::StartRecord(char* szFileName)
// {
// 	return VN_PLAY_StartDataRecord(m_nChannelID, szFileName, 0);
// }
// BOOL CDecodeChannel::StopRecord()
// {
// 	return VN_PLAY_StopDataRecord(m_nChannelID);
// }

BOOL CDecodeChannel::Pause(BOOL bpause)
{
	return VN_PLAY_Pause(m_nChannelID,bpause);
}
BOOL CDecodeChannel::Fast()
{
	return VN_PLAY_Fast(m_nChannelID);
}
BOOL CDecodeChannel::Slow()
{
	return VN_PLAY_Slow(m_nChannelID);
}

//获取
long CDecodeChannel::GetSourceBufferRemain()
{
	return VN_PLAY_GetSourceBufferRemain(m_nChannelID); //回放缓存中剩余的值
}
long CDecodeChannel::GetBufferValue()
{
	return VN_PLAY_GetBufferValue(m_nChannelID, BUF_VIDEO_RENDER); //获得显存值
}
void CDecodeChannel::SetDownData(long ldata)
{
	m_nDownData = ldata;
}
long CDecodeChannel::GetDownData()
{
	return m_nDownData;
}
// BOOL CDecodeChannel::GetCurrentPlayTime(RVNET_TIME& netTime)
// {
// 	int retLen = 0;
// 	return VN_PLAY_QueryInfo(m_nChannelID, 1, (char*)&netTime, sizeof(RVNET_TIME), &retLen);
// }