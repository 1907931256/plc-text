#pragma  once

#include "DecodeChannel.h"

typedef void (WINAPI *fnAdioRecCB)(LPBYTE pDataBuffer, DWORD DataLength, long nUser);
//�������ͨ��
class CDecodeManager
{
public:
	~CDecodeManager();
	CDecodeManager();
	static CDecodeManager* Instance();
	static void UnInstance();
private:
	static CDecodeManager*	s_DecManager;
	CDecodeChannel*			m_decChannel[MACDECODENUM]; //ǰ��64��ͨ��Ϊʵʱ���ӽ��룬����Ϊ�طŽ���ͨ��
public:
	void	CreateDecChannnel(int nNum);
	CDecodeChannel* GetDecChannel(int nChannelID);
//�Խ�����99��100�˿�---------------------------------------------
	BOOL OpenTalkPort(int nPort);
	BOOL CloseTalkPort(int nPort);
	BOOL InputTalkData(int nPort,PBYTE pBufData, DWORD dwDataLen);
	//����Ƶ�ɼ�
	BOOL OpenAudioRecord(fnAdioRecCB pCallBack, long nBitsPerSample, long nSamplesPerSec, long nLength, long nReserved, long nUser);
	BOOL CloseAudioRecord();
//--��Ƶ
	BOOL OpenVideo(int nPort,BOOL bStreamMode=TRUE,HWND hwnd =NULL);
	BOOL Close(int nPort);
	BOOL OpenFile(int nPort,char* szFileName);
	BOOL CloseFile(int nPort);
	BOOL ResetBuffer(int nPort);
	BOOL ResetSourceBuffer(int nPort);
	BOOL InputData(int nPort,unsigned char *pData, unsigned long nLen, unsigned long ulParam);
//--��Ƶ
	BOOL CloseAudio(int nPort);
	BOOL OpenAudio(int nPort);

//ץͼ--
	BOOL SnapPicture(int nPort,const char*szPictureName);
//¼��
	BOOL StartRecord(int nPort,const char*szRecordName);
	BOOL StopRecord(int nPort);


};