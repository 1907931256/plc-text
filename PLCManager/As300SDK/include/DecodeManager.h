#pragma  once

#include "DecodeChannel.h"

typedef void (WINAPI *fnAdioRecCB)(LPBYTE pDataBuffer, DWORD DataLength, long nUser);
//管理解码通道
class CDecodeManager
{
public:
	~CDecodeManager();
	CDecodeManager();
	static CDecodeManager* Instance();
	static void UnInstance();
private:
	static CDecodeManager*	s_DecManager;
	CDecodeChannel*			m_decChannel[MACDECODENUM]; //前面64个通道为实时监视解码，后面为回放解码通道
public:
	void	CreateDecChannnel(int nNum);
	CDecodeChannel* GetDecChannel(int nChannelID);
//对讲仅在99和100端口---------------------------------------------
	BOOL OpenTalkPort(int nPort);
	BOOL CloseTalkPort(int nPort);
	BOOL InputTalkData(int nPort,PBYTE pBufData, DWORD dwDataLen);
	//打开音频采集
	BOOL OpenAudioRecord(fnAdioRecCB pCallBack, long nBitsPerSample, long nSamplesPerSec, long nLength, long nReserved, long nUser);
	BOOL CloseAudioRecord();
//--视频
	BOOL OpenVideo(int nPort,BOOL bStreamMode=TRUE,HWND hwnd =NULL);
	BOOL Close(int nPort);
	BOOL OpenFile(int nPort,char* szFileName);
	BOOL CloseFile(int nPort);
	BOOL ResetBuffer(int nPort);
	BOOL ResetSourceBuffer(int nPort);
	BOOL InputData(int nPort,unsigned char *pData, unsigned long nLen, unsigned long ulParam);
//--音频
	BOOL CloseAudio(int nPort);
	BOOL OpenAudio(int nPort);

//抓图--
	BOOL SnapPicture(int nPort,const char*szPictureName);
//录像
	BOOL StartRecord(int nPort,const char*szRecordName);
	BOOL StopRecord(int nPort);


};