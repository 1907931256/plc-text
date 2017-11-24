#include "stdafx.h"
#include "DecodeManager.h"
#include "vnplayer.h"

CDecodeManager*	CDecodeManager::s_DecManager=NULL;

CDecodeManager::CDecodeManager()
{
	for (int i=0;i<MACDECODENUM;i++)
	{
		m_decChannel[i] = NULL;
	}
}

CDecodeManager::~CDecodeManager()
{
	for (int i=0;i<MACDECODENUM;i++)
	{
		if (m_decChannel[i])
		{
			delete m_decChannel[i];
			m_decChannel[i] = NULL;
		}
	}
}

CDecodeManager* CDecodeManager::Instance()
{
	if (s_DecManager==NULL)
	{
		s_DecManager = new CDecodeManager();
	}
	return s_DecManager;
}
void CDecodeManager::UnInstance()
{
	if (s_DecManager)
	{
		delete s_DecManager;
		s_DecManager = NULL;
	}
}
CDecodeChannel* CDecodeManager::GetDecChannel(int nChannelID)
{
	if (nChannelID>=0&&nChannelID<80)
	{
		return m_decChannel[nChannelID];
	}
	return NULL;
}
void CDecodeManager::CreateDecChannnel(int nNum)
{
	for (int i=0;i<nNum;i++)
	{
		m_decChannel[i] = new CDecodeChannel(i);
	}
}


BOOL CDecodeManager::OpenTalkPort(int nPort)
{
	BOOL bResult = FALSE;

	bResult = VN_PLAY_OpenStream(nPort, NULL, 0, SOURCE_BUF_MIN*20);
	if ( !bResult )
	{
		return FALSE;
	}

	bResult = VN_PLAY_SetStreamOpenMode(nPort, STREAME_REALTIME);
	if ( !bResult )
	{
		VN_PLAY_CloseStream(nPort);
		return FALSE;
	}

	bResult = VN_PLAY_SetDisplayBuf(nPort, MIN_DIS_FRAMES);
	if ( !bResult )
	{
		VN_PLAY_CloseStream(nPort);
		return FALSE;
	}

	bResult = VN_PLAY_Play(nPort, NULL);
	if (!bResult)
	{
		VN_PLAY_CloseStream(nPort);
		return FALSE;
	}
	bResult = VN_PLAY_PlaySoundShare(nPort);
	if (!bResult)
	{
		VN_PLAY_Stop(nPort);
		VN_PLAY_CloseStream(nPort);
		return FALSE;
	}
	
	return bResult;
}

BOOL CDecodeManager::CloseTalkPort(int nPort)
{
	BOOL bResult = FALSE;
	bResult	 = VN_PLAY_StopSoundShare(nPort);
	bResult &= VN_PLAY_Stop(nPort);
	bResult &= VN_PLAY_CloseStream(nPort);


	return bResult;
}

BOOL CDecodeManager::InputTalkData(int nPort,PBYTE pBufData, DWORD dwDataLen)
{
	BOOL  bResult = FALSE;
	bResult = VN_PLAY_InputData( nPort, pBufData, dwDataLen );
	return bResult;
}

BOOL CDecodeManager::OpenAudioRecord(fnAdioRecCB pCallBack, long nBitsPerSample, long nSamplesPerSec, long nLength, long nReserved, long nUser)
{
	BOOL bRet = FALSE;
	bRet = VN_PLAY_OpenAudioRecord( pCallBack , nBitsPerSample , nSamplesPerSec , nLength , nReserved , nUser );
	return bRet;

}
BOOL CDecodeManager::CloseAudioRecord()
{		
	return VN_PLAY_CloseAudioRecord();
}
BOOL CDecodeManager::OpenVideo(int nPort,BOOL bStreamMode,HWND hwnd )
{
	BOOL bRet = FALSE;
	if (m_decChannel[nPort])
	{
		 bRet = m_decChannel[nPort]->Open(bStreamMode);
		 if (bRet)
		 {
			 bRet = m_decChannel[nPort]->StartPreview(hwnd);
		 }
	}
	return bRet;
}
BOOL CDecodeManager::Close(int nPort)
{
	BOOL bRet = FALSE;
	if (m_decChannel[nPort])
	{
		bRet = m_decChannel[nPort]->StopPreview();
		if (bRet)
		{
			bRet =  m_decChannel[nPort]->Close();
		}

	}
	return bRet;
}
BOOL CDecodeManager::OpenFile(int nPort,char* szFileName)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->OpenFile(szFileName);
	}
	return FALSE;
}
BOOL CDecodeManager::CloseFile(int nPort)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->CloseFile();
	}
	return FALSE;
}

BOOL CDecodeManager::ResetBuffer(int nPort)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->ResetBuffer();
	}
	return FALSE;
}

BOOL CDecodeManager::ResetSourceBuffer(int nPort)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->ResetSourceBuffer();
	}
	return FALSE;
}
BOOL CDecodeManager::InputData(int nPort,unsigned char *pData, unsigned long nLen, unsigned long ulParam)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->InputData(pData,nLen,ulParam);
	}
	return FALSE;
}

BOOL CDecodeManager::OpenAudio(int nPort)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->StartAudio();
	}
	return FALSE;

}
BOOL CDecodeManager::CloseAudio(int nPort)
{
	if (m_decChannel[nPort])
	{
		return m_decChannel[nPort]->StopAudio();
	}
	return FALSE;
}
BOOL CDecodeManager::SnapPicture(int nPort,const char*szPictureName)
{
	if (m_decChannel[nPort])
	{
		//return m_decChannel[nPort]->SnapPicture((char*)szPictureName);
	}
	return FALSE;
}
//Â¼Ïñ
BOOL CDecodeManager::StartRecord(int nPort,const char*szRecordName)
{
	if (m_decChannel[nPort])
	{
	//	return m_decChannel[nPort]->StartRecord((char*)szRecordName);
	}
	return FALSE;
}

BOOL CDecodeManager::StopRecord(int nPort)
{
	if (m_decChannel[nPort])
	{
	//	return m_decChannel[nPort]->StopRecord();
	}
	return FALSE;
}