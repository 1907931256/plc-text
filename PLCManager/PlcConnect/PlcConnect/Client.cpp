//#include "HttpConfig.h"
#include <afx.h>
#include "Client.h"
#include "value.h"
#include "reader.h"
#include "writer.h"
#include "NetUDP.h"

CClientManager * CClientManager::m_pThis = NULL;

#define __countof(array) (sizeof(array)/sizeof(array[0]))
#pragma warning (disable:4996)

void TraceMsgA(LPCSTR pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	int nBuff;
	CHAR szBuffer[0x7fff];
	nBuff = _vsnprintf(szBuffer, __countof(szBuffer), pFormat, args);
	//::wvsprintf(szBuffer, pFormat, args);
	//assert(nBuff >=0);
	OutputDebugStringA(szBuffer);
	va_end(args);	
}

CClientManager::CClientManager()
{
	m_bEnablePriority = false;
	m_InfoCallFun = NULL;
	m_CallLoginfo = NULL;
	m_PLCHttpIp = "0.0.0.0";
	m_nPLCHttpPort = 5000;
	m_KeyId = 1;
	m_TvWallPromat = "视频切换到主控台";
	m_dwDataUser = 0;
	m_SwitchGroupAddr = 0;
	m_bReConnect = TRUE;
	m_bGetPlcData = TRUE;
	m_bGetModbusData = TRUE;
	m_bReConnectModbus = TRUE;
	m_bKeepAlive = FALSE;
	m_bKeepAliveModbus = FALSE;
	m_iIpcStateOffSet = 100;
	m_iModbusIpcStateOffSet = 1;
	m_usHeartBeet = 0;
	m_iMaxFeet = -1;
	m_iMaxHeight = -1;
	m_nRefresh = 10;
	m_nIDGenerator = 0;
	m_bBolModbus = FALSE;
	m_KeepAliveThread = NULL;
	m_GetPlcDataThread = NULL;
	m_ReConnectThread = NULL;
	m_ReConnectThreadModbus = NULL;
	m_GetModbusDataThread = NULL;
	m_KeepAliveThreadModbus = NULL;
	//m_pGetInfoThread = new CGetInfoThread(this);
	m_ModbusServerThread = NULL;
	ptzOperationCmdOld = 0;
	//===2017/8/9 opc nanSha
	mOpcServer = NULL;
	mOpcGroup = NULL;
	m_bGetOpcData = FALSE;
	m_bReconnectOpc = FALSE;
	mConnectOpc = FALSE;
	m_ReConnectOpcThread = NULL;
	m_GetOpcDataThread = NULL;
	mOpcFlag = false;

	CIniFile iniFile(GetCurrentPath() + "\\softset.ini");
	int  nLevel = 0;
	int iPTZ = 0;
	int iZOOM = 0;
	int iTVWALL = 0;
	int iSwitchGroup;
	iniFile.ReadInt("LOG", "level", nLevel);
	iniFile.ReadInt("LOG", "ptz", iPTZ);
	iniFile.ReadInt("LOG", "zoom", iZOOM);
	iniFile.ReadInt("LOG", "tvwall", iTVWALL);
	iniFile.ReadInt("LOG", "switchgroup", iSwitchGroup);

	m_plog = new Logger(GetCurrentPath().c_str(), (EnumLogLevel)nLevel, iPTZ, iZOOM, iTVWALL, iSwitchGroup);//修改日志等级

	m_nMaxIndex = 0;
	m_nSendSwitch = 0;

	m_lHeartBeetIndex = 0;
	groupDataRefresh = FALSE;
	memset(m_lHeartBeetTime, 0, 50*sizeof(long));
	//=====PLC 通过tcp 连接=========
	//tcpConnect = false;
}

CClientManager::~CClientManager()
{
	if (!m_bBolModbus&&!mOpcFlag)
	{
		StopReConnect();

		StopGetPlcData();
	}
	else if(!mOpcFlag)
	{
		StopGetPlcDataModbus();

		StopReConnectModbus();

		stopKeepAlive2Modbus();
		if(mModbusServerCreateF)
			stopModbusServer();
	}
	else if(mOpcFlag)
	{
		StopGetOpcData();
		//Sleep(3000);
		StopReConnectOpc();
	}

	stopKeepAlive2TvWall();
	//m_lockClient.acquire();
	CClient * pClient = NULL;
	std::map<long, CClient*>::iterator iter = m_clients.begin();
	while (iter != m_clients.end())
	{
		pClient = iter->second;
		if (pClient)
		{
			pClient->release();
		}
		++iter;
	}
	m_clients.clear();
	//m_lockClient.release();
	//  内容中的智能指针可自动删除，xionggao.lee @2017.02.16
	// 	for (std::map<std::string, int*>::iterator ite = m_zoomPuid2FeetHeightMap.begin(); ite != m_zoomPuid2FeetHeightMap.end(); ite++)
	// 	{
	// 		delete []ite->second;
	// 	}
	m_zoomPuid2FeetHeightMap.clear();


	//m_pGetInfoThread->stopSend();
	//delete m_pGetInfoThread;
	//m_pGetInfoThread = NULL;

	//StopReConnect();

	//StopGetPlcData();

	//stopKeepAlive2TvWall();

	if (m_plog)
	{
		delete m_plog;
		m_plog = NULL;
	}
}

CClientManager* CClientManager::GetInstance()
{
	if(m_pThis != NULL)
	{
		return m_pThis;
	}
	m_pThis = new CClientManager();
	return m_pThis;
}

std::string CClientManager::GetCurrentPath()
{
	HMODULE module = GetModuleHandle(0); 
	char pFileName[MAX_PATH]; 
	GetModuleFileNameA(module, pFileName, MAX_PATH); 

	std::string csFullPath = pFileName; 
	int nPos = csFullPath.rfind( '\\' );
	if( nPos < 0 ) 
	{
		return ""; 
	}
	else 
	{
		return csFullPath.substr(0, nPos); 
	}
	return "";
}
void CClientManager::InitModbusOrPlc()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");
	int ipcStaFromdBbEnable=0;
	iniFile.ReadInt(_T("IPCINFOFROMAS300"),_T("Enable"),ipcStaFromdBbEnable);
	if(ipcStaFromdBbEnable)
	{
		ipcStaFromAs300 = true;
	}
	else
	{
		ipcStaFromAs300 = false;
	}

	int nSwitch;
	iniFile.ReadInt("PLCORMODBUS", "Switch", nSwitch);
	if(2 == nSwitch)
	{
		m_bBolModbus = TRUE;
		//Modbus
		InitModbusData();

		int serverEnable = 0;
		mModbusServerCreateF = false;
		iniFile.ReadInt("MODBUSSERVER","ServerCreate",serverEnable);
		if(serverEnable)
		{
			mModbusServerCreateF = true;
			StartModbusServer();
		}

		StartReConnectModbus();//开启定时重连Modbus线程
		StartGetModbusData(); //开启50ms读取modbus数据的线程
		//StartKeepAlive2TvWall();//开启TVWALL心跳线程
		StartKeepAlive2Modbus();//开启MODBUS心跳线程
	}
	else if(1==nSwitch)
	{
		m_bBolModbus = FALSE;
		//PLC
		InitData();
		StartGetPlcData(); //开启50ms读取plc_server数据的线程
		StartReConnect();//开启定时重连线程
		//StartKeepAlive2TvWall();//开启TVWALL心跳线程	
	}
	else if(3==nSwitch) //OPC client
	{
		opcInit();
		InitData();
		mOpcFlag = TRUE;
		//StartGetOpcData();
		StartReConnectOpc();
		StartGetOpcData();
	}

}
void CClientManager::InitData()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");
	///////////////////////////////////////////////////////////////////////////////
	int  nOffSet = 0;
	int  nNum = 0;
	iniFile.ReadInt("DB_SET", "db_offset", nOffSet);
	iniFile.ReadInt("DB_SET", "db_bytenum", nNum);

	std::string strDbData;
	std::vector<std::string> db_vecotr;
	iniFile.ReadString("DB_SET", "db_set", strDbData);

	StringSplit(strDbData, ",", db_vecotr);

	for (std::vector<std::string>::iterator ite = db_vecotr.begin();
		ite != db_vecotr.end(); ite++)
	{
		std::vector<std::string> dbvecotr;
		StringSplit(*ite, ":", dbvecotr);
		m_GroupDBMap.insert(std::make_pair(atoi(dbvecotr[0].c_str()), atoi(dbvecotr[1].c_str())));
		//===2017/6/2 
		mGroupSwitchFirst.insert(std::make_pair(atoi(dbvecotr[0].c_str()),true));
	}

	if (nOffSet >= 0)
	{
		m_PLCFormatInfo.nOffSet = nOffSet;
	}
	if (nNum >= 0)
	{
		m_PLCFormatInfo.nByteNum = nNum;
	}


	iniFile.ReadInt("DB_SET", "screen_mode", m_PLCFormatInfo.nModeMax);
	iniFile.ReadInt("DB_SET", "screen_num", m_PLCFormatInfo.nScreenNum);

	iniFile.ReadInt("DB_SET", "camera_num", m_PLCFormatInfo.nPtzNum);	

	iniFile.ReadInt("DB_SET", "ipcStateAddr", m_iIpcStateOffSet);

	iniFile.ReadInt("DB_SET","EnalbePriority",m_bEnablePriority);
	//iniFile.ReadInt("DB_SET", "zoom_offset", m_PLCFormatInfo.nZoomOffset);

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////PLC SCREEN ADDR//////////////////////////*/

	/*switch screen plc address */
	std::vector<std::string> PlcScreenAddArray;
	iniFile.ReadSectionString("PLCSCREENADDR", PlcScreenAddArray);

	/* SCREENID<<8|MODE,此为唯一性*/
	m_PlcScreenAddMap.empty();
	for(std::vector<std::string>::iterator iteScreenAddr = PlcScreenAddArray.begin(); 
		iteScreenAddr != PlcScreenAddArray.end(); iteScreenAddr++)
	{
		std::vector<std::string> vectorPLCScreenAdd;
		StringSplit(*iteScreenAddr, ",", vectorPLCScreenAdd);

		//PLCScreen1=1,1,65,0 ;SCREENID,MODE,ADDROFFSET,BIT

		unsigned short iPlcScreenId = atoi(vectorPLCScreenAdd[0].c_str()) & 0xff;
		unsigned short iSCREENMode = atoi(vectorPLCScreenAdd[1].c_str()) & 0xff;

		PlcScreenAddrInfo stScreenAddrInfo;	 
		stScreenAddrInfo.iAddr = atoi(vectorPLCScreenAdd[2].c_str()) & 0xff;
		stScreenAddrInfo.iBit = atoi(vectorPLCScreenAdd[3].c_str()) & 0xff;
		if (vectorPLCScreenAdd.size() >= 5)
			stScreenAddrInfo.iPriority = atoi(vectorPLCScreenAdd[4].c_str())&0xff;
		else
			stScreenAddrInfo.iPriority = 0;
		int iScreenModeMap = iPlcScreenId<<8|iSCREENMode;/* 高８位，低８位*/
		TraceMsgA("iScreenModeMap = %x\tiAdd = %d\tiBit = %d\tiPriority = %d\n",iScreenModeMap,stScreenAddrInfo.iAddr,stScreenAddrInfo.iBit,stScreenAddrInfo.iPriority);
		m_PlcScreenAddMap.insert(std::make_pair(iScreenModeMap,stScreenAddrInfo));
	}

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////PLC IPC ADDR//////////////////////////*/

	/*switch screen plc address */
	std::vector<std::string> vectorPlcIPCAddrArray;
	iniFile.ReadSectionString("PLCIPCADDR", vectorPlcIPCAddrArray);

	/* IPC,此为唯一性*/
	m_PlcIPCAddrMap.empty();
	for(std::vector<std::string>::iterator itePLCIPCAddr = vectorPlcIPCAddrArray.begin(); 
		itePLCIPCAddr != vectorPlcIPCAddrArray.end(); itePLCIPCAddr++)
	{
		std::vector<std::string> vectorPLCIPCAddr;
		StringSplit(*itePLCIPCAddr, ",", vectorPLCIPCAddr);

		//PLCIPC1=1,5 ;IPCID,ADDROFFSET

		unsigned short iPlcIPCAddrId = atoi(vectorPLCIPCAddr[0].c_str()) & 0xff;
		unsigned short iPlcIPCAddrOffset = atoi(vectorPLCIPCAddr[1].c_str()) & 0xff;


		m_PlcIPCAddrMap.insert(std::make_pair(iPlcIPCAddrId,iPlcIPCAddrOffset));
	}


	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////GROUP,PC,MODE,SCREENID,SPLITWINDOW,NO:IPCID,NO:IPCID//////////////////////////*/
	/* plc ip,port*/
	iniFile.ReadString("ADDR", "ServerIp", m_PLCHttpIp);
	iniFile.ReadInt("ADDR", "ServerPort", m_nPLCHttpPort);
	iniFile.ReadInt("ADDR", "ServerKeyId", m_KeyId);
	iniFile.ReadString("ADDR", "TvWallPromat", m_TvWallPromat);

	m_zoomFeet2AddrMap.empty();
	std::vector<std::string> ZoomFeetVector;
	iniFile.ReadSectionString("PLCZOOMADDR", ZoomFeetVector);
	for(std::vector<std::string>::iterator it = ZoomFeetVector.begin(); it != ZoomFeetVector.end(); it++)
	{
		std::vector<std::string> feetInfoVec;
		StringSplit(*it, ",", feetInfoVec);
		PlcScreenAddrInfo stZoomAddrInfo;
		stZoomAddrInfo.iAddr = atoi(feetInfoVec[1].c_str());
		stZoomAddrInfo.iBit = atoi(feetInfoVec[2].c_str());
		m_zoomFeet2AddrMap.insert(std::make_pair(atoi(feetInfoVec[0].c_str()), stZoomAddrInfo));
	}

	//////////////////zoom/////////////////////////////
	// 之前版本的配置文件中存在拼写错误(Height误写为Hight)，为了兼容老版本,增加一两种读取方式
	if (!iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHeight", m_iMaxHeight))
		iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHight", m_iMaxHeight);
	if (!iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MinHight", m_iMinHeight))
		m_iMinHeight = -25;

	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxFeet", m_iMaxFeet);


	std::string strHeight;
	int nIndex = 0;
	TCHAR szFiledName[128] = {0};
	do 
	{
		PlcScreenAddrInfo plcAddrInfo;
		if (!nIndex)
			_tcscpy(szFiledName,"HEIGHT");
		else
			_stprintf(szFiledName,"HEIGHT_%d",nIndex);
		nIndex ++;
		if (!iniFile.ReadString("PLCZOOMHEIGHTADDR", szFiledName, strHeight))
			break;
		std::vector<std::string> HeightAddrVec;
		StringSplit(strHeight, ",", HeightAddrVec);
		plcAddrInfo.iAddr = atoi(HeightAddrVec[0].c_str());
		plcAddrInfo.iBit = atoi(HeightAddrVec[1].c_str());
		m_ZoomHeightAddr.push_back(plcAddrInfo);
	} while (true);


	iniFile.ReadInt("SWITCHGROUPADDR", "SwitchGroup", m_SwitchGroupAddr);


	iniFile.ReadInt("SENDSWITCH", "Value", m_nSendSwitch);
	std::vector<std::string> SendSwitchArry;
	iniFile.ReadSectionString("THIRDADDR", SendSwitchArry);

	m_MapThirdAddr.empty();
	m_MapThirdAddr.clear();
	int count = 0;
	for (std::vector<std::string>::iterator it = SendSwitchArry.begin(); it != SendSwitchArry.end(); it++)
	{
		std::vector<std::string> strInfoArry;
		StringSplit(*it, ",", strInfoArry);
		ThirdADDR addr;
		addr.strIP = strInfoArry[0];
		addr.Port = atoi(strInfoArry[1].c_str());

		m_MapThirdAddr.insert(std::make_pair(++count, addr));
	}
	//==========textBar 2017/4/8  motify textBarMessage 2017/4/17 ==================

	//====read spreader address
	std::string SpreaderStr[SPREADER_COUNT];
	SpreaderStr[0]="WSPREADER";
	SpreaderStr[1]="LSPREADER";
	SpreaderStr[2]="PSPREADER"; //应与enum Spreder 顺序一致
	for(int iSpreader=0;iSpreader<SPREADER_COUNT;iSpreader++)
	{
		std::string landedString;
		if(iniFile.ReadString(SpreaderStr[iSpreader].c_str(),"Landed",landedString))
		{
			std::vector<std::string> LandedAddrVec;
			StringSplit(landedString, ",", LandedAddrVec);
			mSpreaderAddr[iSpreader].mland.iAddress = atoi(LandedAddrVec[0].c_str());
			mSpreaderAddr[iSpreader].mland.iBit = atoi(LandedAddrVec[1].c_str()); 
			std::string lockedString;
			if(iniFile.ReadString(SpreaderStr[iSpreader].c_str(),"Locked",lockedString))
			{
				std::vector<std::string>LockedAddrVec;
				StringSplit(lockedString,",",LockedAddrVec);
				mSpreaderAddr[iSpreader].mlock.iAddress = atoi(LockedAddrVec[0].c_str());
				mSpreaderAddr[iSpreader].mlock.iBit = atoi(LockedAddrVec[1].c_str());
				std::string twinModeString;
				if(iniFile.ReadString(SpreaderStr[iSpreader].c_str(),"TwinMode",twinModeString))
				{
					std::vector<std::string>twinModeAddrVec;
					StringSplit(twinModeString,",",twinModeAddrVec);
					mSpreaderAddr[iSpreader].mtwinMode.iAddress = atoi(twinModeAddrVec[0].c_str());
					mSpreaderAddr[iSpreader].mtwinMode.iBit = atoi(twinModeAddrVec[1].c_str());
					std::string unlockedStr;
					if(iniFile.ReadString(SpreaderStr[iSpreader].c_str(),"Unlocked",unlockedStr))
					{
						std::vector<std::string>unlockedAddrVec;
						StringSplit(unlockedStr,",",unlockedAddrVec);
						mSpreaderAddr[iSpreader].munlocked.iAddress = atoi(unlockedAddrVec[0].c_str());
						mSpreaderAddr[iSpreader].munlocked.iBit = atoi(unlockedAddrVec[1].c_str());
						std::string feetStr;
						if(iniFile.ReadString(SpreaderStr[iSpreader].c_str(),"Feet",feetStr))
						{
							std::vector<std::string>feetAddrVec;
							StringSplit(feetStr,",",feetAddrVec);
							for(int iFeetNum=0;iFeetNum<feetAddrVec.size();iFeetNum++)
							{
								std::vector<std::string>feetAddrGetVec;
								StringSplit(feetAddrVec[iFeetNum],"-",feetAddrGetVec);
								AddressBoolValue mAddrFeet;
								mAddrFeet.iAddress = atoi(feetAddrGetVec[1].c_str());
								mAddrFeet.iBit =atoi(feetAddrGetVec[2].c_str());
								int Feet = atoi(feetAddrGetVec[0].c_str());
								mSpreaderAddr[iSpreader].mfeet.insert(std::make_pair(Feet,mAddrFeet));
							}
							mSpreaderAddr[iSpreader].addressReady = true;
						}
					}
				}
			}
		}
	}
	//== read LiftMode address
	std::string ScrInSSMDstr;
	if(iniFile.ReadString("LIFTMODE","ScrInSSMD",ScrInSSMDstr))
	{
		std::vector<std::string>ssmdVec;
		StringSplit(ScrInSSMDstr,",",ssmdVec);
		mLiftModeAddr.ScrInSSMD.iAddress = atoi(ssmdVec[0].c_str());
		mLiftModeAddr.ScrInSSMD.iBit=atoi(ssmdVec[1].c_str());
		std::string ScrInDSMDstr;
		if(iniFile.ReadString("LIFTMODE","ScrInDSMD",ScrInDSMDstr))
		{
			std::vector<std::string>dsmdVec;
			StringSplit(ScrInDSMDstr,",",dsmdVec);
			mLiftModeAddr.ScrInDSMD.iAddress = atoi(dsmdVec[0].c_str());
			mLiftModeAddr.ScrInDSMD.iBit = atoi(dsmdVec[1].c_str());
			std::string ScrInPSMDstr;
			if(iniFile.ReadString("LIFTMODE","ScrInPSMD",ScrInPSMDstr))
			{
				std::vector<std::string>psmdVec;
				StringSplit(ScrInPSMDstr,",",psmdVec);
				mLiftModeAddr.ScrInPSMD.iAddress = atoi(psmdVec[0].c_str());
				mLiftModeAddr.ScrInPSMD.iBit =atoi(psmdVec[1].c_str());
				mLiftModeAddr.addressReady = true;
			}
		}
	}
	//===read height==
	std::string targetDistanceAddr;
	if(iniFile.ReadString("THEIGHT","TargetDistance",targetDistanceAddr))
	{
		mHeightAddr.TargetDistanceAddress = atoi(targetDistanceAddr.c_str());
		std::string hoistPositionAddr;
		if(iniFile.ReadString("THEIGHT","HoistPosition",hoistPositionAddr))
		{
			mHeightAddr.HoistPositionAddress = atoi(hoistPositionAddr.c_str());
			std::string phoistPositionAddr;
			if(iniFile.ReadString("THEIGHT","PHoistPosition",phoistPositionAddr));
			{	
				mHeightAddr.PHoistPositionAddress = atoi(phoistPositionAddr.c_str());
				std::string ptrolleyPosition;
				if(iniFile.ReadString("THEIGHT","PTrolleyPosition",ptrolleyPosition))
				{
					mHeightAddr.PTrolleyPositionAddress = atoi(ptrolleyPosition.c_str());
					std::string trolleyPosition;
					if(iniFile.ReadString("THEIGHT","TrolleyPosition",trolleyPosition))
					{
						mHeightAddr.TrolleyPositionAddress = atoi(trolleyPosition.c_str());
						mHeightAddr.addressReady = true;
					}
				}
			}

		}
	}
	//========2017/4/22  add doubleSpreader zoom value
	std::string dsAddrStr;
	if(iniFile.ReadString("DOUBLESPREADERADDR","DoubleSpreader",dsAddrStr))
	{
		std::vector<std::string>dsAddrValue;
		StringSplit(dsAddrStr,",",dsAddrValue);
		mDoubleSpreaderAddr.iAddress = std::atoi(dsAddrValue[0].c_str());
		mDoubleSpreaderAddr.iBit = std::atoi(dsAddrValue[1].c_str());
	}
	else
	{
		mDoubleSpreaderAddr.iAddress=0;
		mDoubleSpreaderAddr.iBit=0;
	}
	//=====2017/5/24 preset point call
	mPresetpointCallAddr.iAddress=0;
	mPresetpointCallAddr.iBit = 0;
	std::string prePointCallAddStr;
	if(iniFile.ReadString("PRESETPOINTCALLADDR","PresetPointCall",prePointCallAddStr))
	{
		std::vector<std::string>pcAddrValue;
		StringSplit(prePointCallAddStr,",",pcAddrValue);
		mPresetpointCallAddr.iAddress = std::atoi(pcAddrValue[0].c_str());
		mPresetpointCallAddr.iBit = std::atoi(pcAddrValue[1].c_str());
	}
	//====2017/6/9 ptz command send to cam
	int ptzSendDst=0;
	iniFile.ReadInt("PTZSENDTOCAM","Enable",ptzSendDst);
	if(ptzSendDst==1)
		mPtzCommandToCam = true;
	else
		mPtzCommandToCam = false;

	//=======获取是否通过TCP 获取PLC数据=============
	//int tcpEnable=0;
	//if(iniFile.ReadInt("TCPCONNECTPLC", "Enable", tcpEnable))
	//{
	//	if(tcpEnable==1)
	//		tcpConnect = true;
	//	else
	//		tcpConnect = false;
	//}
	//else
	//	tcpConnect = false;

	//初始化分组配置文件
	InitGroupData();
}

void CClientManager::InitModbusData()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\modbus.ini");

	string sOffSet1 = "";
	string sOffSet2 = "";
	string sOffSet3 = "";

	iniFile.ReadString("DB_SET", "db_offset1", sOffSet1);
	iniFile.ReadString("DB_SET", "db_offset2", sOffSet2);
	iniFile.ReadString("DB_SET", "db_offset3", sOffSet3);

	if (sOffSet1 != "")
	{
		std::vector<std::string> vecotr;
		StringSplit(sOffSet1, ",", vecotr);
		m_ModBusFormatInfo.nOffSet1 = std::atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum1 = std::atoi(vecotr[1].c_str());
	}
	if (sOffSet2 != "")
	{
		std::vector<std::string> vecotr;
		StringSplit(sOffSet2, ",", vecotr);
		m_ModBusFormatInfo.nOffSet2 = std::atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum2 = std::atoi(vecotr[1].c_str());
	}
	if (sOffSet3 != "")
	{
		std::vector<std::string> vecotr;
		StringSplit(sOffSet3, ",", vecotr);
		m_ModBusFormatInfo.nOffSet3 = std::atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum3 = std::atoi(vecotr[1].c_str());
	}



	std::vector<std::string> db_vecotr;
	iniFile.ReadSectionString("DB_ADDR", db_vecotr);	//多个PLC

	for (std::vector<std::string>::iterator ite = db_vecotr.begin();
		ite != db_vecotr.end(); ite++)
	{
		std::vector<std::string> strDbData;
		StringSplit(*ite, ",", strDbData);

		ModBusIp ModbusInfo;
		ModbusInfo.strIP = strDbData[1];
		ModbusInfo.nPort = atoi(strDbData[2].c_str());
		ModbusInfo.ServerKeyId = atoi(strDbData[3].c_str());
		m_GroupModBusMap.insert(std::make_pair(atoi(strDbData[0].c_str()), ModbusInfo));
	}

	std::vector<std::string> GroupArry;
	iniFile.ReadSectionString("DB_GROUP", GroupArry);	//多个PLC

	for (std::vector<std::string>::iterator ite = GroupArry.begin();
		ite != GroupArry.end(); ite++)
	{
		std::vector<std::string> strDbData;
		StringSplit(*ite, ",", strDbData);

		ModBusDB ModbusInfo;
		if (strDbData.size() >= 2)
			ModbusInfo.addr1 = atoi(strDbData[1].c_str());
		else
			ModbusInfo.addr1 = -1;
		if (strDbData.size() >= 3)
			ModbusInfo.addr3 = atoi(strDbData[2].c_str());
		else
			ModbusInfo.addr3 = -1;
		if (strDbData.size() >= 4)
			ModbusInfo.addr4 = atoi(strDbData[3].c_str());
		else
			ModbusInfo.addr4 = -1;
		m_GroupModBusDBMap.insert(std::make_pair(atoi(strDbData[0].c_str()), ModbusInfo));
	}

	iniFile.ReadInt("DB_SET", "screen_mode", m_ModBusFormatInfo.nModeMax);
	iniFile.ReadInt("DB_SET", "screen_num", m_ModBusFormatInfo.nScreenNum);

	iniFile.ReadInt("DB_SET", "camera_num", m_ModBusFormatInfo.nPtzNum);	

	iniFile.ReadInt("DB_SET", "ipcStateAddr", m_iModbusIpcStateOffSet);

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////PLC SCREEN ADDR//////////////////////////*/

	/*switch screen plc address */
	std::vector<std::string> ScreenAddArray;
	iniFile.ReadSectionString("MODBUSSCREENADDR", ScreenAddArray);

	/* SCREENID<<8|MODE,此为唯一性*/
	m_ModbusScreenAddMap.empty();
	for(std::vector<std::string>::iterator iteScreenAddr = ScreenAddArray.begin(); 
		iteScreenAddr != ScreenAddArray.end(); iteScreenAddr++)
	{
		std::vector<std::string> vectorScreenAdd;
		StringSplit(*iteScreenAddr, ",", vectorScreenAdd);

		unsigned short iScreenId = atoi(vectorScreenAdd[0].c_str()) & 0xff;
		unsigned short iSCREENMode = atoi(vectorScreenAdd[1].c_str()) & 0xff;

		ModbusADDR sAddr;

		sAddr.iAddr = atoi(vectorScreenAdd[2].c_str()) & 0xff;
		sAddr.iPriority = atoi(vectorScreenAdd[3].c_str()) & 0xff;

		int iScreenModeMap = iScreenId<<8|iSCREENMode;/* 高８位，低８位*/

		m_ModbusScreenAddMap.insert(std::make_pair(iScreenModeMap,sAddr));
	}

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////MODBUS IPC ADDR//////////////////////////*/

	/*switch screen plc address */
	std::vector<std::string> vectorModbusIPCAddrArray;
	iniFile.ReadSectionString("MODBUSIPCADDR", vectorModbusIPCAddrArray);

	/* IPC,此为唯一性*/
	m_ModbusIPCAddrMap.empty();
	for(std::vector<std::string>::iterator itePLCIPCAddr = vectorModbusIPCAddrArray.begin(); 
		itePLCIPCAddr != vectorModbusIPCAddrArray.end(); itePLCIPCAddr++)
	{
		std::vector<std::string> vectorIPCAddr;
		StringSplit(*itePLCIPCAddr, ",", vectorIPCAddr);

		//PLCIPC1=1,5 ;IPCID,ADDROFFSET
		unsigned short iIPCAddrId = atoi(vectorIPCAddr[0].c_str()) & 0xff;		
		unsigned short iAddr = atoi(vectorIPCAddr[1].c_str()) & 0xff;

		m_ModbusIPCAddrMap.insert(std::make_pair(iIPCAddrId,iAddr));
	}


	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////GROUP,PC,MODE,SCREENID,SPLITWINDOW,NO:IPCID,NO:IPCID//////////////////////////*/

	m_ModbuszoomFeet2AddrMap.empty();
	std::vector<std::string> ZoomFeetVector;
	iniFile.ReadSectionString("MODBUSZOOMADDR", ZoomFeetVector);
	for(std::vector<std::string>::iterator it = ZoomFeetVector.begin(); it != ZoomFeetVector.end(); it++)
	{
		std::vector<std::string> feetInfoVec;
		StringSplit(*it, ",", feetInfoVec);

		int iAddr = atoi(feetInfoVec[1].c_str());
		m_ModbuszoomFeet2AddrMap.insert(std::make_pair(atoi(feetInfoVec[0].c_str()), iAddr));
	}

	//////////////////zoom/////////////////////////////
	// 之前版本的配置文件中存在拼写错误(Height误写为Hight)，为了兼容老版本,增加一两种读取方式
	if (!iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHeight", m_iMaxHeight))
		iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHight", m_iMaxHeight);
	if (!iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MinHight", m_iMinHeight))
		m_iMinHeight = -25;
	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxFeet", m_iMaxFeet);

	//iniFile.ReadInt("MODBUSZOOMHEIGHTADDR", "HEIGHT", m_iModbusZoomHeightAddr);

	iniFile.ReadInt("SWITCHGROUPADDR", "SwitchGroup", m_iModbusSwitchGroupAddr);

	//=============根据场景Zoom 2017/1/19=====================================//
	int ret = 0;
	iniFile.ReadInt("ZOOMCONFIG","Enable",ret);
	if(ret == 1)
		m_ZoomChoose = true;
	else
		m_ZoomChoose = false;
	m_ZoomChooseAddr =195;
	iniFile.ReadInt("ZOOMCONFIG","Addr",m_ZoomChooseAddr);
	//=========2017/6/8 zoomOnly for 塞内加尔
	int zoomOnlyInt = 0;
	iniFile.ReadInt("ZOOMONLYSET","Enable",zoomOnlyInt);
	if(zoomOnlyInt==1)
	{
		mModbusZoomOnly = true;
		iniFile.ReadInt("ZOOMONLYSET","Height",mZoomOnlyHeightAddr);
		iniFile.ReadInt("ZOOMONLYSET","Feet",mZoomOnlyFeetAddr);
	}
	else
	{
		mModbusZoomOnly = false;
	}

	//初始化分组配置文件
	InitGroupData();
}

//初始化分组信息
void CClientManager::InitGroupData()
{
	CClientManager::GetInstance()->groupDataLock.acquire();
	char szlog[MAX_STR_LEN]={0};
	_snprintf(szlog,MAX_STR_LEN-1,"load group data configure...");
	CClientManager::GetInstance()->Showlog2Dlg(szlog);

	int iSwitchGroup = 0;
	CFileFind filefind;
	std::string strFilePath(GetCurrentPath() + _T("\\config\\*.*"));
	CString sFilePath = strFilePath.c_str();

	m_Group2PcMap.empty();
	m_TvWallIp2CutMapInfo.empty(); //ip和port为key，value为该tv_wall各个屏幕的预案
	m_GroupPCScreen.empty();
	m_GroupIpcId.empty();
	m_MapFreeCutScreen.empty();

	m_Group2PcMap.clear();
	m_TvWallIp2CutMapInfo.clear();
	m_switchWithHeigt.clear();
	m_GroupIpcId.clear();
	m_MapFreeCutScreen.clear();
	//zoom
	m_ZoomIpc2GroupMap.empty();		
	m_zoomPuid2FeetHeightMap.empty();
	//m_PuidDefaultZoomMap.empty();
	m_heightScrrenModeInfo.clear();
	m_GroupPCScreen.clear();
	m_PtzIpc2GroupMap.clear();
	mZoomMualReset.clear();
	m_ZoomIpc2GroupMap.clear();
	m_zoomPuid2FeetHeightMap.clear();
	m_PuidDefaultPtzPointMap.clear();
	m_IPCgroupID.clear();
	m_GroupScreenMap.clear();
	//======textBar=======
	for(IpcText::iterator iter = mIpcText.begin();iter!=mIpcText.end();iter++)
	{
		iter->second.clear();
	}
	mIpcText.clear();
	//========textBar Message 2017/4/18===========
	mIpcPositionMap.clear();
	mTextBarInfo.clear();

	//===2017/4/22 双吊具zoom映射
	m_zoomPuid2FeetHeightMap2.clear();
	//==2017/5/3 特殊模式切屏
	mFreeModeMap.clear();
	//==2017/5/24 preset point call
	mIpcPresetPointMsgMap.clear();

	for(PtzPuid2Ptz::iterator iter =m_ptzPuid2FeetHeightPresentPMap.begin(); iter!=m_ptzPuid2FeetHeightPresentPMap.end();)
	{
		delete[] iter->second;
		m_ptzPuid2FeetHeightPresentPMap.erase(iter++);
	}
	m_ptzPuid2FeetHeightPresentPMap.clear();
	if (filefind.FindFile(sFilePath, 0))
	{
		BOOL bBool = TRUE;
		while(bBool)
		{
			bBool = filefind.FindNextFile();
			if (filefind.IsDots())
			{
				continue;
			}
			CString sFileName = filefind.GetFileName();
			if (sFileName != "General.ini" && sFileName != "modbus.ini")
			{
				std::string strFileName = sFileName.GetBuffer(0);
				CIniFile iniFile(GetCurrentPath() + "\\config\\"+strFileName);
				sFileName.ReleaseBuffer();
				/*switch screen*/
				//m_TvWallIp2CutMapInfo.clear();
				std::vector<std::string> GroupArray, ScreenArray;
				iniFile.ReadSectionString("GROUP", GroupArray);
				iniFile.ReadSectionString("SCREENCAMERA", ScreenArray);

				/*/////GROUP,SCREENID 唯一性,给组编号和每组的屏幕编号,根据这些获取到后三者参数/////////////////
				/////GROUP,SCREENID,IP,PORT,TVSSCREENID/////////*/

				/* Group<<8|SCREENID,此为唯一性*/


				for(std::vector<std::string>::iterator ite = GroupArray.begin(); ite != GroupArray.end(); ite++)
				{
					std::vector<std::string> Group;
					StringSplit(*ite, ",", Group);

					//GroupScreen1=1,1,172.16.2.11,90000,1 Group,pc,ip,port

					unsigned short iGroup = atoi(Group[0].c_str()) & 0xff;
					unsigned short iSCREENID = atoi(Group[1].c_str()) & 0xff;

					unsigned short iTVSSCREENID = atoi(Group[4].c_str()) & 0xff;
					int iGroup_PC = iGroup<<8|iSCREENID;/* 高８位，低８位*/
					TvWallInfo info;
					info.Ip = Group[2];
					info.Port = atoi(Group[3].c_str());
					info.iTVSSCREENID = iTVSSCREENID;

					m_Group2PcMap.insert(std::make_pair(iGroup_PC,info));

					StateScreenId2Cut ScreenId2CutMap;
					m_TvWallIp2CutMapInfo.insert(std::make_pair(Group[2] + "," + Group[3], ScreenId2CutMap));
				}
				//====2017/5/24 preset point call
				std::vector<std::string>ipcPresetPointStrVec;
				iniFile.ReadSectionString("PRESETPOINTCALLCAM",ipcPresetPointStrVec);
				for(std::vector<std::string>::iterator iterP = ipcPresetPointStrVec.begin();
					iterP!=ipcPresetPointStrVec.end();iterP++)
				{
					int groupIndex=0;
					ipcPresetPointMsgVec camPresetPointVec;
					std::vector<std::string>camInfo;
					StringSplit(*iterP,",",camInfo);
					for(int i=0;i<camInfo.size();i++)
					{
						if(i==0)
							groupIndex = std::atoi(camInfo[i].c_str());
						else
						{
							ipcPresetPointMsg camPresetPoint;
							std::vector<std::string>oneCamInfo;
							StringSplit(camInfo[i],"-",oneCamInfo);
							camPresetPoint.ipcIndex = std::atoi(oneCamInfo[0].c_str());
							camPresetPoint.pointStatuse = true;
							camPresetPoint.erroTime=0;
							camPresetPoint.presetPointName = oneCamInfo[1];
							camPresetPointVec.push_back(camPresetPoint);
						}
					}
					mIpcPresetPointMsgMap.insert(std::make_pair(groupIndex,camPresetPointVec));

					mPresetPointCallResetMap.insert(std::make_pair(groupIndex,true));
				}

				//====2017/5/4 特殊模式点切屏 ===========
				std::vector<std::string>freeModeStrVec;
				iniFile.ReadSectionString("FREEMODESWITCH",freeModeStrVec);
				freeModeVec freeModeGroupV;	
				int freeGroup=0;
				for(std::vector<std::string>::iterator iterFree=freeModeStrVec.begin();
					iterFree!=freeModeStrVec.end();iterFree++)
				{
					freeModeSt freeModeG;
					if(sscanf(iterFree->c_str(),"%d,%d-%d,%d,%d",&freeGroup,&freeModeG.modeAddr.iAddress,&freeModeG.modeAddr.iBit,
						&freeModeG.screenID,&freeModeG.mode)==5)

					{
						freeModeGroupV.push_back(freeModeG);
					}
					//std::vector<std::string>strVec;
					//StringSplit(*iterFree,",",strVec);
					//if(strVec.size()==4)//group,srceenID,addr,mode
					//{
					//	freeModeG.screenID = std::atoi(strVec[2].c_str());
					//	sscanf(ite->c_str(),"%d-%d",&freeModeG.modeAddr.iAddress,&freeModeG.modeAddr.iBit);
					//}				
					//freeGroup = std::atoi(strVec[0].c_str());
				}
				if(freeModeGroupV.size()>0)
					mFreeModeMap.insert(std::make_pair(freeGroup,freeModeGroupV));

				//================随高度切屏==================================================
				std::string switchFlagS;
				iniFile.ReadString("SWITCHSCREENWITHHEIGHT","Enable",switchFlagS);
				std::vector<std::string> switchFlag;
				StringSplit(switchFlagS, ",",switchFlag);

				if(switchFlag.size()>1)
				{
					if(atoi(switchFlag[1].c_str())==1)
						m_switchWithHeigt.insert(std::make_pair(atoi(switchFlag[0].c_str()),true));
					else 
						m_switchWithHeigt.insert(std::make_pair(atoi(switchFlag[0].c_str()),false));
					std::vector<std::string>heightScreenArray;
					iniFile.ReadSectionString("HEIGHTMODE",heightScreenArray);
					for(std::vector<std::string>::iterator ite = heightScreenArray.begin(); 
						ite != heightScreenArray.end(); ite++)
					{
						//						std::vector<std::string> heightScreenV;						
						// 						unsigned short iGroup = atoi(heightScreenV[0].c_str())&0xff;
						// 						unsigned short iHeightMin = atoi(heightScreenV[1].c_str())&0xff;
						// 						unsigned short iHeightMax = atoi(heightScreenV[2].c_str())&0xff;
						// 						unsigned short iScreenID = atoi(heightScreenV[3].c_str())&0xff;
						// 						unsigned short iMode = atoi(heightScreenV[4].c_str())&0xff;
						// 						heigtScreenS.group = iGroup;
						// 						heigtScreenS.minHeight = iHeightMin;
						// 						heigtScreenS.maxHeight = iHeightMax;
						// 						heigtScreenS.screenID = iScreenID;
						// 						heigtScreenS.mode = iMode;

						// 直接使用sscanf大字符串获取相关值
						// 李雄高 2017.01.04
						HeightScreenMode heigtScreenS;
						if (sscanf(ite->c_str(),
							"%d,%d,%d,%d,%d",
							&heigtScreenS.group,
							&heigtScreenS.minHeight,
							&heigtScreenS.maxHeight,
							&heigtScreenS.screenID,
							&heigtScreenS.mode) == 5)
							m_heightScrrenModeInfo.push_back(heigtScreenS);
					}
				}
				//=============================================================================

				/////////////////////////////ipclist////////////////////////////////////////////
				///////////////////////////////////////////////////////////////////////////////
				std::vector<std::string> GroupIPCIDlist;
				iniFile.ReadSectionString("CARAMPTZ", GroupIPCIDlist);

				for(std::vector<std::string>::iterator itIPCIdList = GroupIPCIDlist.begin(); 
					itIPCIdList != GroupIPCIDlist.end();
					itIPCIdList++)
				{
					std::vector<std::string> GroupIPCId;
					StringSplit(*itIPCIdList, ",", GroupIPCId);

					//group1=1,1,11000000000000,192.168.1.108,34567

					unsigned short iGroup = atoi(GroupIPCId[0].c_str()) & 0xff;
					unsigned short iIPCID = atoi(GroupIPCId[1].c_str()) & 0xff;

					int iGroup_IPCID = iGroup<<8|iIPCID;/* 高８位，低８位*/
					IPCIDInfo infoIPCId;
					infoIPCId.Ip = GroupIPCId[3];
					infoIPCId.Port = atoi(GroupIPCId[4].c_str());
					infoIPCId.strPUID = GroupIPCId[2];

					if (GroupIPCId.size() >= 10)
					{
						infoIPCId.stream_type = atoi(GroupIPCId[9].c_str());	//主副码流
					}
					else
					{
						infoIPCId.stream_type = 1;	//默认主码流	
					}
					if (GroupIPCId.size() >= 11)
					{
						infoIPCId.istep = atoi(GroupIPCId[10].c_str());
					}
					else
					{
						infoIPCId.istep = 4;   //步长4	
					}

					m_GroupIpcId.insert(std::make_pair(iGroup_IPCID,infoIPCId));
				}

				/*//GROUP,SCREENID,MODE<PLC Value>, WINDOWS,WINDOWSINDEX,IPC-IPC-IPC/
				///ScreenCamera0=1,1,0,4,1,11000000000000-1000000000000-1000000000000-1000000000000/////////*/

				for (std::vector<std::string>::iterator ite = ScreenArray.begin(); ite != ScreenArray.end(); ite++)
				{
					ScreenModeInfo stScreenMode;

					std::vector<std::string> Group;
					StringSplit(*ite, ",", Group);

					unsigned short iGroup = atoi(Group[0].c_str()) & 0xff;
					unsigned short iSCREENID = atoi(Group[1].c_str()) & 0xff;
					int iMode = atoi(Group[2].c_str()) & 0xff ;
					/* Group<<16|SCREENID<<8|MODE,此为唯一性*/
					int iPCSreenMode = (iGroup<<16)|(iSCREENID<<8)|iMode;

					/*画面分割和列表*/
					ScreenModeInfo stModeInfo;
					{
						int iWindows = atoi(Group[3].c_str()) & 0xff;
						int iWindowsIndex = atoi(Group[4].c_str()) & 0xff;

						stModeInfo.splitNum = iWindows;
						stModeInfo.windowsIndex = iWindowsIndex;

						/*IPLIST*/
						std::string strIPCList = Group[5];
						std::vector<std::string> strIPCinfo;
						/* 11000000000000-1000000000001-1000000000002-1000000000004*/
						StringSplit(strIPCList, "-", strIPCinfo);
						int istIPCInfosize = strIPCinfo.size();

						WINDOWS_IPCID stIPCID2Windows;

						for (int i = 0; i< istIPCInfosize; i++)
						{
							//========Text bar 2017/4/7==================
							std::size_t found = strIPCinfo[i].find("T");
							if(found!=std::string::npos)
							{
								std::string barMode = strIPCinfo[i].substr(1,1);
								mIpcTextScreen.insert(std::make_pair(iPCSreenMode,std::atoi(barMode.c_str())));
								continue;
							}

							//std::size_t found = strIPCinfo[i].find(":");
							//if(found!=std::string::npos)
							//{
							//	std::vector<std::string>textIpcRead;
							//	StringSplit(strIPCinfo[i],":",textIpcRead);
							//	strIPCinfo[i] = textIpcRead[0];
							//	if(textIpcRead[1].substr(0,1)=="T")
							//	{
							//		IpcText::iterator iterText = mIpcText.find(iGroup);//查找分组
							//		if(iterText!=mIpcText.end())
							//		{
							//			for(int iText=0;iText<iterText->second.size();iText++)
							//			{
							//				if(iterText->second[iText]==textIpcRead[0])
							//					continue;
							//				else
							//				{
							//					iterText->second.push_back(textIpcRead[0]);
							//					//=====add textBar mode 2017/4/17 =====
							//					std::string	tMode = textIpcRead[1].substr(1); 
							//					mTextBarInfo.insert(std::make_pair(textIpcRead[0],std::atoi(tMode.c_str())));
							//				}
							//			}
							//		}
							//		else
							//		{
							//			std::vector<std::string>puidTextVec;
							//			puidTextVec.push_back(textIpcRead[0]);
							//			mIpcText.insert(std::make_pair(iGroup,puidTextVec));//group,puid
							//			//===========textBarMode 2017/4/17================
							//			std::string tMode = textIpcRead[1].substr(1);
							//			//mTextBarInfo.insert(std::make_pair(textIpcRead[0],std::atoi(tMode.c_str())));
							//		}
							//	}
							//}
							//=============================================
							stIPCID2Windows.winNum = i + 1;
							stIPCID2Windows.DeviceId = strIPCinfo[i];
							for(GroupIPCIDMap::iterator it = m_GroupIpcId.begin(); it != m_GroupIpcId.end();it++)
							{
								if (strIPCinfo[i] == it->second.strPUID)
								{
									stIPCID2Windows.stream_type = it->second.stream_type;
									break;
								}
							}

							stModeInfo.winId.push_back(stIPCID2Windows);
						}	
					}
					m_GroupPCScreen.insert(std::make_pair(iPCSreenMode,stModeInfo));
				}
				//===== add ipcPosition 2017/4/18=====
				std::vector<std::string>ipcPositionStrVec;
				iniFile.ReadSectionString("CAMERAPOSITION", ipcPositionStrVec);
				ipcPosition mGroupIpcPosition;
				textBarMonitorID monitorIDT;
				for(std::vector<std::string>::iterator iterPosition=ipcPositionStrVec.begin();iterPosition!=ipcPositionStrVec.end();iterPosition++)
				{
					std::vector<std::string> positionStrV;
					StringSplit((*iterPosition).c_str(),",",positionStrV);
					int groupNum = atoi(positionStrV[0].c_str());
					for(int positionIndex=1;positionIndex<positionStrV.size();positionIndex++)
					{
						std::vector<std::string>ipcP;
						StringSplit(positionStrV[positionIndex].c_str(),"-",ipcP);
						mGroupIpcPosition.insert(std::make_pair(ipcP[0],ipcP[1]));

						//if(ipcP.size()!=3)
						//	break;
						//mGroupIpcPosition.insert(std::make_pair(ipcP[0],ipcP[2]));
						//if(ipcP[2]=="WS")
						//	monitorIDT.wsMonitorID = std::atoi(ipcP[1].c_str());
						//else if(ipcP[2]=="LS")
						//	monitorIDT.lsMonitorID = std::atoi(ipcP[1].c_str());
						//else if(ipcP[2]=="PS")
						//	monitorIDT.psMonitorID = std::atoi(ipcP[1].c_str());
					}
					mIpcPositionMap.insert(std::make_pair(groupNum,mGroupIpcPosition));
					mTextBarMonitorID.insert(std::make_pair(groupNum,monitorIDT));
				}
				/////////////////////== 2016/12/18 锁头跟随==///////////////////////
				///////////////////////////////
				std::vector<std::string> GroupPtzVector;
				iniFile.ReadSectionString("CAMERAFOLLOW",GroupPtzVector);
				for(std::vector<std::string>::iterator it = GroupPtzVector.begin(); it != GroupPtzVector.end();it++)
				{
					std::vector<std::string>pcInfo;
					StringSplit(*it,",",pcInfo);
					unsigned short iGroup = atoi(pcInfo[0].c_str());
					std::vector<std::string>ipcInfo;
					StringSplit(pcInfo[1],"-",ipcInfo);
					m_PtzIpc2GroupMap.insert(std::make_pair(iGroup,ipcInfo));
				}
				//Ptz值得映射
				std::vector<std::string>PtzFeetHeightVector;
				iniFile.ReadSectionString("CAMERAPTZHEIGHT",PtzFeetHeightVector);
				for(std::vector<std::string>::iterator it=PtzFeetHeightVector.begin();it!=PtzFeetHeightVector.end();it++)
				{
					std::vector<std::string>feetHeightVec;
					StringSplit(*it,",",feetHeightVec);
					std::string sPuid = feetHeightVec[0];
					int iDefaultValue = atoi(feetHeightVec[1].c_str());
					m_PuidDefaultPtzPointMap.insert(std::make_pair(sPuid, iDefaultValue));
					int ipcgroupId = atoi(feetHeightVec[2].c_str());
					m_IPCgroupID.insert(std::make_pair(sPuid,ipcgroupId));
					int *numPresentP = new  int[(m_iMaxFeet+1)*(m_iMaxHeight+1)];
					memset(numPresentP,0,sizeof(int)*(m_iMaxFeet+1)*(m_iMaxHeight+1));
					m_ptzPuid2FeetHeightPresentPMap.insert(std::make_pair(sPuid,numPresentP));
					for (int i = 3; i < feetHeightVec.size(); i++)
					{
						//std::vector<std::string> feet_height_Vec;
						//StringSplit(feetHeightVec[i], "-", feet_height_Vec);
						//int iFeet = atoi(feet_height_Vec[0].c_str());
						//int iLowHeight = atoi(feet_height_Vec[1].c_str());
						//int iHighHeight = atoi(feet_height_Vec[2].c_str());
						//int iPtzPresentPValue = atoi(feet_height_Vec[3].c_str());
						// 直接使用sscanf大字符串获取相关值
						// 李雄高 2017.01.04
						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iPtzPresentPValue = 0;
						if (sscanf(feetHeightVec[i].c_str(),"%d-%d-%d-%d",&iFeet,&iLowHeight,&iHighHeight,&iPtzPresentPValue) == 4)
							for (int j = iLowHeight; j <= iHighHeight; j++)
							{
								numPresentP[iFeet*m_iMaxHeight + j] = iPtzPresentPValue;
							}
					}		
				}
				//////////// read zoom manual reset camera //////////////////////////
				std::vector<std::string> zoomResetVector;
				iniFile.ReadSectionString("MANUALZOOMRESET",zoomResetVector);
				for(std::vector<std::string>::iterator iter=zoomResetVector.begin();iter!=zoomResetVector.end();iter++)
				{
					std::vector<std::string> resetInfo;
					StringSplit(*iter,",",resetInfo);
					unsigned short iGroup = atoi(resetInfo[0].c_str());
					std::vector<std::string> ipcIDstr;
					StringSplit(resetInfo[1],"-",ipcIDstr);
					mZoomMualReset.insert(std::make_pair(iGroup,ipcIDstr));
				}
				//////////////////////////////////////////////////////////////////////////
				////////////////////////////* zoom mode*//////////////////////////////////////////////////////
				std::vector<std::string> GroupZoomVector;
				iniFile.ReadSectionString("CAMERAZOOM", GroupZoomVector);
				for(std::vector<std::string>::iterator it = GroupZoomVector.begin(); it != GroupZoomVector.end(); it++)
				{
					std::vector<std::string> pcInfo;
					StringSplit(*it, ",", pcInfo);
					unsigned short iGroup = atoi(pcInfo[0].c_str());

					//ZoomIpcVector zoomIpcInfo;

					std::vector<std::string> ipcInfo;
					StringSplit(pcInfo[1], "-", ipcInfo);
					//zoomIpcInfo.zoomIpcVec = ipcInfo;
					m_ZoomIpc2GroupMap.insert(std::make_pair(iGroup,ipcInfo));
				}


				//zoom值的映射
				// 				std::vector<std::string> ZoomFeetHeightVector;
				// 				iniFile.ReadSectionString("CAMERAZOOMHEIGHT", ZoomFeetHeightVector);
				// 				for(std::vector<std::string>::iterator it = ZoomFeetHeightVector.begin(); it != ZoomFeetHeightVector.end(); it++)
				// 				{
				// 					std::vector<std::string> feetHeightVec;
				// 					StringSplit(*it, ",", feetHeightVec);
				// 					std::string sPuid = feetHeightVec[0];
				// 					int iDefaultValue = atoi(feetHeightVec[1].c_str());
				// 					m_PuidDefaultZoomMap.insert(std::make_pair(sPuid, iDefaultValue));
				// 
				// 					int *num = new  int[(m_iMaxFeet+1)*(m_iMaxHeight+1)];
				// 					memset(num, 0, (m_iMaxFeet+1)*(m_iMaxHeight+1));
				// 					m_zoomPuid2FeetHeightMap.insert(std::make_pair(sPuid, num));
				// 
				// 					for (int i = 2; i < feetHeightVec.size(); i++)
				// 					{
				// 						// 直接使用sscanf大字符串获取相关值
				// 						// 李雄高 2017.01.04
				// 						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iZoomValue = 0;
				// 						if (sscanf(feetHeightVec[i].c_str(),"%d-%d-%d-%d",&iFeet,&iLowHeight,&iHighHeight,&iZoomValue) == 4)
				// 						for (int j = iLowHeight; j <= iHighHeight; j++)
				// 						{
				// 							num[iFeet*m_iMaxHeight + j] = iZoomValue;
				// 						}
				// 					}
				// 				}
				struct  ZoomPuid2Zoom2
				{
					int		nDefaultZoom;
					int		*pZoomValueArray;
				};

				std::vector<std::string> ZoomFeetHeightVector;
				iniFile.ReadSectionString("CAMERAZOOMHEIGHT", ZoomFeetHeightVector);
				for(std::vector<std::string>::iterator it = ZoomFeetHeightVector.begin(); it != ZoomFeetHeightVector.end(); it++)
				{
					std::vector<std::string> feetHeightVec;
					StringSplit(*it, ",", feetHeightVec);
					int nValueIndex = 0;

					// 取得高度索引
					// 修改高度切屏的存储方式
					// 以高度索引找到相机的Camer-Feet-height-zoom集，根据相机ID找到Feet-height-zoom集，根据Feet值找到Height-Zoom集
					int nHeightIndex =  atoi(feetHeightVec[nValueIndex ++].c_str());
					ZoomPuid2ZoomPtr ZoomPtr;

					// 以高度索引找到相机的Camer-Feet-height-zoom集
					if(nHeightIndex>10000) //兼容原配置
					{
						nHeightIndex = 0;
						nValueIndex--;
					}

					map<INT, ZoomPuid2ZoomPtr>::iterator itFindHeight = m_zoomPuid2FeetHeightMap.find(nHeightIndex);
					if (itFindHeight != m_zoomPuid2FeetHeightMap.end())
					{
						ZoomPtr = itFindHeight->second;
					}
					else
					{
						ZoomPtr = ZoomPuid2ZoomPtr(new ZoomPuid2Zoom);
						m_zoomPuid2FeetHeightMap.insert(std::make_pair<INT, ZoomPuid2ZoomPtr>(nHeightIndex,ZoomPtr));
					}

					std::string sPuid = feetHeightVec[nValueIndex ++];
					int nDefaultZoom = atoi(feetHeightVec[nValueIndex ++].c_str());
					// int nArrayCount = (m_iMaxFeet+1)*(m_iMaxHeight+1);
					int nArrayCount = m_iMaxHeight - m_iMinHeight + 1;
					//根据相机ID找到Feet-height-zoom集
					ZoomPuid2Zoom::iterator itFindCamera = ZoomPtr->find(sPuid);
					FeetHeightZoomValueMapPtr FHZVMPtr;
					if (itFindCamera != ZoomPtr->end())
					{
						FHZVMPtr = itFindCamera->second;
					}
					else
					{
						FHZVMPtr = shared_ptr<FeetHeightZoomValueMap>(new FeetHeightZoomValueMap());
						ZoomPtr->insert(pair<string, FeetHeightZoomValueMapPtr>(sPuid,FHZVMPtr));
					}
					// 一个相机的一组高度-Zoom值
					// HeightZoomValuePtr HZVPtr = HeightZoomValuePtr(new HeightZoomValue(nDefaultZoom,nArrayCount));
					//ZoomPtr->insert(std::make_pair(sPuid, HZVPtr));

					for (int i = nValueIndex; i < feetHeightVec.size(); i++)
					{
						// 直接使用sscanf从字符串获取相关值
						// 李雄高 2017.01.04
						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iZoomValue = 0;
						if (sscanf(feetHeightVec[i].c_str(),"%d-%d-%d-%d",&iFeet,&iLowHeight,&iHighHeight,&iZoomValue) == 4)
						{
							// 根据Feet值找到Height-Zoom集
							FeetHeightZoomValueMap::iterator itFindFeet = FHZVMPtr->find(iFeet);
							HeightZoomValuePtr HZVPtr;
							if (itFindFeet != FHZVMPtr->end())
								HZVPtr = itFindFeet->second;
							else
							{
								HZVPtr = HeightZoomValuePtr(new HeightZoomValue(nDefaultZoom,nArrayCount));
								FHZVMPtr->insert(pair<int, HeightZoomValuePtr>(iFeet,HZVPtr));
							}

							for (int j = iLowHeight; j <= iHighHeight; j++)
								HZVPtr->pZoomValueArray[j-m_iMinHeight] = iZoomValue;
						}
					}

				}

				//=======2017/4/22  双吊具zoom 映射====
				iniFile.ReadSectionString("CAMERAZOOMHEIGHT2", ZoomFeetHeightVector);
				for(std::vector<std::string>::iterator it = ZoomFeetHeightVector.begin(); it != ZoomFeetHeightVector.end(); it++)
				{
					std::vector<std::string> feetHeightVec;
					StringSplit(*it, ",", feetHeightVec);
					int nValueIndex = 0;

					// 取得高度索引
					// 修改高度切屏的存储方式
					// 以高度索引找到相机的Camer-Feet-height-zoom集，根据相机ID找到Feet-height-zoom集，根据Feet值找到Height-Zoom集
					int nHeightIndex =  atoi(feetHeightVec[nValueIndex ++].c_str());
					ZoomPuid2ZoomPtr ZoomPtr;

					// 以高度索引找到相机的Camer-Feet-height-zoom集
					map<INT, ZoomPuid2ZoomPtr>::iterator itFindHeight = m_zoomPuid2FeetHeightMap2.find(nHeightIndex);
					if (itFindHeight != m_zoomPuid2FeetHeightMap2.end())
					{
						ZoomPtr = itFindHeight->second;

					}
					else
					{
						ZoomPtr = ZoomPuid2ZoomPtr(new ZoomPuid2Zoom);
						m_zoomPuid2FeetHeightMap2.insert(std::make_pair<INT, ZoomPuid2ZoomPtr>(nHeightIndex,ZoomPtr));
					}

					std::string sPuid = feetHeightVec[nValueIndex ++];
					int nDefaultZoom = atoi(feetHeightVec[nValueIndex ++].c_str());
					// int nArrayCount = (m_iMaxFeet+1)*(m_iMaxHeight+1);
					int nArrayCount = m_iMaxHeight - m_iMinHeight + 1;
					//根据相机ID找到Feet-height-zoom集
					ZoomPuid2Zoom::iterator itFindCamera = ZoomPtr->find(sPuid);
					FeetHeightZoomValueMapPtr FHZVMPtr;
					if (itFindCamera != ZoomPtr->end())
					{
						FHZVMPtr = itFindCamera->second;
					}
					else
					{
						FHZVMPtr = shared_ptr<FeetHeightZoomValueMap>(new FeetHeightZoomValueMap());
						ZoomPtr->insert(pair<string, FeetHeightZoomValueMapPtr>(sPuid,FHZVMPtr));
					}
					// 一个相机的一组高度-Zoom值
					// HeightZoomValuePtr HZVPtr = HeightZoomValuePtr(new HeightZoomValue(nDefaultZoom,nArrayCount));
					//ZoomPtr->insert(std::make_pair(sPuid, HZVPtr));

					for (int i = nValueIndex; i < feetHeightVec.size(); i++)
					{
						// 直接使用sscanf从字符串获取相关值
						// 李雄高 2017.01.04
						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iZoomValue = 0;
						if (sscanf(feetHeightVec[i].c_str(),"%d-%d-%d-%d",&iFeet,&iLowHeight,&iHighHeight,&iZoomValue) == 4)
						{
							// 根据Feet值找到Height-Zoom集
							FeetHeightZoomValueMap::iterator itFindFeet = FHZVMPtr->find(iFeet);
							HeightZoomValuePtr HZVPtr;
							if (itFindFeet != FHZVMPtr->end())
								HZVPtr = itFindFeet->second;
							else
							{
								HZVPtr = HeightZoomValuePtr(new HeightZoomValue(nDefaultZoom,nArrayCount));
								FHZVMPtr->insert(pair<int, HeightZoomValuePtr>(iFeet,HZVPtr));
							}

							for (int j = iLowHeight; j <= iHighHeight; j++)
								HZVPtr->pZoomValueArray[j-m_iMinHeight] = iZoomValue;
						}
					}
				}
				//=================随场景启用变焦 2017/1/19===================
				if(m_ZoomChoose)
				{
					std::vector<std::string> cameraZoomCVector;
					iniFile.ReadSectionString("CAMERAZOOMCONFIG",cameraZoomCVector);
					for(std::vector<std::string>::iterator it = cameraZoomCVector.begin(); it != cameraZoomCVector.end();it++)
					{
						std::vector<std::string>pcInfo;
						StringSplit(*it,",",pcInfo);
						//unsigned short iGroup = atoi(pcInfo[0].c_str());
						for(std::vector<std::string>::iterator itSize = pcInfo.begin();
							itSize!=pcInfo.end();itSize++)
						{
							std::vector<std::string>ipcInfo;
							StringSplit(*itSize,"-",ipcInfo);
							int retS = atoi(ipcInfo[1].c_str()); 
							m_zoomCameraC.insert(std::make_pair(ipcInfo[0],retS));
						}
					}
				}
				//////////////////////////自由分屏//////////////////////////////////////////////
				std::vector<std::string> FreeCutScreenVector;
				iniFile.ReadSectionString("FREECUTSCREEN", FreeCutScreenVector);
				int monitorID = 0;
				iSwitchGroup++;
				FreeCutInfo sFreeCutInfo;
				int nScreenId;
				for (std::vector<std::string>::iterator ite = FreeCutScreenVector.begin(); ite != FreeCutScreenVector.end(); ite++)
				{
					std::vector<std::string> CutGroup;
					StringSplit(*ite, ",", CutGroup);

					unsigned int iIndex = atoi(CutGroup[2].c_str()) & 0xff;		//组序号
					unsigned int iGroup = atoi(CutGroup[0].c_str()) & 0xff;		//组号

					CutScreenInfo CutScreen;
					unsigned short iOffSet = atoi(CutGroup[3].c_str()) & 0xff;
					CutScreen.OffSet = iOffSet;
					CutScreen.IPCIndex = 0;
					CutScreen.ScreenID = atoi(CutGroup[1].c_str()) & 0xff;
					//CutScreen.monitor_ID = ++monitorID;
					//2017/5/5 兼容多屏
					CutScreen.monitor_ID = iIndex;

					CutScreen.SwitchGroup = iSwitchGroup;
					CutScreen.nGroup = 0;

					nScreenId = atoi(CutGroup[1].c_str()) & 0xff;
					/* Group<<8|INDEX ,此为唯一性*/
					// 2017/5/5 修改索引 适配多个屏幕自由切屏
					//	int iFreeCutID = iGroup<<8|iIndex;
					int iFreeCutID = iGroup;
					iFreeCutID=(iFreeCutID<<8)|CutScreen.ScreenID;
					iFreeCutID=(iFreeCutID<<8)|iIndex;

					m_MapFreeCutScreen.insert(std::make_pair(iFreeCutID, CutScreen));
					m_nMaxIndex = m_nMaxIndex > iIndex? m_nMaxIndex : iIndex ;
				}
				//////m_GroupScreenMap//////////////////////
				//sFreeCutInfo.CutNum = monitorID;
				sFreeCutInfo.CutNum = m_nMaxIndex;
				sFreeCutInfo.nScreenID = nScreenId;

				m_GroupScreenMap.insert(std::make_pair(iSwitchGroup, sFreeCutInfo));//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
				//m_nMaxIndex > monitorID ? m_nMaxIndex : m_nMaxIndex = monitorID;

			}//if (sFileName != "General.ini")
		}//while(bBool)
	}//if (filefind.FindFile(sFilePath, 0))
	CClientManager::GetInstance()->groupDataLock.release();
}

void CClientManager::SetIpcState(const char* pIpcStatelist)
{
	m_lockClient.acquire();
	if(ipcStaFromAs300)
	{
		for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			iter->second->WriteIpcStateFromAs300(pIpcStatelist);
		}
	}
	else
	{
		for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			iter->second->WriteIpcState(pIpcStatelist);
		}
	}
	m_lockClient.release();
}

void CClientManager::SetGroupsWitch(int iGroupNum, int iScreenNum, int iModeNum, int iScreenNUm)
{
	ScreenSwitch2TVWALL(iGroupNum, iScreenNum, iModeNum, iScreenNUm);
}

//根据不同的分组切换标志，将切屏的方案发送给不同的 nGroup,screen,nMode
void CClientManager::ScreenSwitch2TVWALL(int nGroup,int nScreenId, int nMode ,int iSwitchGroup)
{
	//是否改变切换，如果从１组显示２组的视频
	int iGroupScreen = (nGroup<<8)|nScreenId;
	/* 如果是要整体切换，需要换方案*/
	int iPCSreenMode =( iSwitchGroup<<16)|(nScreenId<<8)|nMode;

	Group2PcMap::iterator iteGroup2PC = m_Group2PcMap.find(iGroupScreen);
	if (iteGroup2PC == m_Group2PcMap.end())
	{   
		return;
	}

	/* 从ｇｒｏｕｐ，screen号找ｐｃ的ｔｖｓ的ｉｐ，ｐｏｒｔ和屏幕号*/
	std::string strIP = iteGroup2PC->second.Ip;
	int iPort = iteGroup2PC->second.Port;
	int iTVScreenID = iteGroup2PC->second.iTVSSCREENID;


	Screen2GroupPCMap::iterator itPCSreenMode = m_GroupPCScreen.find(iPCSreenMode);
	if (itPCSreenMode == m_GroupPCScreen.end())
	{
		return;
	}

	/* 从ｇｒｏｕｐ，screen和ｍｏｄｅ号，找对应的操作列表,先分割切换*/
	std::string sRet;
	sRet.append("{");
	SetValue(sRet, "key_id", m_KeyId);
	SetValue(sRet, "command", "SCREEN_CUT", FALSE);
	SetValue(sRet, "reset", 1, FALSE);

	int iWindowNum = itPCSreenMode->second.splitNum;
	int iWindowIndex = itPCSreenMode->second.windowsIndex;	//画面分割的子类

	std::map<int, FreeCutInfo>::iterator iteGroupScreen = m_GroupScreenMap.find(iSwitchGroup);//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
	if (iteGroupScreen != m_GroupScreenMap.end())
	{
		if (nScreenId == iteGroupScreen->second.nScreenID)
		{
			iteGroupScreen->second.CutNum = iWindowNum;	

		}
	}

	SetValue(sRet, "screen_id", iTVScreenID);
	SetValue(sRet, "number", iWindowNum);
	SetValue(sRet, "mode", iWindowIndex);
	SetjsonEnd(sRet);
	SendInfo2TVWALLServer(sRet, strIP, iPort);

	/*组，ｐｃ上的ｔｖｓ，再进行画面切换；*/
	std::string sRetVec;
	sRetVec.append("{");
	SetValue(sRetVec, "key_id", m_KeyId);
	SetValue(sRetVec, "command", "MAP_IPC", FALSE);
	textBarModeLoad(sRetVec,iPCSreenMode);
	sRetVec.append("\"maps\":[");			

	for (std::vector<WINDOWS_IPCID>::iterator ite_win = itPCSreenMode->second.winId.begin(); 
		ite_win != itPCSreenMode->second.winId.end(); ite_win++)
	{
		sRetVec.append("{");
		SetValue(sRetVec, "ipc_id", ite_win->DeviceId.c_str(), FALSE);
		SetValue(sRetVec, "monitor_id", ite_win->winNum + 64*(iTVScreenID-1), FALSE);//edit by jeckean,64 nocheck
		//SetValue(sRetVec, "stream_type", ite_win->stream_type, TRUE);	//主副码流
		//=============textBar 2017/4/7===================================
		TextIpcRetVecGet(sRetVec,nGroup,ite_win->DeviceId,ite_win->stream_type,ite_win->winNum + 64*(iTVScreenID-1));
		//===============================================================
		sRetVec.append("},");
	}
	sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
	sRetVec.append("]}");
	SendInfo2TVWALLServer(sRetVec, strIP, iPort);

	//保存最后一次的分割和上墙状态
	stringstream ss;
	string str;
	ss<<iPort;
	ss>>str;
	std::string sIpPort = strIP + + "," + str;


	IpPort2CutMapInfo::iterator ite = m_TvWallIp2CutMapInfo.find(sIpPort);
	if (ite != m_TvWallIp2CutMapInfo.end())
	{
		ScreenId2Cut::iterator it = ite->second.ScreenId2CutMap.find(iTVScreenID);
		if (it != ite->second.ScreenId2CutMap.end())//存在，修改值
		{
			it->second.sCutInfo = sRet;
			it->second.sIpcMap = sRetVec;
		}
		else//不存在，添加上
		{
			sCutIpc Tmp;
			Tmp.sCutInfo = sRet;
			Tmp.sIpcMap = sRetVec;
			ite->second.ScreenId2CutMap.insert(std::make_pair(iTVScreenID,Tmp));
		}
	}
}

void CClientManager::PtzOpration2TVALL(int nGroup, int nIPCIndex, std::string SubCmd,bool isStart, int iSwitchGroup)
{
	int iGroupIpcId = (nGroup<<8)|nIPCIndex;

	GroupIPCIDMap::iterator iteGroupIPCID = m_GroupIpcId.find(iGroupIpcId);
	if (iteGroupIPCID == m_GroupIpcId.end())
	{   
		return;
	} //获取ipc的puid
	std::string strPUID = iteGroupIPCID->second.strPUID;
	std::string strIP = "";
	int iPort = 0;
	int istep = iteGroupIPCID->second.istep;

	std::string sRet;
	sRet.append("{");
	SetValue(sRet, "key_id", m_KeyId);
	SetValue(sRet, "command", "PTZ");
	std::string str = isStart ? "start" : "stop";
	SetValue(sRet, "sub_cmd", SubCmd.c_str(), FALSE);
	SetValue(sRet, "state", str.c_str(), false);
	SetValue(sRet, "ipc_id", strPUID.c_str(), FALSE);
	//SetValue(sRet, "monitor_id", strPUID.c_str(), FALSE);
	SetValue(sRet, "stepX", istep, FALSE);
	SetValue(sRet, "stepY", istep, FALSE);
	SetjsonEnd(sRet);
	//获取tv_wall的地址和端口
	for (Group2PcMap::iterator iteGroupPc = m_Group2PcMap.begin(); iteGroupPc != m_Group2PcMap.end(); iteGroupPc++)
	{
		if (iSwitchGroup == (iteGroupPc->first>>8 & 0xff) && (strIP != iteGroupPc->second.Ip || iPort != iteGroupPc->second.Port))
		{
			strIP = iteGroupPc->second.Ip;
			iPort = iteGroupPc->second.Port;
			if(SendInfo2TVWALLServer(sRet, strIP, iPort))
			{
				//=2017/4/26 一组中两台TVWALll,发送给两个操作台
				continue;

				//break;
			}
			continue;
		}
	}
}
//==============2017/5/27 ptz send to cam=============
//void CClientManager::PtzOpration2CAM(int nGroup, int nIPCIndex, std::string SubCmd,int cmdIndex, int iSwitchGroup)
bool CClientManager::PtzOpration2CAM(int nGroup, int nIPCIndex, bool isStart,int cmdIndex, int iSwitchGroup)
{
	int iGroupIpcId = (nGroup<<8)|nIPCIndex;

	GroupIPCIDMap::iterator iteGroupIPCID = m_GroupIpcId.find(iGroupIpcId);
	if (iteGroupIPCID == m_GroupIpcId.end())
	{   
		return false ;
	} //获取ipc的puid
	std::string strIP = iteGroupIPCID->second.Ip;
	std::string commandStr;
	std::string step ="4";
	//"WIPER"/*, "ZOOM"*/,"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE" continuouspantiltmove
	//char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"rtilt","rtilt","rpan","rpan", "rzoom", "rzoom"};
	//char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"up","down","left","right", "continuouszoommove", "continuouszoommove"};
	//char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"continuouspantiltmove","continuouspantiltmove","continuouspantiltmove","continuouspantiltmove", "continuouszoommove", "continuouszoommove"};
	PTZ_Ctrl cmdBuffer[12]={PTZ_Up,PTZ_Up,PTZ_Down,PTZ_Left,PTZ_Right,PTZ_ZoomAdd,PTZ_ZoomDec,PTZ_LeftUp,PTZ_LeftDown,PTZ_RightUp,PTZ_RightDown};
	if(cmdIndex>=0&&cmdIndex<=12)
	{
		LoginDevInfo ipcDevice;
		ipcDevice.userName="root";
		ipcDevice.password="pass";
		ipcDevice.nDVRPort = 80;
		ipcDevice.pchDVRIP = strIP;
		Vix_PtzCfgParam ptzParam;
		ptzParam.bStop = !isStart;
		ptzParam.lParam1=2;
		ptzParam.lParam2=2;
		return PtzCmdCtrl(ipcDevice,cmdBuffer[cmdIndex],ptzParam);
		//commandStr = strPtzBuffer[cmdIndex];
		//if(cmdIndex<=4)
		//{
		//	if(SubCmd=="STOP")
		//		commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?"+commandStr+"=0,0";
		//	else
		//	{
		//		if(cmdIndex<=2)
		//		{
		//			if(cmdIndex==2)
		//				step="-"+step;	
		//			commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?"+commandStr+"=0,"+step;
		//		}
		//		else
		//		{
		//			if(cmdIndex==3)
		//				step="-"+step;	
		//			commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?"+commandStr+"="+step+",0";
		//		}
		//	}
		//}
		//else if(cmdIndex>4&&cmdIndex<=6)
		//{
		//	if(SubCmd=="STOP")
		//		commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?"+commandStr+"=0";
		//	else
		//	{
		//		if(cmdIndex%2==0)
		//			step="-"+step;	
		//		commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?"+commandStr+"="+step;
		//	}
		//}
		//  sendCgiCommad(commandStr,nGroup); 
	}
	return false;
}
bool CClientManager::PtzCmdCtrl(LoginDevInfo logDevice, PTZ_Ctrl lConfigType, Vix_PtzCfgParam nParam)
{
	bool lRet = false;

	LoginDevInfo* pDevInfo = &logDevice;
	if (pDevInfo == NULL)
	{
		return false;
	}

	PTZ_Ctrl  ptzCmd = (PTZ_Ctrl)lConfigType;
	switch (ptzCmd)
	{
	case PTZ_PointLoop:
		break;
	case PTZ_Up:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=0,%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_Down:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=0,-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_Left:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=-%d,0",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_Right:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?camera=1&continuouspantiltmove=%d,0",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_LeftUp:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=-%d,%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_RightUp:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=%d,%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_LeftDown:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=-%d,-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_RightDown:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam1*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouspantiltmove=%d,-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_ZoomAdd:   //变倍
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouszoommove=%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_ZoomDec:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuouszoommove=-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_FocusAdd:  //聚焦
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuousfocusmove=%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_FocusDel:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuousfocusmove=-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_ApeAdd:    //光圈
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuousirismove=%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_ApeDel:
		{
			int nSpeed = 0;
			if(!nParam.bStop)
			{
				nSpeed = nParam.lParam2*100/8;
			}

			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?continuousirismove=-%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nSpeed,nSpeed);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_PointMove:
		{
			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?gotoserverpresetname=%s",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nParam.presetpointName.c_str());
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	case PTZ_StartPanCruise:
		break;
	case PTZ_StopPanCruise:
		break;
	case PTZ_StartLineScan:
		break;
	case PTZ_CloseLineScan:
		break;
	case PTZ_SetLeftBorder:
		break;
	case PTZ_SetRightBorder:
		break;
	case PTZ_SetModesStart:
		break;
	case PTZ_SetModesStop:
		break;
	case PTZ_RunMode:
		break;
	case PTZ_StopMode:
		break;
	case PTZ_AuxiOpen:
		{
			switch (nParam.lParam1)
			{
			case 100:
				{
					std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?zoom=%d",
						pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nParam.lParam2);
					lRet = CCuHttpConfigClient::StartCGI(url);
				}
				break;
			case 101:
				{
					if (nParam.lParam2)
					{
						std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?autofocus=on",
							pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort);
						lRet = CCuHttpConfigClient::StartCGI(url);
					}
					else
					{
						std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?autofocus=off",
							pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort);
						lRet = CCuHttpConfigClient::StartCGI(url);
					}
				}
			}
		}
		break;
	case PTZ_AuxiClose:
		break;
	case PTZ_ZoomSet:
		{
			std::string url = base::StringPrintf("%s:%s@%s:%d/axis-cgi/com/ptz.cgi?zoom=%d",
				pDevInfo->userName.c_str(),pDevInfo->password.c_str(),pDevInfo->pchDVRIP.c_str(),pDevInfo->nDVRPort,nParam.lParam2);
			lRet = CCuHttpConfigClient::StartCGI(url);
		}
		break;
	default:
		return false;
	}
	return lRet;

}

//=============2016/12/18 锁头跟随===================================//
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
	size_t newLength = size*nmemb;
	size_t oldLength = s->size();
	try
	{
		s->resize(oldLength + newLength);
	}
	catch(std::bad_alloc &e)
	{
		//handle memory problem
		return 0;
	}

	std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
	return size*nmemb;
}
int CClientManager::sendCgiCommad(const std::string command,int nGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl!=NULL)
	{
		std::string strResponse;
		curl_easy_setopt(curl,CURLOPT_URL,command.c_str());
		curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
		curl_easy_setopt(curl,CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
		curl_easy_setopt(curl,CURLOPT_USERNAME,"root");
		curl_easy_setopt(curl,CURLOPT_PASSWORD,"pass");
		curl_easy_setopt(curl,CURLOPT_HEADER,1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResponse);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
		res = curl_easy_perform(curl);
		if(res !=CURLE_OK)
		{
			char szlog[MAX_STR_LEN]={0};
			_snprintf(szlog,MAX_STR_LEN-1,"group_%d, send cgi command failure! commad_%s ，error_%s",
				nGroup,command.c_str(),curl_easy_strerror(res));
			pInstance->m_plog->TraceInfo(szlog);
			pInstance->Showlog2Dlg(szlog);
			curl_easy_cleanup(curl); 
			curl_global_cleanup();
			return 1;
		}
		else
		{
			char szlog[MAX_STR_LEN]={0};
			_snprintf(szlog,MAX_STR_LEN-1,"group_%d, send cgi command success! commad_%s",nGroup,command.c_str());
			pInstance->Showlog2Dlg(szlog);
			pInstance->m_plog->TraceInfo(szlog);
		}
	}
	curl_easy_cleanup(curl); 
	curl_global_cleanup();
	return 0;
}

//2017/4/3 cgi setZoom========
int CClientManager::CamIpGet(std::string sPuid, int nGroup,std::string& camIp)
{
	int iGroupIpcId=0;
	int iIpcNum = 0;
	CClientManager* pInstance = CClientManager::GetInstance(); //找到组内ID (nGroup<<8)|nIPCIndex
	if (pInstance->m_bBolModbus)
		iIpcNum = pInstance->m_ModBusFormatInfo.nPtzNum;
	else
		iIpcNum = pInstance->m_PLCFormatInfo.nPtzNum;

	for (int nIPCIndex = 1; nIPCIndex < iIpcNum; nIPCIndex++)
	{
		iGroupIpcId = (nGroup<<8)|nIPCIndex;
		GroupIPCIDMap::iterator iteGroupIPCID = pInstance->m_GroupIpcId.find(iGroupIpcId);
		if (iteGroupIPCID == pInstance->m_GroupIpcId.end())
		{   
			continue;
		} 
		//获取ipc的puid
		std::string strID = iteGroupIPCID->second.strPUID;
		if (strID == sPuid)
		{
			break;
		}
	}
	GroupIPCIDMap::iterator iteIPCInfoMap = CClientManager::GetInstance()->m_GroupIpcId.find(iGroupIpcId); //找相机IP
	if(iteIPCInfoMap == CClientManager::GetInstance()->m_GroupIpcId.end())
		return 2;
	camIp = iteIPCInfoMap->second.Ip;
	return 0;
}

int CClientManager::sendCgiCommadZoom(std::string sPuid, int nZoomValue, int nGroup)
{
	//get ip
	std::string strIP;
	if(CamIpGet(sPuid,nGroup,strIP))
	{
		return 0;
	}
	//std::string commandStr;
	//ostringstream streamZoomValue;
	//streamZoomValue<<nZoomValue;
	//commandStr = "http://"+strIP+"/axis-cgi/com/ptz.cgi?zoom="+streamZoomValue.str();	
	//return sendCgiCommad(commandStr,nGroup);
	LoginDevInfo ipcDevice;
	ipcDevice.pchDVRIP = strIP;
	ipcDevice.nDVRPort = 80;
	ipcDevice.userName = "root";
	ipcDevice.password = "pass";
	Vix_PtzCfgParam ptzParam;
	ptzParam.lParam2 = nZoomValue;
	return 	PtzCmdCtrl(ipcDevice,PTZ_ZoomSet,ptzParam);

}
//===============================================================================//
void CClientManager::IpcZoom2TVWALL(std::string sPuid, int nZoomValue, int iSwitchGroup)
{
	int iSize = m_Group2PcMap.size();
	if (iSize <= 0)
	{
		return;
	}
	std::string strIP = "";
	int iPort = 0;
	std::string sRet;
	sRet.append("{");
	SetValue(sRet, "key_id", m_KeyId);
	SetValue(sRet, "command", "PTZ");
	SetValue(sRet, "sub_cmd", "SETZOOM", FALSE);
	SetValue(sRet, "ipc_id", sPuid.c_str(), FALSE);
	SetValue(sRet, "value", nZoomValue, FALSE);
	SetjsonEnd(sRet);
	//获取tv_wall的地址和端口
	for (Group2PcMap::iterator iteGroupPc = m_Group2PcMap.begin(); iteGroupPc != m_Group2PcMap.end(); iteGroupPc++)
	{
		if (iSwitchGroup == (iteGroupPc->first>>8 & 0xff) && (strIP != iteGroupPc->second.Ip || iPort != iteGroupPc->second.Port))
		{
			strIP = iteGroupPc->second.Ip;
			iPort = iteGroupPc->second.Port;
			if (SendInfo2TVWALLServer(sRet, strIP, iPort))
			{
				//for two tvWall in one group 2017/4/28
				continue;
				//break;
			}
			continue;
		}
	}	
}

bool CClientManager::SendInfo2TVWALLServer(std::string sRet, std::string ip, int nPort)
{
	//通过回调函数实现,连接不存在时容易卡死
	char szlog[MAX_STR_LEN] = {0};
	_snprintf(szlog, MAX_STR_LEN-1, "Ip:%s，port:%d, info:%s",ip.c_str(), nPort, sRet.c_str());
	CClientManager::GetInstance()->m_plog->TraceTVWALLInfo(szlog);
	bool bRet = m_InfoCallFun(sRet.c_str(),ip.c_str(), nPort, m_dwDataUser);
	return bRet;
}

#define __countof(array) (sizeof(array)/sizeof(array[0]))
#pragma warning (disable:4996)


void CClientManager::SendLog2Dlg(const char *pFormat,...)
{
	va_list args;
	va_start(args, pFormat);
	int nBuff;
	CHAR szBuffer[4096];
	nBuff = _vsnprintf(szBuffer, __countof(szBuffer), pFormat, args);	
	va_end(args);
	m_CallLoginfo(0, szBuffer, m_dwDataUser);

}

void CClientManager::Showlog2Dlg(const char *pStr, int itype)
{
	//=== 2017/3/15 DataViwer
	if(itype==CONNECT_PLC_DATA)
	{
		char data[BLOCK_SIZE_COPY] ={0};
		memcpy(data,pStr,BLOCK_SIZE_COPY);
		m_CallLoginfo(itype,data,m_dwDataUser);
		return ;
	}
	//==========================
	m_CallLoginfo(itype, pStr, m_dwDataUser);
}

uint32 CClientManager::GetSequence(void)
{ 
	return m_nIDGenerator++;
}

long CClientManager::createClient(const char* szIp, int nPort, PLCADDRDataInfo info, int nRack, int nSlot, int nServerKeyID , int nGroup)
{
	uint32 nTmp = GetSequence();
	m_lockClient.acquire();
	m_clients[nTmp] = new CClient(this, info);
	m_clients[nTmp]->SetPlcServerInfo(szIp, nPort, nRack, nSlot, nGroup, nServerKeyID);
	m_lockClient.release();
	m_clients[nTmp]->addRef();
	return nTmp;
}

CClient* CClientManager::getClient(long lID)
{
	m_lockClient.acquire();
	std::map<long, CClient*>::iterator iter = m_clients.find(lID);
	if (iter != m_clients.end())
	{
		iter->second->addRef();
		m_lockClient.release();
		return iter->second;
	}
	m_lockClient.release();
	return NULL;
}

long CClientManager::getClient(const char* szIp, int nPort, int nRack, int nSlot)
{
	m_lockClient.acquire();
	std::map<long, CClient*>::iterator iter = m_clients.begin();
	while (iter != m_clients.end())
	{
		if (iter->second->isSameDevice(szIp,nPort,nRack,nSlot))
		{
			m_lockClient.release();
			return iter->first;
		}
		++iter;
	}
	m_lockClient.release();
	return 0;
}

void CClientManager::remvoeClient(long lID)
{
	m_lockClient.acquire();
	std::map<long, CClient*>::iterator iter = m_clients.find(lID);
	if (iter != m_clients.end())
	{
		iter->second->release();
		m_clients.erase(iter);
	}
	m_lockClient.release();
}

void CClientManager::GetPlcDataInfo()
{
	m_lockClient.acquire();
	for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
	{
		iter->second->ReadPLCDataProcess();
	}
	m_lockClient.release();
}
void CClientManager::GetModbusDataInfo()
{
	m_lockClient.acquire();
	for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
	{
		iter->second->ReadModbusDataProcess();
	}
	m_lockClient.release();
}

void CClientManager::StartReConnect()
{
	PLCADDRDataInfo info;
	CClientManager::GetInstance()->createClient(m_PLCHttpIp.c_str(), m_nPLCHttpPort, info, 0, 2);
	m_bReConnect = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadReConnect, this, 0, 0, &m_ReConnectThread, 0, 0, 0);
}
//===2017/6/7 modbus server
void CClientManager::StartModbusServer()
{
	//根据配置 映射出所要读的地址
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");
	int offset[3]={0};
	iniFile.ReadInt("MODBUSSERVER", "addressSet1", offset[0]);
	iniFile.ReadInt("MODBUSSERVER", "addressSet2", offset[1]);
	iniFile.ReadInt("MODBUSSERVER", "addressSet3", offset[2]);
	mbMapping = modbus_mapping_new(0,offset[0],offset[2],offset[1]);
	if (mbMapping == NULL)
	{
		char errorLog[200];
		sprintf_s(errorLog,sizeof(errorLog)-1,"Failed to allocate the mapping: %s\n",modbus_strerror(errno));
		SendLog2Dlg(errorLog);
		m_plog->TraceError(errorLog);
		return;
	}
	mModbusServerListen = true;
	AX_Thread::spawn(CClientManager::WorkThreadModbusServer, this, 0, 0, &m_ModbusServerThread, 0, 0, 0);
}

void *CClientManager::WorkThreadModbusServer(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	mModbusServerSocket = -1;
	pThis->RunModbusServer();
	return 0;
}
void CClientManager::RunModbusServer()
{
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int master_socket;
	int rc;
	fd_set refset;
	fd_set rdset;
	/* Maximum file descriptor number */
	int fdmax;
	mModbusServer = modbus_new_tcp("127.0.0.1", 502);
	//mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
	//	MODBUS_MAX_READ_REGISTERS, 0);
	//if (mb_mapping == NULL) {
	//	fprintf(stderr, "Failed to allocate the mapping: %s\n",
	//		modbus_strerror(errno));
	//	modbus_free(ctx);
	//	return -1;
	//}

	mModbusServerSocket = modbus_tcp_listen(mModbusServer, MODBUS_NB_CONNECTION);
	if (mModbusServerSocket == -1) {
		fprintf(stderr, "Unable to listen TCP connection\n");
		modbus_free(mModbusServer);
		return ;
	}

	signal(SIGINT, ModbusCloseSigint);
	/* Clear the reference set of socket */
	FD_ZERO(&refset);
	/* Add the server socket */
	FD_SET(mModbusServerSocket, &refset);
	/* Keep track of the max file descriptor */
	fdmax = mModbusServerSocket;
	while(mModbusServerListen)
	{
		rdset = refset;
		if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1)
		{
			perror("Server select() failure.");
			ModbusCloseSigint(1);
		}

		/* Run through the existing connections looking for data to be
		* read */
		for (master_socket = 0; master_socket <= fdmax; master_socket++) {

			if (!FD_ISSET(master_socket, &rdset))
			{
				continue;
			}

			if (master_socket == mModbusServerSocket)
			{
				/* A client is asking a new connection */
				socklen_t addrlen;
				struct sockaddr_in clientaddr;
				int newfd;

				/* Handle new connections */
				addrlen = sizeof(clientaddr);
				memset(&clientaddr, 0, sizeof(clientaddr));
				newfd = accept(mModbusServerSocket, (struct sockaddr *)&clientaddr, &addrlen);
				if (newfd == -1)
				{
					perror("Server accept() error");
				}
				else {
					FD_SET(newfd, &refset);

					if (newfd > fdmax)
					{
						/* Keep track of the maximum */
						fdmax = newfd;
					}
					char connectInfo[200];
					sprintf_s(connectInfo,sizeof(connectInfo)-1,"New modbus client connection from %s:%d on socket %d\n",
						inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
					SendLog2Dlg(connectInfo);
					m_plog->TraceInfo(connectInfo);
				}
			}
			else 
			{
				modbus_set_socket(mModbusServer, master_socket);
				rc = modbus_receive(mModbusServer, query);
				if (rc > 0) 
				{
					modbus_reply(mModbusServer, query, rc, mbMapping);
				}
				else if (rc == -1) 
				{
					/* This example server in ended on connection closing or
					* any errors. */
					//printf("Connection closed on socket %d\n", master_socket);
					char closeInfo[200];
					sprintf_s(closeInfo,sizeof(closeInfo)-1,"Connection closed on socket %d\n", master_socket);
					SendLog2Dlg(closeInfo);
					m_plog->TraceInfo(closeInfo);
					//close(master_socket);
					/* Remove from reference set */
					FD_CLR(master_socket, &refset);
					if (master_socket == fdmax) 
					{
						fdmax--;
					}
				}
			}
		}
	}
}
void CClientManager::ModbusCloseSigint(int dummy)
{
	if (mModbusServerSocket != -1) {
		//close(server_socket);
	}
	modbus_free(mModbusServer);
	modbus_mapping_free(mbMapping);
	exit(dummy);
}
void CClientManager::stopModbusServer()
{
	if(m_ModbusServerThread)
	{
		mModbusServerListen = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_GetModbusDataThread,1000))
		{
			TerminateThread(m_ModbusServerThread,-1);
			m_ModbusServerThread = NULL;
			if(mModbusServer)
				modbus_free(mModbusServer);
			if(mbMapping)
				modbus_mapping_free(mbMapping);
		}
	}

}


void CClientManager::StartReConnectModbus()
{
	PLCADDRDataInfo info;
	for (std::map<int, ModBusIp>::iterator ite = m_GroupModBusMap.begin(); ite != m_GroupModBusMap.end(); ite++)
	{
		string strIP = ite->second.strIP;
		int nPort = ite->second.nPort;
		int nserverKeyID = ite->second.ServerKeyId;
		int nGroup = ite->first;
		CClientManager::GetInstance()->createClient(strIP.c_str(), nPort, info, 0, 2, nserverKeyID, nGroup);
	}
	m_bReConnectModbus = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadReConnectModbus, this, 0, 0, &m_ReConnectThreadModbus, 0, 0, 0);
}


void CClientManager::StartGetPlcData()
{
	m_bGetPlcData = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadReadPlcData, this, 0, 0, &m_GetPlcDataThread, 0, 0, 0);
}
void CClientManager::StartGetModbusData()
{
	m_bGetModbusData = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadReadModbusData, this, 0, 0, &m_GetModbusDataThread, 0, 0, 0);
}
void CClientManager::StopGetPlcDataModbus()
{
	if(m_GetModbusDataThread)
	{
		m_bGetModbusData = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_GetModbusDataThread,1000))
		{
			TerminateThread(m_GetModbusDataThread,-1);
			m_GetModbusDataThread = NULL;
		}
	}
}
void CClientManager::StopGetPlcData()
{
	if(m_GetPlcDataThread)
	{
		m_bReConnect = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_GetPlcDataThread,1000))
		{
			TerminateThread(m_GetPlcDataThread,-1);
			m_GetPlcDataThread = NULL;
		}
	}
}

void CClientManager::StopReConnect()
{
	if(m_ReConnectThread)
	{
		m_bReConnect = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_ReConnectThread,1000))
		{
			TerminateThread(m_ReConnectThread,-1);
			m_ReConnectThread = NULL;
		}
	}
}

void CClientManager::StopReConnectModbus()
{
	if(m_ReConnectThreadModbus)
	{
		m_bReConnectModbus = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_ReConnectThreadModbus,1000))
		{
			TerminateThread(m_ReConnectThreadModbus,-1);
			m_ReConnectThreadModbus = NULL;
		}
	}
}

void CClientManager::StartKeepAlive2TvWall()
{
	m_bKeepAlive = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadKeepAlive, this, 0, 0, &m_KeepAliveThread, 0, 0, 0);
}
void CClientManager::StartKeepAlive2Modbus()
{
	m_bKeepAliveModbus = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadKeepAliveModbus, this, 0, 0, &m_KeepAliveThreadModbus, 0, 0, 0);
}

void CClientManager::stopKeepAlive2TvWall()
{
	if(m_KeepAliveThread)
	{
		m_bKeepAlive = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_KeepAliveThread,1000))
		{
			TerminateThread(m_KeepAliveThread,-1);
			m_KeepAliveThread = NULL;
		}
	}
}
void CClientManager::stopKeepAlive2Modbus()
{
	if(m_KeepAliveThreadModbus)
	{
		m_bKeepAliveModbus = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_KeepAliveThreadModbus,1000))
		{
			TerminateThread(m_KeepAliveThreadModbus,-1);
			m_KeepAliveThreadModbus = NULL;
		}
	}
}

void *CClientManager::WorkThreadReConnect(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunReConnect();
	return 0;
}
void *CClientManager::WorkThreadReConnectModbus(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunReConnectModbus();
	return 0;
}

void *CClientManager::WorkThreadReadPlcData(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunGetPlcData();
	return 0;
}
void *CClientManager::WorkThreadReadModbusData(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunGetModbusData();
	return 0;
}

void *CClientManager::WorkThreadKeepAlive(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunKeepAlive();
	return 0;
}
void *CClientManager::WorkThreadKeepAliveModbus(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunKeepAliveModbus();
	return 0;
}
void CClientManager::MySleep(const long sec)
{
	const long StartTime = GetTickCount();

	while ((GetTickCount()-StartTime) < sec)
	{
	}

}


void CClientManager::RunKeepAliveModbus()
{
	while(m_bKeepAliveModbus)
	{
		m_lockClient.acquire();
		for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			iter->second->HeartBeetModbus();
		}
		m_lockClient.release();
		Sleep(1000);
	}
}
void CClientManager::RunKeepAlive()
{
	while(m_bKeepAlive)
	{
		//m_lockClient.acquire();
		DealKeepAlive();
		//m_lockClient.release();
		Sleep(5000);
		//MySleep(5000);
	}
}
//处理tv_wall的状态，当tv_wall重启成功时，发送最新的对应切屏和视频信息
void CClientManager::DealKeepAlive()
{ 
	int iIndex = 1;
	CClientManager* pInstance = CClientManager::GetInstance();
	pInstance->groupDataLock.acquire();
	for (IpPort2CutMapInfo::iterator ite = m_TvWallIp2CutMapInfo.begin(); ite != m_TvWallIp2CutMapInfo.end(); ite++)
	{
		std::vector<std::string> IpPortVec;
		StringSplit(ite->first, ",", IpPortVec);
		std::string str = ite->first;
		Json::Value RevRoot;   
		RevRoot["key_id"] = 12;
		RevRoot["command"] = "HeartBeat";
		RevRoot["seq"] = ++m_lHeartBeetIndex;
		Json::FastWriter Write;
		std::string strGetList = Write.write(RevRoot);

		CCuHttpConfigClient* HttpKeepAlive = new CCuHttpConfigClient;
		HttpKeepAlive->SetIP(IpPortVec[0].c_str(), atoi(IpPortVec[1].c_str()));
		HttpKeepAlive->SetQueryStr("/api/v1/netkbd");
		HttpKeepAlive->SetChn(0, 0);
		HttpKeepAlive->SetJsonBody(strGetList.c_str());

		std::string json_reponse;
		if(HttpKeepAlive->Start(json_reponse))
		{
			if (0 != json_reponse.length() && -1 != json_reponse.find("\"HeartBeat\""))
			{
				bool bNormal = false;
				Json::Value RevRoot;
				Json::Reader reader;
				if(reader.parse(json_reponse, RevRoot))
				{
					////////////////////////////////////////////////////////////
					//临时添加 解决TVWALL包序不对的现象
					int seq = RevRoot["seq"].asInt();
					if (seq == m_lHeartBeetIndex)
					{
						bNormal = true;
						int State = RevRoot["state"].asInt();

						if (0 != ite->second.iTvState && State != ite->second.iTvState)
						{
							char szlog[MAX_STR_LEN] = {0};

							_snprintf(szlog, MAX_STR_LEN-1, "KeepAlive connect to TVWALL Success ,tvwall_ip-%s, port-%d", IpPortVec[0].c_str(), atoi(IpPortVec[1].c_str()));
							pInstance->m_plog->TraceInfo(szlog);
							pInstance->Showlog2Dlg(szlog);

							SendOldScreeninfo(ite->first, ite->second.ScreenId2CutMap);
							SendOldFreeCutScreeninfo(ite->first);

							ite->second.iTvState = State;
						}

						if (0 == ite->second.iTvState)		//第一次
						{
							ite->second.iTvState = State;
						}
					}
				}

				if (bNormal == false)
				{
					CCuHttpConfigClient* HttpKeep = new CCuHttpConfigClient;
					HttpKeep->SetIP(IpPortVec[0].c_str(), atoi(IpPortVec[1].c_str()));
					HttpKeep->SetQueryStr("/api/v1/netkbd");
					HttpKeep->SetChn(0, 0);
					HttpKeep->SetJsonBody("{\"okokok\"}");


					HttpKeep->Start(json_reponse);

					delete HttpKeep;
				}
			}
			else if (0 != json_reponse.length() && -1 == json_reponse.find("\"HeartBeat\""))
			{
				CCuHttpConfigClient* HttpKeep = new CCuHttpConfigClient;
				HttpKeep->SetIP(IpPortVec[0].c_str(), atoi(IpPortVec[1].c_str()));
				HttpKeep->SetQueryStr("/api/v1/netkbd");
				HttpKeep->SetChn(0, 0);
				HttpKeep->SetJsonBody("{\"okokok\"}");


				HttpKeep->Start(json_reponse);

				delete HttpKeep;
			}
		}//if(HttpKeepAlive->Start(json_reponse))
		else
		{
			if (GetTickCount() - m_lHeartBeetTime[iIndex] >= 30000)
			{
				m_lHeartBeetTime[iIndex] = GetTickCount();

				char szlog[MAX_STR_LEN] = {0};
				_snprintf(szlog, MAX_STR_LEN-1, "wait HeartBeat timeout, tvwall_ip-%s, port-%d", IpPortVec[0].c_str(), atoi(IpPortVec[1].c_str()));
				pInstance->m_plog->TraceInfo(szlog);
				pInstance->Showlog2Dlg(szlog);
			}
		}
		iIndex++;
		delete HttpKeepAlive;
	}
	pInstance->groupDataLock.release();
}

////当TVWALL断线重连时发送最新的自由切屏信息
void CClientManager::SendOldFreeCutScreeninfo(std::string strIpPort)
{
	std::string strIP = strIpPort.substr(0, strIpPort.find(','));
	std::string sPort = strIpPort.substr(strIpPort.find(",")+1);
	int iPort = atoi(sPort.c_str());
	CClientManager* pInstance = CClientManager::GetInstance();
	for (CutScreenMap::iterator ite = m_MapFreeCutScreen.begin(); ite != m_MapFreeCutScreen.end(); ite++)
	{
		if (ite->second.strIP == strIP && ite->second.iPort == iPort)
		{
			std::map<int, FreeCutInfo>::iterator iteGroupScreen = m_GroupScreenMap.find(ite->second.nGroup);
			if (iteGroupScreen == pInstance->m_GroupScreenMap.end())
			{
				return;
			}
			if (ite->second.monitor_ID <= iteGroupScreen->second.CutNum)	//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
			{
				std::string sRetVec;
				sRetVec.append("{");
				SetValue(sRetVec, "key_id", pInstance->m_KeyId);
				SetValue(sRetVec, "command", "MAP_IPC", FALSE);
				//textBarModeLoad(sRetVec,iPCSreenMode);
				sRetVec.append("\"maps\":[");

				sRetVec.append("{");
				SetValue(sRetVec, "ipc_id", ite->second.sIPCPuid.c_str(), FALSE);
				SetValue(sRetVec, "monitor_id", ite->second.monitor_ID + 64*(ite->second.iTVScreenID-1), FALSE);
				//SetValue(sRetVec, "stream_type", ite->second.stream_type, TRUE);
				//=============textBar 2017/4/7===================================
				TextIpcRetVecGet(sRetVec,ite->second.nGroup,ite->second.sIPCPuid,ite->second.stream_type, ite->second.monitor_ID + 64*(ite->second.iTVScreenID-1));
				//===============================================================
				sRetVec.append("},");

				sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
				sRetVec.append("]}");
				pInstance->SendInfo2TVWALLServer(sRetVec, strIP, iPort);
			}
		}
	}
}

void CClientManager::SendOldScreeninfo(std::string strIpPort, ScreenId2Cut vec)
{
	std::vector<std::string> IpPortVec;
	StringSplit(strIpPort, ",", IpPortVec);
	std::string sIp = IpPortVec[0];
	int iPort = atoi(IpPortVec[1].c_str());

	for (ScreenId2Cut::iterator ite = vec.begin(); ite != vec.end(); ite++)
	{
		SendInfo2TVWALLServer(ite->second.sCutInfo, sIp, iPort);
		Sleep(100);
		SendInfo2TVWALLServer(ite->second.sIpcMap, sIp, iPort);
	}
}

void CClientManager::RunGetPlcData()
{
	m_nPLCHeartBeat = GetSysTimeMicros()/1000;
	while(m_bGetPlcData)
	{
		GetPlcDataInfo();
		Sleep(m_nRefresh);
	}
}
void CClientManager::RunGetModbusData()
{
	while(m_bGetModbusData)
	{
		GetModbusDataInfo();
		Sleep(m_nRefresh);
	}
}

void CClientManager::RunReConnect()
{
	while(m_bReConnect)
	{
		m_lockClient.acquire();
		for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			iter->second->Login();
		}
		m_lockClient.release();
		Sleep(1000);
	}
}
void CClientManager::RunReConnectModbus()
{
	while(m_bReConnectModbus)
	{
		m_lockClient.acquire();
		for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			iter->second->LoginModbus();
		}
		m_lockClient.release();
		Sleep(1000);
	}
}


void CClientManager::StringSplit(const std::string& src, const std::string& separator, std::vector<std::string>& dest)
{
	dest.clear();
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator,start);
		if (index != std::string::npos)
		{    
			substring = str.substr(start,index-start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator,index);
			if (start == std::string::npos) return;
		}
	}while(index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}

void CClientManager::groupConfigureRefreshCall()
{
	groupDataRefresh = true;
}

bool CClientManager::TextIpcFind(int nGroup,std::string puid)
{
	IpcText::iterator iterText = mIpcText.find(nGroup);
	if(iterText!=mIpcText.end())
	{
		for(int iIpcN=0;iIpcN<iterText->second.size();iIpcN++)
		{
			if(iterText->second[iIpcN]==puid)
			{
				return true;
			}
		}
	}
	return false;
}

bool CClientManager::TextIpcRetVecGet(std::string& RetVec,int nGroup,std::string puid,int stream_type,int monitorID)
{
	if(TextIpcFind(nGroup,puid))
	{	
		SetValue(RetVec, "stream_type",stream_type, FALSE);	//主副码流
		//====2017/4/17  find textBar mode======
		int mTextMode = 0; 
		textBarInfo::iterator iter = mTextBarInfo.find(puid); 	
		if(iter!=mTextBarInfo.end())
			mTextMode = iter->second;
		SetValue(RetVec,"TextBar",mTextMode,TRUE);
		textIpcMonitorIdRefresh(puid,monitorID);
	}
	else
	{
		SetValue(RetVec, "stream_type", stream_type, TRUE);	//主副码流
	}
	textIpcMonitorIdRefresh(puid,monitorID);
	return true;
}

int CClientManager::textIpcMonitorIdRefresh(std::string puid,int monitor_id)
{

	IpcMonitorID::iterator iter = mTextIpcMonitor.find(puid);
	if(iter!=mTextIpcMonitor.end())
	{
		iter->second = monitor_id;
	}
	else
	{
		mTextIpcMonitor.insert(std::make_pair(puid,monitor_id));
	}
	return 0;
}

int CClientManager::textBarSendToTVwall(int nGroup,TextBarMessage mMessage,int iSwitchGroup)
{
	int monitorId[3]={0};
	positionMonitorIDfind(nGroup,monitorId);
	std::string sRetVec;
	sRetVec.append("{");
	SetValue(sRetVec, "key_id", m_KeyId);
	SetValue(sRetVec, "command", "Text", FALSE);
	sRetVec.append("\"Text\":");			
	sRetVec.append("{");
	//====2017/4/18  textBar message motify===
	//==liftMode
	sRetVec.append("\"LiftMode\":");
	sRetVec.append("{");
	SetValue(sRetVec,"ScrInSSMD",mMessage.mLiftMsg.ScrInSSMD,FALSE);
	SetValue(sRetVec,"ScrInDSMD",mMessage.mLiftMsg.ScrInDSMD,FALSE);
	SetValue(sRetVec,"ScrInPSMD",mMessage.mLiftMsg.ScrInPSMD,TRUE);
	sRetVec.append("},");
	//====Height
	sRetVec.append("\"Height\":");
	sRetVec.append("{");
	SetFloatValue(sRetVec,"TargetDistance",mMessage.mHeightMsg.TargetDistance,FALSE);
	SetFloatValue(sRetVec,"Hoist_Position",mMessage.mHeightMsg.HositPosition,FALSE);
	SetFloatValue(sRetVec,"Hoist2_Position",mMessage.mHeightMsg.TrolleyPosition,FALSE);
	SetFloatValue(sRetVec,"PHoist_Position",mMessage.mHeightMsg.PHoistPosition,FALSE);
	SetFloatValue(sRetVec,"PTrolley_Position",mMessage.mHeightMsg.PTrolleyPosition,TRUE);
	sRetVec.append("},");
	//===Hoist
	sRetVec.append("\"Hoist\":");
	sRetVec.append("[");
	for(int i=0;i<3;i++)
	{
		sRetVec.append("{");
		SetValue(sRetVec,"MonitorID",monitorId[i],FALSE);
		SetValue(sRetVec,"SpreaderLanded",mMessage.mSpreaderMsg[i].SpreaderLanded,FALSE);
		SetValue(sRetVec,"SpreaderLocked",mMessage.mSpreaderMsg[i].SpreaderLocked,FALSE);
		SetValue(sRetVec,"SpreaderUnlocked",mMessage.mSpreaderMsg[i].SpreaderUnlocked,FALSE);
		SetValue(sRetVec,"TwinMode",mMessage.mSpreaderMsg[i].TwinMode,FALSE);
		SetValue(sRetVec,"SpeederFeet",mMessage.mSpreaderMsg[i].SpeederFeet,TRUE);
		sRetVec.append("},");
	}
	sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
	sRetVec.append("]");
	sRetVec.append("},");
	sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
	sRetVec.append("}");
	std::string strIP ="";
	int iPort =0;
	for (Group2PcMap::iterator iteGroupPc = m_Group2PcMap.begin(); iteGroupPc != m_Group2PcMap.end(); iteGroupPc++)
	{
		if (iSwitchGroup == (iteGroupPc->first>>8 & 0xff) && (strIP != iteGroupPc->second.Ip || iPort != iteGroupPc->second.Port))
		{
			strIP = iteGroupPc->second.Ip;
			iPort = iteGroupPc->second.Port;
			if (SendInfo2TVWALLServer(sRetVec, strIP, iPort))
			{
				break;
			}
			continue;
		}
	}	
	return 0;
}
int CClientManager::positionMonitorIDfind(int nGroup,int* monitorId)
{
	//IpcText::iterator iter = mIpcText.find(nGroup);
	//IpcPositionMap::iterator iterPosition  = mIpcPositionMap.find(nGroup); 
	//if(/*iter!=mIpcText.end()*/&&iterPosition!=mIpcPositionMap.end())
	//{
	//	//for(int i=0;i<iter->second.size();i++)
	//	{
	//		ipcPosition::iterator iterIpcPosition = iterPosition->second.find(iter->second[i]);
	//		if(iterIpcPosition!=iterPosition->second.end())
	//		{
	//			int monitorID =0;
	//			IpcMonitorID::iterator iterMonitorID = mTextIpcMonitor.find(iter->second[i]);
	//			if(iterMonitorID==mTextIpcMonitor.end())
	//				continue;
	//			else
	//				monitorID = iterMonitorID->second;
	//			if(iterIpcPosition->second=="WS")
	//			{
	//				monitorId[0]=monitorID;
	//			}
	//			else if(iterIpcPosition->second=="LS")
	//			{
	//				monitorId[1]=monitorID;
	//			}
	//			else if(iterIpcPosition->second=="PS")
	//			{
	//				monitorId[1]=monitorID;
	//			}
	//		}
	//	}
	//}




	IpcPositionMap::iterator iterPosition  = mIpcPositionMap.find(nGroup); 
	if(/*iter!=mIpcText.end()&&*/iterPosition!=mIpcPositionMap.end())
	{
		//for(int i=0;i<iter->second.size();i++)
		for(ipcPosition::iterator positionS = iterPosition->second.begin();positionS!=iterPosition->second.end();positionS++)
		{

			//ipcPosition::iterator iterIpcPosition = iterPosition->second.find(iter->second[i]);
			std::string puid = positionS->first;

			//if(iterIpcPosition!=iterPosition->second.end())
			//{
			int monitorID =0;
			IpcMonitorID::iterator iterMonitorID = mTextIpcMonitor.find(puid);
			if(iterMonitorID==mTextIpcMonitor.end())
				continue;
			else
				monitorID = iterMonitorID->second;
			if(positionS->second=="WS")
			{
				monitorId[0]=monitorID;
			}
			else if(positionS->second=="LS")
			{
				monitorId[1]=monitorID;
			}
			else if(positionS->second=="PS")
			{
				monitorId[2]=monitorID;
			}
			//	}
		}
	}

	//textBarMonitorIDMap::iterator ite = mTextBarMonitorID.find(nGroup);
	//if(ite!=mTextBarMonitorID.end())
	//{
	//	monitorId[0] = ite->second.wsMonitorID;
	//	monitorId[1]=ite->second.lsMonitorID;
	//	monitorId[2]=ite->second.psMonitorID;
	//}
	return 0;
}

int CClientManager::textBarModeLoad(std::string& RetVec,int id)
{
	ipcTextF::iterator ite = mIpcTextScreen.find(id);
	if(ite!=mIpcTextScreen.end())
	{	
		SetValue(RetVec, "TextBar",ite->second, FALSE);	//主副码流
	}
	else
	{
		SetValue(RetVec, "TextBar",0, FALSE);	//主副码流
	}
	return 0;
}

void CClientManager::ptzOperationTest(int nGroup,int ipcIndex,int ptzType,int type)
{
	int iGroupIpcId = (nGroup<<8)|ipcIndex;
	CClientManager* pInstance = CClientManager::GetInstance();
	GroupIPCIDMap::iterator iteGroupIPCID = CClientManager::GetInstance()->m_GroupIpcId.find(iGroupIpcId);
	if (iteGroupIPCID == CClientManager::GetInstance()->m_GroupIpcId.end())
	{   
		TCHAR szText[1024] = {0};
		_stprintf(szText,"the camera can not find,group:%d index:%d",nGroup,ipcIndex);
		pInstance->Showlog2Dlg(szText);
		return;
	} //获取ipc的puid
	char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE","LEFT_UP","LEFT_DOWN","RIGHT_UP","RIGHT_DOWN","STOP"};
	std::string strPTZCmd;
	strPTZCmd = strPtzBuffer[ptzType];
	bool bStartPtz = true;
	if(strPTZCmd=="STOP")
	{
		bStartPtz = false;
		ptzType = ptzOperationCmdOld;
		strPTZCmd = strPtzBuffer[ptzType];
	}
	ptzOperationCmdOld = ptzType;
	if(type==1) //send to cam
	{
		bool rect = false;
		TCHAR szText[1024] = {0};
		_stprintf(szText,"ptz operation test: group:%d ipcIndex:%d ptzCmd:%s status:%s.",nGroup,ipcIndex,strPTZCmd.c_str(),bStartPtz?"start":"stop");
		pInstance->Showlog2Dlg(szText);
		rect = pInstance->PtzOpration2CAM(nGroup, ipcIndex,bStartPtz,ptzType, 0);	
	}
	else if(type==0)//send to cctv
	{
		TCHAR szText[1024] = {0};
		_stprintf(szText,"ptz operation test: group:%d ipcIndex:%d ptzCmd:%s status:%s.",nGroup,ipcIndex,strPTZCmd.c_str(),bStartPtz?"start":"stop");
		pInstance->Showlog2Dlg(szText);
		pInstance->PtzOpration2TVALL(nGroup, ipcIndex,strPTZCmd,bStartPtz, 0);	
	}
}

void CClientManager::opcInit()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");
	iniFile.ReadString("OPCGROUOPSET","OPC_SERVER",mOpcServerName);
	//READ ROS SET
	readConfigIntString(iniFile,"OPCGROUPSET","ROS_SET",mOpcRosMap);
	//READ RMG SET
	readConfigIntString(iniFile,"OPCGROUPSET","RMG_SET",mOpcRmgMap);
	//READ MODE SET
	readConfigIntString(iniFile,"OPCSWITCHGROUP","MODE_SET",mOpcModeMap);
	//READ FEET SET
	readConfigIntString(iniFile,"OPCZOOMFEETSET","FEET",mOpcFeetMap);
	//READ HEIGHT SET
	iniFile.ReadString("OPCZOOMHEIGHTSET","HEIGHT",mOpcHeight);
	//READ RCCS FEET SET
	readConfigIntString(iniFile,"OPCZOOMFEETSET","RCCSFEET",mOpcRccsFeetMap);
	//READ RCCS HEIGHT SET
	iniFile.ReadString("OPCZOOMHEIGHTSET","RCCSHEIGHT",mOpcRccsHeight);

	//READ CONNECT ROS
	std::string connectStr;
	iniFile.ReadString("OPCGROUPSET","CONNECT_ROS",connectStr);
	std::vector<std::string> connectStrVec;
	StringSplit(connectStr,",",connectStrVec);
	for(int i=0;i<connectStrVec.size();i++)
	{
		mOpcConnetAlwaysRos.push_back(std::atoi(connectStrVec[i].c_str()));
	}

}

void CClientManager::readConfigIntString(CIniFile iniFile,std::string groupName,std::string valueName,OpcValueItemNameMap& valueMap)
{
	std::string setStr;
	iniFile.ReadString(groupName.c_str(),valueName.c_str(),setStr);
	if(setStr.length())
	{
		std::vector<std::string> setVec;
		StringSplit(setStr,",",setVec);
		for(int i=0;i<setVec.size();i++)
		{
			std::vector<std::string>valueVec;
			StringSplit(setVec[i],":",valueVec);
			int id = std::atoi(valueVec[0].c_str());
			valueMap.insert(std::make_pair(id,valueVec[1]));
		}
	}
}

void CClientManager::StartGetOpcData()
{
	m_bGetOpcData = TRUE;
	AX_Thread::spawn(CClientManager::WorkThreadReadOpcData, this, 0, 0, &m_GetOpcDataThread, 0, 0, 0);
}

void CClientManager::StopGetOpcData()
{
	if(m_GetOpcDataThread)
	{
		m_bGetOpcData = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_GetOpcDataThread,1000))
		{
			TerminateThread(m_GetOpcDataThread,-1);
			m_GetOpcDataThread = NULL;
		}
	}
}
void * CClientManager::WorkThreadReadOpcData(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunGetOpcData();
	return 0;
}

void CClientManager::RunGetOpcData()
{
	//m_nPLCHeartBeat = GetSysTimeMicros()/1000;
	while(m_bGetOpcData)
	{
		GetOpcDataInfo();
		Sleep(m_nRefresh);
	}
}

void CClientManager::GetOpcDataInfo()
{
	m_lockClient.acquire();
	for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
	{
		iter->second->ReadOpcDataProcess();
	}
	m_lockClient.release();

}

void CClientManager::StartReConnectOpc()
{
	m_bReconnectOpc = TRUE;
	mOpcServer = new COPCServer();
	PLCADDRDataInfo info;
	CClientManager::GetInstance()->createClient("127.0.0.1", 0, info, 0, 2,505, 1);
	AX_Thread::spawn(CClientManager::WorkThreadReConnectOpc, this, 0, 0, &m_ReConnectOpcThread, 0, 0, 0);
}

void * CClientManager::WorkThreadReConnectOpc(void *lpParam)
{
	CClientManager* pThis = (CClientManager*)lpParam;
	pThis->RunReConnectOpc();
	return 0;

}
void CClientManager::RunReConnectOpc()
{
	while(m_bReconnectOpc)
	{
		if(!mConnectOpc)
		{
			opcServerConnect();
		}
		Sleep(3000);
	}
}

void CClientManager::StopReConnectOpc()
{
	if(m_ReConnectOpcThread)
	{
		m_bReconnectOpc = FALSE;
		mConnectOpc = FALSE;
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_ReConnectOpcThread,1000))
		{
			TerminateThread(m_ReConnectOpcThread,-1);
			m_ReConnectOpcThread = NULL;
		}
	}
	//释放内存
	{

		//if(mOpcGroup)
		//{
		//	delete mOpcGroup;
		//	mOpcGroup = NULL;
		//}	


	
		if(mOpcServer)
		{
			delete mOpcServer;
			mOpcServer = NULL;
			mOpcGroup = NULL;
		}
		for(OpcItemMap::iterator iterItem = mOpcItemMap.begin();iterItem!=mOpcItemMap.end();iterItem++)
		{
			if(iterItem->second)
			{
				iterItem->second = NULL;
			}
		}
		mOpcItemMap.clear();
	}
}

void CClientManager::opcServerConnect()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	OPCServerInfo info ;
	//info.m_Description = _T("ZPMC OPCServer");
	//info.m_ProgID = _T("ZPMC.OPCServer.2");
	wchar_t* p = UTF8ToUnicode(mOpcServerName.c_str());
	CString serverNameInfo(p);
	info.m_ProgID = serverNameInfo;
	info.m_Description = serverNameInfo;
	free(p);

	CString	m_strNodeName;

	OPCServerInfo*		server_info;
	ServerInfoList		server_infos;
	CLSID			cat_id;
	CLSID			clsid;
	cat_id = CATID_OPCDAServer20;

	unsigned long const NEXT_COUNT = 100;

	IOPCServerList*	server_list = 0;
	COSERVERINFO	si;
	MULTI_QI	qi;

	si.dwReserved1 = 0;
	si.dwReserved2 = 0;
	si.pwszName = L"127.0.0.1";
	si.pAuthInfo = NULL;

	qi.pIID = &IID_IOPCServerList;
	qi.pItf = NULL;
	qi.hr = 0;

	char connectLog[MAX_STR_LEN];
	sprintf_s(connectLog,MAX_STR_LEN-1,"opc server %s connecting......!",mOpcServerName.c_str());
	CClientManager::GetInstance()->Showlog2Dlg(connectLog,CONNECT_PLC_ERR);

	HRESULT hr = CoCreateInstanceEx(
		CLSID_OPCServerList,
		NULL,
		CLSCTX_ALL,
		&si,
		1,
		&qi);
	if(FAILED(hr) || FAILED(qi.hr)){
		CString msg(_T("Error connecting to OPC 2.0 Server Browser."));
		if( !m_strNodeName.IsEmpty() )
			msg.Format(_T("Error connecting to OPC 2.0 Server Browser on %s."), (LPCTSTR)m_strNodeName);

		if( hr == REGDB_E_CLASSNOTREG )
		{
			CString msg(_T("Please install the OPC 2.0 Components."));
			if( !m_strNodeName.IsEmpty() )
				msg.Format(_T("Please install the OPC 2.0 Components on %s."), (LPCTSTR)m_strNodeName);
		}
		//if( FAILED(hr) )
		//	theDoc->ReportError(msg, hr);
		//else
		//	theDoc->ReportError(msg, qi.hr);
	}
	else{
		server_list = (IOPCServerList*)qi.pItf;
		IEnumGUID* enum_guid = NULL;
		hr = server_list->EnumClassesOfCategories(
			1,
			&cat_id,
			1,
			&cat_id,
			&enum_guid);
		if(SUCCEEDED(hr)){
			unsigned long count = 0;
			CLSID cls_id[NEXT_COUNT];

			do{
				hr = enum_guid->Next(NEXT_COUNT, cls_id, &count);
				for(unsigned int index = 0; index < count; index ++){
					LPOLESTR prog_id;
					LPOLESTR user_type;
					HRESULT hr2 = server_list->GetClassDetails(cls_id[index], &prog_id, &user_type);
					if(SUCCEEDED(hr2)){
						OPCServerInfo* info = new OPCServerInfo(prog_id, user_type, cls_id[index]);
						if(info){
							//info->m_NodeName = sz_node;
							server_infos.AddTail(info);
							server_info = info;
						}

						CString name;
						name.Format(_T("%s"),(LPCTSTR)info->m_ProgID);

						CoTaskMemFree(prog_id);
						CoTaskMemFree(user_type);
					}
				}
			}while(hr == S_OK);
			enum_guid->Release();
			server_list->Release();
		}
		else{
			CString msg(_T("EnumClassesOfCategories failed:"));
		}
	}
	mOpcServer->SetServerInfo(server_info);
	if(mOpcServer->connect())
	{
		//mConnectOpc = true;
		char connectLog[MAX_STR_LEN];
		sprintf_s(connectLog,MAX_STR_LEN-1,"opc server %s connect success!",server_info->m_ProgID);
		CClientManager::GetInstance()->m_plog->TraceInfo(connectLog);
		CClientManager::GetInstance()->Showlog2Dlg(connectLog,CONNECT_PLC_SUC);

	}
	else
	{
		mConnectOpc = false;
		char connectLog[MAX_STR_LEN];
		sprintf_s(connectLog,MAX_STR_LEN-1,"opc server %s connect failed!",server_info->m_ProgID);
		CClientManager::GetInstance()->m_plog->TraceInfo(connectLog);
		CClientManager::GetInstance()->Showlog2Dlg(connectLog,CONNECT_PLC_ERR);
		return;
	}
	//add opc item
	opcItemAdd(mOpcFeetMap,1);//feet
	opcItemAdd(mOpcModeMap,0);//mode
	opcItemAdd(mOpcRccsFeetMap,2);//rccs feet
	//height
	for(OpcValueItemNameMap::iterator iterRmg = mOpcRmgMap.begin();iterRmg!=mOpcRmgMap.end();iterRmg++)
	{
		Item* item = new Item;
		ASSERT(item);
		std::string nameStr;
		if(opcGroupConnectAlways(iterRmg->first))
			nameStr = iterRmg->second+"."+mOpcRccsHeight;
		else
			nameStr = iterRmg->second+"."+mOpcHeight;
		
		wchar_t* p = UTF8ToUnicode(nameStr.c_str());
		CString itemName(p);
		item->name = itemName;
		free(p);
		item->access_path = "";
		item->native_type = 0;
		item->quality = OPC_QUALITY_GOOD;
		item->active = true;
		opcSingleItemAdd(item,nameStr);
	}
	//add rosSet
	for(OpcValueItemNameMap::iterator iterRos = mOpcRosMap.begin();iterRos!=mOpcRosMap.end();iterRos++)
	{
		Item* item = new Item;
		ASSERT(item);
		std::string nameStr = iterRos->second;
		wchar_t* p = UTF8ToUnicode(nameStr.c_str());
		CString itemName(p);
		item->name = itemName;
		free(p);
		item->access_path = "";
		item->native_type = 0;
		item->quality = OPC_QUALITY_GOOD;
		item->active = true;
		opcSingleItemAdd(item,nameStr);
	}
		mConnectOpc = true;


}
bool CClientManager::opcGroupConnectAlways(int nGroup)
{
	bool isConnect= false;
	for(int i=0;i<mOpcConnetAlwaysRos.size();i++)
	{
		if(mOpcConnetAlwaysRos[i]==nGroup)
		{
			isConnect = true;
			break;
		}
	}
	return isConnect;
}

wchar_t * CClientManager::UTF8ToUnicode(const char* str)
{
	int textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_ACP, 0, str,-1, NULL,0 ); 
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
	memset(result,0,(textlen+1)*sizeof(wchar_t)); 
	MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen ); 
	return result; 

}
void CClientManager::opcItemAdd(OpcValueItemNameMap ItemNameMap,int mode)
{
	if(mOpcGroup==NULL)
	{
		mOpcGroup = new COPCGroup(mOpcServer);
		if(mOpcGroup){
			mOpcGroup->set_name("group");
			mOpcGroup->set_update_rate(100);
			mOpcGroup->set_dead_band(0);
			mOpcGroup->set_time_bias(0);
			mOpcGroup->set_local_id(0);
			mOpcGroup->set_active(true);

			mOpcGroup->parent = mOpcServer;
			mOpcServer->add_group(mOpcGroup);
		}
	}
	//===add item
	for(OpcValueItemNameMap::iterator iterRmg = mOpcRmgMap.begin();iterRmg!=mOpcRmgMap.end();iterRmg++)
	{
		if(mode==1)
		{
			if(opcGroupConnectAlways(iterRmg->first))
				continue;
		}
		else if(mode==2)
		{
			if(!opcGroupConnectAlways(iterRmg->first))
				continue;
		}
		for(OpcValueItemNameMap::iterator itemIter = ItemNameMap.begin(); itemIter!=ItemNameMap.end();itemIter++)
		{
			Item* item = new Item;
			ASSERT(item);
			std::string nameStr = iterRmg->second+"."+itemIter->second;
			wchar_t* p = UTF8ToUnicode(nameStr.c_str());
			CString itemName(p);
			item->name = itemName;
			free(p);
			item->access_path = "";
			item->native_type = 0;
			item->quality = OPC_QUALITY_GOOD;
			item->active = true;
			opcSingleItemAdd(item,nameStr);
		}
	}
}

void CClientManager::opcSingleItemAdd(Item* item,std::string name)
{
	if(mOpcServer != NULL)
	{
		COPCGroup* group = mOpcServer->get_current_group();
		if(group != NULL)
		{
			if(group->add_item(item))
			{
				mOpcItemMap.insert(std::make_pair(name,item));
				char connectLog[MAX_STR_LEN];
				sprintf_s(connectLog,MAX_STR_LEN-1,"add item %s success!",name.c_str());
				CClientManager::GetInstance()->m_plog->TraceInfo(connectLog);
				CClientManager::GetInstance()->SendLog2Dlg(connectLog);

			}
			else
			{
				char connectLog[MAX_STR_LEN];
				sprintf_s(connectLog,MAX_STR_LEN-1,"add item %s failured!",name.c_str());
				CClientManager::GetInstance()->m_plog->TraceInfo(connectLog);
				CClientManager::GetInstance()->SendLog2Dlg(connectLog);
			}
				//if(item)
			//	theDoc->UpdateAllViews(NULL, UPDATE_GROUP, (CObject*)group);
		}
	}
}


CClient::CClient(CClientManager *pClientMgr, PLCADDRDataInfo info)
{

	addRef();
	memset(&_fds, 0, sizeof(_fds));
	_di = NULL;
	_dc = NULL;
	m_nRack = 0;
	m_nSlot = 2;
	m_PLCFormatInfo = info;
	m_bConnect = false;
	InitOldState();
	m_nNumIndex = 0;
}

CClient::~CClient(void)
{
	DeleteOldState();
}

void CClient::SetPlcServerInfo(std::string sIp, int nPort, int nRack, int nSlot, int nCurrentGroup, int ServerKeyId)
{
	m_strIp = sIp;
	m_nPort = nPort;
	m_nRack = nRack;
	m_nSlot = nSlot;
	m_nGroup = nCurrentGroup;
	m_iServerKeyID = ServerKeyId;
	m_nCurrentTime = 0;
}

void CClient::InitOldState()
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (!pInstance->m_bBolModbus)
	{
		int nGroup = pInstance->m_GroupDBMap.size();
		for (std::map<int, int>::iterator ite = pInstance->m_GroupDBMap.begin();
			ite !=  pInstance->m_GroupDBMap.end(); ite++)
		{
			unsigned short *num = new  unsigned short[32*32];
			memset(num, 0, 32*32);
			m_MapScreenState.insert(std::make_pair(ite->first, num));

			unsigned char *buf = new unsigned char[256];
			memset(buf, 0, 256);
			m_MapRecvData.insert(std::make_pair(ite->first, buf));
		}

	}
	else
	{
		for (std::map<int, ModBusDB>::iterator ite = pInstance->m_GroupModBusDBMap.begin();
			ite !=  pInstance->m_GroupModBusDBMap.end(); ite++)
		{
			unsigned short *num = new  unsigned short[32*32];
			memset(num, 0, 32*32);
			m_MapScreenState.insert(std::make_pair(ite->first, num));
		}
	}
	for (int i = 0; i < 32; i++)
	{
		m_stGroupSwitchState[i] = 0;
	}
	memset(m_stPtzState, 0, sizeof(m_stPtzState));
	memset(m_stIpcOnlineState, 0, sizeof(m_stIpcOnlineState));
	memset(m_stModbusIpcOnlineState, 0, sizeof(m_stModbusIpcOnlineState));
	memset(m_stModbusPtzState, 0, sizeof(m_stModbusPtzState));
	memset(m_bZoomAuto, 0, sizeof(m_bZoomAuto));
}

void CClient::DeleteOldState()
{
	for (std::map<int, unsigned short *>::iterator ite = m_MapScreenState.begin(); ite != m_MapScreenState.end(); ite++)
	{
		delete []ite->second;
	}
	m_MapScreenState.clear();

	for (std::map<int, unsigned char *>::iterator it = m_MapRecvData.begin(); it != m_MapRecvData.end(); it++)
	{
		delete [] it->second;
	}
	m_MapRecvData.clear();
}

//uint32 CClient::GetSequence(void)
//{ 
//	base::AtomicRefCountInc(&m_nIDGenerator);
//	return m_nIDGenerator;
//}

bool CClient::isSameDevice(const char* szIP, int nPort, int nRack, int nSlot)
{
	return m_strIp.compare(szIP) || (m_nPort != nPort) || (m_nRack != nRack) || (m_nSlot != nSlot);
}

void CClient::HeartBeetModbus()
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (m_bConnect)
	{
		//for 塞内加尔 屏蔽心跳
		if(pInstance->mModbusZoomOnly)
			return;
		for (std::map<int, ModBusDB>::iterator ite = pInstance->m_GroupModBusDBMap.begin();
			ite !=  pInstance->m_GroupModBusDBMap.end(); ite++)
		{
			int iIpcByteOffset = ite->second.addr4 - 40000;
			uint16 a = pInstance->m_usHeartBeet;
			int res = modbus_write_registers(m_ctx, iIpcByteOffset,1, &a);
		}

		pInstance->m_usHeartBeet++;
		if (pInstance->m_usHeartBeet > 255)
		{
			pInstance->m_usHeartBeet = 0;
		}
	}

}
//PLC login
int CClient::Login(/*const char *Ip, int nPort,  int nRack, int nSlot*/)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (m_bConnect)
	{
		return 1;
	}
	//=======tcp connect 2017/3/27============
	//if(pInstance->tcpConnect)
	//{
	//	pInstance->addrSrv.sin_family = AF_INET;  
	//	pInstance->addrSrv.sin_port = htons(m_nPort);  
	//	pInstance->addrSrv.sin_addr.S_un.S_addr = inet_addr(m_strIp.c_str());  
	//	pInstance->sockClient = socket(AF_INET, SOCK_STREAM, 0);  
	//	if(SOCKET_ERROR != pInstance->sockClient){  
	//				m_bConnect = TRUE;
	//		char szlog[MAX_STR_LEN] = {0};
	//		_snprintf(szlog, MAX_STR_LEN-1, "connect plc tcp success，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
	//		pInstance->m_plog->TraceInfo(szlog);
	//		pInstance->Showlog2Dlg("connect plc success", CONNECT_PLC_SUC);
	//		return 1;  
	//	}  
	//	if (GetTickCount() - m_nCurrentTime >= 30000)
	//	{
	//		m_nCurrentTime = GetTickCount();
	//		char szlog[MAX_STR_LEN] = {0};
	//		_snprintf(szlog, MAX_STR_LEN-1, "connect plc tcp failed，ip: %s, port: %d, error: %d", m_strIp.c_str(), m_nPort,WSAGetLastError());
	//		pInstance->m_plog->TraceInfo(szlog);
	//		pInstance->Showlog2Dlg("connect plc failed", CONNECT_PLC_ERR);
	//	}
	//	return -1;
	//}
	//==========================================================
	_fds.rfd = openSocket(m_nPort, m_strIp.c_str());
	//errno=0;    
	//int opt=1;
	//int res=setsockopt((SOCKET)(fds.rfd), SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
	//LOG3("setsockopt %s %d\n", strerror(errno),res);
	_fds.wfd=_fds.rfd;
	int useProtocol = daveProtoISOTCP;

	if (_fds.rfd>0) 
	{ 
		pInstance->m_plog->TraceInfo("openSocket successful");
		_di =daveNewInterface(_fds,"IF1",0, useProtocol, daveSpeed187k);
		daveSetTimeout(_di, 5000000);
		daveInitAdapter(_di);
		int Mpi = 2;
		_dc =daveNewConnection(_di,Mpi,m_nRack, m_nSlot);  // insert your rack and slot here
		char szconnlog[256] = {0};
		_snprintf(szconnlog, 255, "daveNewConnection: %d, Mpi=%d, nrack=%d, slot=%d", _dc, Mpi, m_nRack, m_nSlot);
		pInstance->m_plog->TraceInfo(szconnlog);
		if (0==daveConnectPLC(_dc))
		{
			m_bConnect = TRUE;

			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect plc success，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			pInstance->m_plog->TraceInfo(szlog);
			pInstance->Showlog2Dlg("connect plc success", CONNECT_PLC_SUC);

			return 1;
		}
		if (GetTickCount() - m_nCurrentTime >= 30000)
		{
			m_nCurrentTime = GetTickCount();
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect plc failed，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			pInstance->m_plog->TraceInfo(szlog);
			pInstance->Showlog2Dlg("connect plc failed", CONNECT_PLC_ERR);
		}
	}
	else
	{
		if (GetTickCount() - m_nCurrentTime >= 30000)
		{
			m_nCurrentTime = GetTickCount();
			pInstance->Showlog2Dlg("openSocket failed", CONNECT_PLC_ERR);
			pInstance->m_plog->TraceInfo("openSocket failed");
		}
	}
	//pInstance->Showlog2Dlg(1, "openSocket failed");
	return -1;
}
int CClient::LoginModbus(/*const char *Ip, int nPort,  int nRack, int nSlot*/)
{
	if (m_bConnect)
	{
		return 1;
	}
	CClientManager* pInstance = CClientManager::GetInstance();
	m_ctx = modbus_new_tcp(m_strIp.c_str(), m_nPort);
	if(m_ctx != NULL)
	{
		modbus_set_slave(m_ctx,1); //设置从设备ID
		if (-1 == modbus_connect(m_ctx)) 
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect modbus slave failed，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			if (GetTickCount() - m_nCurrentTime >= 30000)
			{
				m_nCurrentTime = GetTickCount();

				pInstance->m_plog->TraceInfo("modbus new tcp successful");
				pInstance->m_plog->TraceInfo(szlog);
			}
			pInstance->Showlog2Dlg(szlog);
			modbus_close(m_ctx);
			modbus_free(m_ctx); //释放连接
		}
		else
		{
			pInstance->m_plog->TraceInfo("modbus new tcp successful");
			m_bConnect = TRUE;
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect modbus slave success，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			pInstance->m_plog->TraceInfo(szlog);
			pInstance->Showlog2Dlg(szlog);
			return 1;
		}

	}
	else
		pInstance->m_plog->TraceInfo("modbus new tcp failed");

	return -1;
}


int CClient::Logout()
{
	if (m_bConnect)
	{
		daveDisconnectPLC(_dc);
		daveFree(_dc);
		daveDisconnectAdapter(_di);
		daveFree(_di);
		closeSocket(_fds.rfd);
		m_bConnect = FALSE;
	}
	return 0;
}
void CClient::WriteIpcStateFromAs300(const char* pIpcStatelist)
{
	if(m_bConnect)
	{
		std::string ipcStatusInfo(pIpcStatelist);
		std::vector<std::string> ipcInfoList;
		CClientManager::GetInstance()->StringSplit(ipcStatusInfo,",",ipcInfoList);
		if(ipcInfoList.size()==3)
		{
			int iGroup = _ttoi(ipcInfoList[0].c_str());
			int iIndex = _ttoi(ipcInfoList[1].c_str());
			int iState = _ttoi(ipcInfoList[2].c_str());
			if (CClientManager::GetInstance()->m_bBolModbus)
			{
				CheckWriteIpcStateModbus(iGroup, iIndex, iState);
			}
			else
			{
				CheckWriteIpcState(iGroup, iIndex, iState);	
			}
		}
	}
}
void CClient::WriteIpcState(const char* pIpcStatelist)
{
	if (m_bConnect)//给登录的设备写对应的ipc状态
	{
		Json::Value RevRoot;   
		Json::Reader reader;
		reader.parse(pIpcStatelist, RevRoot);
		int iSize = RevRoot.size();
		if (iSize <= 0)
		{
			return;
		}
		for (int i = 0; i < iSize; i++)
		{
			int iGroup = RevRoot[i]["group"].asInt();
			std::string schlId = RevRoot[i]["chlId"].asString();
			std::string sName = RevRoot[i]["name"].asString();
			int iState = RevRoot[i]["state"].asInt();
			//int iLocalId = RevRoot[i]["local_id"].asInt();
			int iIndex = RevRoot[i]["local_id"].asInt()%10000;
			if (CClientManager::GetInstance()->m_bBolModbus)
			{
				CheckWriteIpcStateModbus(iGroup, iIndex, iState);
			}
			else
			{
				CheckWriteIpcState(iGroup, iIndex, iState);	
			}
		}
	}
}

//DB 100-165表示33个摄像头信息 online or not
void CClient::CheckWriteIpcStateModbus(int iGroup, int iIndex, int iState)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	//2017/6/8 for塞内加尔 屏蔽操作
	if(pInstance->mModbusZoomOnly)
		return;
	if (iState != m_stModbusIpcOnlineState[iGroup][iIndex]) //默认状态都不在线0, 2:表示在线
	{
		std::map<int, ModBusDB>::iterator ite = pInstance->m_GroupModBusDBMap.find(iGroup);
		if(ite != pInstance->m_GroupModBusDBMap.end())
		{
			m_stModbusIpcOnlineState[iGroup][iIndex] = iState;
			int iIpcByteOffset = ite->second.addr4 - 40000 + pInstance->m_iModbusIpcStateOffSet+iIndex-1;
			uint16 a = iState == 2 ? 0x0001 : 0x0000;
			int res = modbus_write_registers(m_ctx, iIpcByteOffset,1, &a);

			//std::string str =  res >= 0 ? "success" : "failed";
			//char szlog[MAX_STR_LEN] = {0};
			//_snprintf(szlog, MAX_STR_LEN-1, "write group:%d, ipc:%d, state:%d %s", iGroup, iIndex, iState, str.c_str());
			//pInstance->m_plog->TraceInfo(szlog);
		}
	}
}

//DB 100-165表示33个摄像头信息 online or not
void CClient::CheckWriteIpcState(int iGroup, int iIndex, int iState)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (iState != m_stIpcOnlineState[iGroup][iIndex]) //默认状态都不在线0, 2:表示在线
	{
		std::map<int, int>::iterator ite = pInstance->m_GroupDBMap.find(iGroup);
		if (ite != pInstance->m_GroupDBMap.end())
		{
			m_stIpcOnlineState[iGroup][iIndex] = iState;
			int iIpcByteOffset = pInstance->m_iIpcStateOffSet + (iIndex-1)*2;
			int a = iState == 2 ? 0x0100 : 0x0000;
			int res = daveWriteBytes(_dc, /*daveAnaOut*/daveDB, ite->second, iIpcByteOffset, 2, &a);//(DB501，第100字节始，2个b字节，值为a)


			std::string str =  iState == 2 ? "online" : "offline";
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "write group:%d, ipcIndex:%d, state: %s", iGroup, iIndex, str.c_str());
			pInstance->SendLog2Dlg(szlog);
			pInstance->m_plog->TraceError(szlog);
			//pInstance->m_plog->TraceInfo(szlog);
		}
	}
}

void CClient::ReadPLCDataProcess() //连接成功的话就定时获取信息
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (m_bConnect)
	{
		//按byte读取
		for (std::map<int, int>::iterator ite = pInstance->m_GroupDBMap.begin(); 
			ite != pInstance->m_GroupDBMap.end(); 
			ite++)
		{

			unsigned char buffer[256] = {0};
			//unsigned char buffer[300]={0};
			//char iGroupSwitch[4] = {0};
			int nDB = ite->second;

			int nOffset = pInstance->m_PLCFormatInfo.nOffSet;
			int nByteNum = pInstance->m_PLCFormatInfo.nByteNum;
#ifdef GET_READPLC_TIME
			clock_t start,finish;
			double totaltime;
			start=clock();
#endif
			int res = daveReadManyBytes(_dc, daveDB, nDB, nOffset, nByteNum,  buffer);
#ifdef GET_READPLC_TIME
			finish=clock();
			totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
			char timeLog[200];
			sprintf_s(timeLog,sizeof(timeLog)-1,"read plc db:%d, time consume:%f s",nDB,totaltime);
			pInstance->Showlog2Dlg(timeLog,CONNECT_PLC_REC);
#endif
			if(res >= 0)
			{
				pInstance->m_nPLCHeartBeat = pInstance->GetSysTimeMicros()/1000;
				//char szlog[MAX_STR_LEN] = {0};//测试时写的数据
				//sprintf_s(szlog, MAX_STR_LEN, "group-%d;all_data",ite->first);
				//pInstance->m_plog->TraceInfo(szlog);
				//pInstance->m_plog->WriteFile(buffer, szlog, nByteNum);//写读取到的数据

#ifdef  _showData // 2017/03/15 add dataViewer
				char dataBuffer[BLOCK_SIZE_COPY]={0};
				memcpy(dataBuffer,buffer,sizeof(dataBuffer));
				dataBuffer[255]= nDB-500;
				CClientManager::GetInstance()->Showlog2Dlg(dataBuffer,CONNECT_PLC_DATA);
#endif

				int iGroup = ite->first; 
				unsigned char *pBuff = (unsigned char *)buffer;		//整体切屏幕  188bit buffer[188],
				//根据该值确定显示到那个工作台的对应屏幕

				////发送给第三方平台
				//if (0 != pInstance->m_nSendSwitch)
				//{
				//	SendToThirdDevice(iGroup, pBuff, nDB, nOffset, nByteNum);
				//	if (1 == pInstance->m_nSendSwitch)
				//	{
				//		continue;
				//	}
				//}
				unsigned int usSwitchOver = (pBuff[pInstance->m_SwitchGroupAddr])<<8 |
					(pBuff[pInstance->m_SwitchGroupAddr+1]);

				// 未绑定任何台号，不作处理 
				// by xionggao.lee @2017.01.02
				if (m_stGroupSwitchState[iGroup]==0&&0 == usSwitchOver )		
					continue;
				if (m_stGroupSwitchState[iGroup] != usSwitchOver)  //对应的组号和切换位不相等，查看是否有变化
				{
					if (0 == usSwitchOver)
					{
						CloseAllIpcInGroup(m_stGroupSwitchState[iGroup], 2);//组号设成0关闭分组 2017/4/13
						m_stGroupSwitchState[iGroup] = usSwitchOver;
						//=======2017/6/2 首次不进行状态切换
						std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(iGroup);
						if(iteFirst!=pInstance->mGroupSwitchFirst.end())
						{
							if(iteFirst->second)
							{
								iteFirst->second = false;
								m_stGroupSwitchState[iGroup]= usSwitchOver;
							}
						}
						continue;
					}
					//=======2017/6/2 首次不进行状态切换
					std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(iGroup);
					if(iteFirst!=pInstance->mGroupSwitchFirst.end())
					{
						if(iteFirst->second)
						{
							//iteFirst->second = false;
							m_stGroupSwitchState[iGroup]= usSwitchOver;
							continue;
						}
					}
					ClearPriority(iGroup);
					//CloseAllIpcInGroup(m_stGroupSwitchState[iGroup], 2);
					std::map<int, unsigned short *>::iterator ite = m_MapScreenState.find(m_stGroupSwitchState[iGroup]);//初始化一下切屏对方的old值
					if (ite != m_MapScreenState.end())
					{
						memset(ite->second, 0, 32*32);
					}

					m_stGroupSwitchState[iGroup] = usSwitchOver;
					std::map<int, unsigned short *>::iterator iteState = m_MapScreenState.find(iGroup);//初始化一下切屏的old值
					if (iteState != m_MapScreenState.end())
					{
						memset(iteState->second, 0, 32*32);
					}
				}


				if(!FreeModeSwitch(iGroup,pBuff,usSwitchOver))//2017/5/5 如果特殊模式点置位，屏蔽常规切屏
					SwitchScreen(iGroup, pBuff, usSwitchOver);//屏幕切换上墙

				IpcPtzOperation(iGroup,pBuff, usSwitchOver);//ptz相关操作

				IpcZoomOperation(iGroup,pBuff, usSwitchOver); //zoom值变

				//===========add switch screen with height 添加随高度切屏=====================================
				SwitchScreenHeight(iGroup,pBuff,usSwitchOver);//随高度自动切屏
				//====================锁头跟随 2016/12/19=============================//
				IpcPtzFollowOperation(iGroup,pBuff,usSwitchOver);
				//===================TextBar 2017/4/8================================
				textBarDataSend(iGroup,pBuff,usSwitchOver);
				//===================preset point call 2017/5/24======================
				PresetPointCall(iGroup,pBuff,usSwitchOver);
				/////////////////////自由切屏///////////////////////////////////////////
				FreeCutScreen(iGroup, pBuff, usSwitchOver);	//自由切屏

			}
			else
			{
				m_bConnect = FALSE;
				pInstance->Showlog2Dlg("read DB data failed, reConnect!", CONNECT_PLC_REC);
				return;
			}
		}
		if(CClientManager::GetInstance()->groupDataRefresh)
		{
			CClientManager::GetInstance()->InitGroupData();
			CClientManager::GetInstance()->groupDataRefresh = false;
		}	
	}
}
void CClient::ClearPriority(int nGroup)	//清除优先级
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int iScreenNum = pInstance->m_PLCFormatInfo.nScreenNum;

	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
	{

		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = pInstance->m_mapPriority.find(iIPCPriority);
		if (itePriority != pInstance->m_mapPriority.end())
		{
			itePriority->second.iAddr = 0;
			itePriority->second.iBit = 0;
			itePriority->second.iMode = 0;
			itePriority->second.iPriority = 0;
		}
		else
		{
			PlcScreenAddrInfo sInfo;
			pInstance->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}
	}
}
void CClient::ReadModbusDataProcess () //连接成功的话就定时获取信息
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (m_bConnect)
	{
		for (std::map<int, ModBusDB>::iterator ite = pInstance->m_GroupModBusDBMap.begin(); 
			ite != pInstance->m_GroupModBusDBMap.end(); 
			ite++)
		{
			pInstance->m_KeyId = m_iServerKeyID;
			int nGroup = ite->first;

			int nOffset1 = -1;
			int nByteNum1 = 0;
			if (ite->second.addr1 != -1)
			{
				nOffset1 = ite->second.addr1 - 10000;			 
				nByteNum1 = pInstance->m_ModBusFormatInfo.nByteNum1;
			}

			int nOffset2 = -1;
			int nByteNum2 = 0;
			if (ite->second.addr3 != -1)
			{
				nOffset2 = ite->second.addr3 - 30000;
				nByteNum2 = pInstance->m_ModBusFormatInfo.nByteNum2;
			}

			int nOffset3 = -1;
			int nByteNum3 = 0;
			if (ite->second.addr4  != -1)
			{
				nOffset3 = ite->second.addr4 - 40000;
				nByteNum3 = pInstance->m_ModBusFormatInfo.nByteNum3;
			}

			unsigned short usSwitchOver;
			//==2017/6/8 zoom Onlt for 塞内加尔
			if(pInstance->mModbusZoomOnly)
			{
				uint16 zoomBuff[256] = {0};
				int res = modbus_read_registers(m_ctx, nOffset3, nByteNum3+1, zoomBuff);
				if (-1 != res)
				{
					IpcZoomOperationOnlyModbus(nGroup,zoomBuff[pInstance->mZoomOnlyHeightAddr],zoomBuff[pInstance->mZoomOnlyFeetAddr],nGroup);
				}
				else
				{
					m_bConnect = FALSE;
					modbus_close(m_ctx);
					modbus_free(m_ctx); //释放连接
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "read modbus data failed, reConnect! addr: 30000, ip: %s, port: %d", m_strIp.c_str(), m_nPort);
					pInstance->Showlog2Dlg(szlog);
					return;
				}
				continue;
			}

			uint16 buff[256] = {0};

			int res = modbus_read_input_registers(m_ctx, nOffset2, nByteNum2+1, buff);
			if (-1 != res)
			{
				usSwitchOver = buff[pInstance->m_iModbusSwitchGroupAddr];

				if (m_stGroupSwitchState[nGroup] != usSwitchOver)  //对应的组号和切换位不相等，查看是否有变化
				{
					if (0 == usSwitchOver)
					{
						m_stGroupSwitchState[nGroup] = usSwitchOver;
						continue;
					}
					ClearPriority(nGroup);
					CloseAllIpcInGroup(m_stGroupSwitchState[nGroup], 2);
					std::map<int, unsigned short *>::iterator ite = m_MapScreenState.find(m_stGroupSwitchState[nGroup]);//初始化一下切屏对方的old值
					if (ite != m_MapScreenState.end())
					{
						memset(ite->second, 0, 32*32);
					}

					m_stGroupSwitchState[nGroup] = usSwitchOver;
					std::map<int, unsigned short *>::iterator iteState = m_MapScreenState.find(nGroup);//初始化一下切屏的old值
					if (iteState != m_MapScreenState.end())
					{
						memset(iteState->second, 0, 32*32);
					}
					//CloseAllIpcInGroup(usSwitchOver, 2);
				}

			}
			else
			{
				m_bConnect = FALSE;
				modbus_close(m_ctx);
				modbus_free(m_ctx); //释放连接
				char szlog[MAX_STR_LEN] = {0};
				_snprintf(szlog, MAX_STR_LEN-1, "read modbus data failed, reConnect! addr: 30000, ip: %s, port: %d", m_strIp.c_str(), m_nPort);
				pInstance->Showlog2Dlg(szlog);
				return;
			}

			uint8 buf[256] = {0};

			int re = modbus_read_input_bits(m_ctx, nOffset1, nByteNum1+1, buf);
			if (-1 != re)
			{
				unsigned char *pBuff = (unsigned char *)buf;

				SwitchScreenModbus(nGroup, pBuff, usSwitchOver);//屏幕切换上墙

				IpcPtzOperationModbus(nGroup,pBuff, usSwitchOver);//ptz相关操作

				IpcZoomOperationModbus(nGroup,pBuff, buff, usSwitchOver); //zoom值变
			}
			else
			{
				m_bConnect = FALSE;
				modbus_close(m_ctx);
				modbus_free(m_ctx); //释放连接
				char szlog[MAX_STR_LEN] = {0};
				_snprintf(szlog, MAX_STR_LEN-1, "read modbus data failed, reConnect! addr: 10000, ip: %s, port: %d", m_strIp.c_str(), m_nPort);
				pInstance->Showlog2Dlg(szlog);
				return;
			}	
		}
	}
}
void CClient::SendToThirdDevice(int nGroup, const unsigned char *pBuff, int nDB, int nOffSet, int nByteNum)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if (pBuff == NULL || nByteNum <= 0)
	{
		return;
	}

	std::map<int , unsigned char *>::iterator ite = m_MapRecvData.find(nGroup);
	if (ite == m_MapRecvData.end())
	{
		return;
	}
	if (0 == memcmp(ite->second, pBuff, nByteNum))	//数据相同
	{
		return;
	}
	//memcpy(ite->second, pBuff, nByteNum);

	/////////包头/////////////////
	typedef struct sendDATA{
		int   identification;	//协议标识 4
		short type;				//命令类型 2
		short DB;				//DB数据块号2
		short seek;				//数据偏移量2
		short num;				//序号2
		int   size;				//数据长度4
	}sendDATA;

	sendDATA sendbuffer;
	int len = 16 + nByteNum;

	sendbuffer.identification = htonl(0xFA010000);
	sendbuffer.type = htons(0x0001);
	sendbuffer.DB = htons(nDB);
	sendbuffer.seek = htons(nOffSet);
	sendbuffer.num = htons(++m_nNumIndex);
	sendbuffer.size = htonl(nByteNum);

	char *buffer = new char[len];

	memcpy(buffer, (const void *)&sendbuffer, sizeof(sendbuffer));
	memcpy(buffer+16, pBuff, nByteNum);

	for (std::map<int, ThirdADDR>::iterator it=pInstance->m_MapThirdAddr.begin(); it != pInstance->m_MapThirdAddr.end(); it++)
	{
		net.SetSendSocket(it->second.Port, it->second.strIP.c_str(), EUDP_Normal);
		if (/*-1 == net.SendTo(buffer, len)*/ net.SendTo(buffer,len)!=len)
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "error: sendto third device fail IP:%s , Port:%d", it->second.strIP, it->second.Port);
			pInstance->Showlog2Dlg(szlog);
		}
		else
		{
			//2017/5/10  发送成功记录，不成功不记录下次继续发送
			memcpy(ite->second, pBuff, nByteNum);
		}
		net.DisConnect();
	}
	delete [] buffer;
}
void CClient::FreeCutScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup)	//自由切屏功能
{
	CClientManager* pInstance = CClientManager::GetInstance();
	for (int i = 1; i <= pInstance->m_nMaxIndex; i++)
	{
		//int iMapIndex = (nGroup<<8)|i;
		//std::map<int, CutScreenInfo>::iterator ite = pInstance->m_MapFreeCutScreen.find(iMapIndex);
		//if (ite == pInstance->m_MapFreeCutScreen.end())
		//{
		//	continue;
		//}

		////unsigned short iIPC = (*(pBuf+ite->second.OffSet))&0xff;	
		//unsigned short iIPC = (pBuf[ite->second.OffSet])<<8|(pBuf[ite->second.OffSet+1]);	

		//FreeCutScreenToTVwall(nGroup, iIPC, iMapIndex, i, iSwitchGroup);

		//2017/5/5 修改索引方式 兼容多个屏幕自由切屏
		for(int iScreen=1;iScreen<pInstance->m_PLCFormatInfo.nScreenNum;iScreen++)
		{
			int iMapIndex = nGroup;
			iMapIndex=(iMapIndex<<8)|iScreen;
			iMapIndex=(iMapIndex<<8)|i;
			std::map<int, CutScreenInfo>::iterator ite = pInstance->m_MapFreeCutScreen.find(iMapIndex);
			if (ite == pInstance->m_MapFreeCutScreen.end())
			{
				continue;
			}

			//unsigned short iIPC = (*(pBuf+ite->second.OffSet))&0xff;	
			unsigned short iIPC = 0;
			iIPC = (pBuf[ite->second.OffSet])<<8|(pBuf[ite->second.OffSet+1]);	

			FreeCutScreenToTVwall(nGroup, iIPC, iMapIndex, i, iSwitchGroup);
		}

	}
}
void CClient::FreeCutScreenToTVwall(int nGroup, unsigned short iIPC, int iMapIndex,  int i, int iSwitchGroup)			//发送自由切屏信息给TVWALL
{
	CClientManager* pInstance = CClientManager::GetInstance();
	std::map<int, CutScreenInfo>::iterator ite = pInstance->m_MapFreeCutScreen.find(iMapIndex);
	if (ite == pInstance->m_MapFreeCutScreen.end())
	{
		return;
	}

	if(iIPC == ite->second.IPCIndex && ite->second.SwitchGroup == iSwitchGroup)
	{
		return;		//没有变化
	}
	if(iIPC != ite->second.IPCIndex && 0 != iIPC  || ite->second.SwitchGroup != iSwitchGroup && 0 != iSwitchGroup)
	{
		int iGroupIpcId;
		if (0 != iIPC)
		{
			iGroupIpcId = (nGroup<<8)|iIPC;
		}
		else
		{
			iGroupIpcId = (nGroup<<8)|ite->second.IPCIndex;
		}

		BOOL bol = FALSE;
		GroupIPCIDMap::iterator iteGroupIpcId =  pInstance->m_GroupIpcId.find(iGroupIpcId);
		if (iteGroupIpcId == pInstance->m_GroupIpcId.end())
		{
			if (ite->second.SwitchGroup != iSwitchGroup && 0 != iSwitchGroup)
			{
				int iOldMapIndex = (ite->second.SwitchGroup << 8)| i;		//初始化一下切屏对方的old值
				std::map<int, CutScreenInfo>::iterator it = pInstance->m_MapFreeCutScreen.find(iOldMapIndex);
				if (it != pInstance->m_MapFreeCutScreen.end())
				{
					it->second.SwitchGroup = 0;
				}
				ite->second.SwitchGroup = iSwitchGroup;
			}
			if (ite->second.IPCIndex != iIPC)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "error: Configuration file not have about IPC-%d information! Group: %d, Offset: %d", iIPC, nGroup, ite->second.OffSet);
				pInstance->m_plog->TraceInfo(szlog);
				pInstance->Showlog2Dlg(szlog);
				ite->second.IPCIndex = iIPC;
			}
			bol = TRUE;
		}

		int nScreenId = ite->second.ScreenID;
		int iGroupTVWall;

		if (iSwitchGroup != 0)
		{
			iGroupTVWall = (iSwitchGroup<<8)|nScreenId;
		}
		else
		{
			iGroupTVWall = (nGroup<<8)|nScreenId;
		}

		Group2PcMap::iterator iteGroup2PC = pInstance->m_Group2PcMap.find(iGroupTVWall);
		if (iteGroup2PC == pInstance->m_Group2PcMap.end())
		{  
			if (ite->second.SwitchGroup != iSwitchGroup)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "error: Configuration file does not have about this Group: %d information!", iSwitchGroup);
				pInstance->m_plog->TraceInfo(szlog);
				pInstance->Showlog2Dlg(szlog);
				ite->second.SwitchGroup = iSwitchGroup;
			}
			bol = TRUE;
		}
		if (bol)
		{
			return;		//iteGroupIpcId == pInstance->m_GroupIpcId.end()
		}

		std::string sIPCPuid = iteGroupIpcId->second.strPUID;
		int stream_type = iteGroupIpcId->second.stream_type;	//主副码流

		std::string strIP = iteGroup2PC->second.Ip;

		int iPort = iteGroup2PC->second.Port;
		int iTVScreenID = iteGroup2PC->second.iTVSSCREENID;

		std::map<int, FreeCutInfo>::iterator iteGroupScreen = pInstance->m_GroupScreenMap.find(nGroup);
		if (iteGroupScreen == pInstance->m_GroupScreenMap.end())
		{
			return;
		}
		if (ite->second.monitor_ID <= iteGroupScreen->second.CutNum)	//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
		{	
			std::string sRetVec;
			sRetVec.append("{");
			SetValue(sRetVec, "key_id", pInstance->m_KeyId);
			SetValue(sRetVec, "command", "MAP_IPC", FALSE);

			sRetVec.append("\"maps\":[");

			sRetVec.append("{");
			SetValue(sRetVec, "ipc_id", sIPCPuid.c_str(), FALSE);
			SetValue(sRetVec, "monitor_id", ite->second.monitor_ID + 64*(iTVScreenID-1), FALSE);
			//SetValue(sRetVec, "stream_type", stream_type, TRUE);
			//=============textBar 2017/4/7===================================
			pInstance->TextIpcRetVecGet(sRetVec,nGroup,sIPCPuid,stream_type,ite->second.monitor_ID + 64*(iTVScreenID-1));
			//===============================================================
			sRetVec.append("},");

			sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
			sRetVec.append("]}");
			pInstance->SendInfo2TVWALLServer(sRetVec, strIP, iPort);
		}
		if (ite->second.SwitchGroup != iSwitchGroup && 0 != iSwitchGroup)
		{
			int iOldMapIndex = (ite->second.SwitchGroup << 8)| i;		//初始化一下切屏对方的old值
			std::map<int, CutScreenInfo>::iterator it = pInstance->m_MapFreeCutScreen.find(iOldMapIndex);
			if (it != pInstance->m_MapFreeCutScreen.end())
			{
				it->second.SwitchGroup = 0;
			}
		}

		if (0 != iIPC)
		{
			ite->second.IPCIndex = iIPC;		//更新值
		}
		if (0 != iSwitchGroup)
		{
			ite->second.SwitchGroup = iSwitchGroup;
		}

		ite->second.strIP = strIP;
		ite->second.iPort = iPort;
		ite->second.iTVScreenID = iTVScreenID;
		ite->second.sIPCPuid = sIPCPuid;
		ite->second.nGroup = nGroup;
		ite->second.stream_type = stream_type;
	}	
}
void CClient::SwitchScreenModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int iScreenNum = pInstance->m_ModBusFormatInfo.nScreenNum;
	int iScreenModeMax = pInstance->m_ModBusFormatInfo.nModeMax;
	int iModeValue = 0;
	int iAddr = 0;

	// 检查所有屏幕
	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
	{
		int iBeforeVla = 0;
		bool bBol = false;
		PlcScreenAddrInfo sInfo;
		int iState = 0; //同一个屏幕两种及以上的模式都被置位或者该屏幕没有模式被置位
		int iMode = 0;
		std::map<int, unsigned short *>::iterator iteState =  m_MapScreenState.find(nGroup);
		if (iteState == m_MapScreenState.end())
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "not save group_%d old state", nGroup);
			pInstance->m_plog->TraceInfo(szlog);
			return;
		}

		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = pInstance->m_mapPriority.find(iIPCPriority);
		if (itePriority == pInstance->m_mapPriority.end())
		{
			pInstance->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}

		for(int iScreenMode = 0; iScreenMode <= iScreenModeMax; iScreenMode++)
		{
			int iPCSreenModeAddr =(iScreenId<<8)|iScreenMode;	
			ModbusScreenAddrMap::iterator iteScreenAddr = pInstance->m_ModbusScreenAddMap.find(iPCSreenModeAddr);
			if (iteScreenAddr == pInstance->m_ModbusScreenAddMap.end())
			{   
				continue;
			}
			iAddr = iteScreenAddr->second.iAddr;
			iModeValue = pBuf[iAddr] && 0xff;

			if(iModeValue == 1)
			{
				iState++;
				iMode = iScreenMode;
			}

			// notify 优先级开关 2017/5/3 ADD pInstance->priorityEnable
			if (iModeValue && sInfo.iPriority <= iteScreenAddr->second.iPriority/*&&pInstance->priorityEnable*/)	//筛选出最高优先级做响应
			{
				sInfo.iAddr = iteScreenAddr->second.iAddr;
				sInfo.iMode = iScreenMode;
				sInfo.iPriority = iteScreenAddr->second.iPriority;
			}

			if(iModeValue == iteState->second[iScreenId*32 + iScreenMode])//没有变化
			{
				continue;
			}

			bBol = true;
			iBeforeVla = iteState->second[iScreenId*32 + iScreenMode];
			iteState->second[iScreenId*32 + iScreenMode] = iModeValue;
		}

		PlcScreenAddrMap::iterator itePRI = pInstance->m_mapPriority.find(iIPCPriority);
		// notify 优先级开关 2017/5/3 ADD pInstance->priorityEnable
		if (itePRI->second.iPriority == sInfo.iPriority && sInfo.iPriority != 0/*&&pInstance->priorityEnable*/)	//如果最高优先级模式改变
		{
			continue;
		}
		itePRI->second.iPriority = sInfo.iPriority;	//保留最高优先级的模式
		itePRI->second.iAddr = sInfo.iAddr;
		itePRI->second.iBit = sInfo.iBit;
		itePRI->second.iMode = sInfo.iMode;

		//日志
		if (bBol && sInfo.iPriority != 0)/*switch mode*/
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "group-%d; screen_id-%d; byteNum-%d;Value-%d-%d",nGroup, iScreenId, sInfo.iAddr, iBeforeVla, iteState->second[iScreenId*32 + sInfo.iMode]);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			pInstance->Showlog2Dlg(szlog);
			//pInstance->m_plog->WriteFile(pBuf, szlog);//写读取到的数据

			iteState->second[iScreenId*32 + 0] = 0;
			pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/iSwitchGroup,iScreenId,sInfo.iMode,nGroup);
		}

		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0))
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			pInstance->Showlog2Dlg(szlog);

			iteState->second[iScreenId*32 + 0] = 1;
			pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/iSwitchGroup,iScreenId,0,nGroup);
		}	
	}
}

// void CClient::SwitchScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
// {
// 	CClientManager* pInstance = CClientManager::GetInstance();
// 	int iScreenNum = pInstance->m_PLCFormatInfo.nScreenNum;
// 	int iScreenModeMax = pInstance->m_PLCFormatInfo.nModeMax;
//     int iAddr = 0;
// 	int iBit = 0;
// 	int iModeValue = 0;
// 	int nSwitchMode = 0;
// 	
// 	struct bytebit
// 	{
// 		byte bit0:1;
// 		byte bit1:1;
// 		byte bit2:1;
// 		byte bit3:1;
// 		byte bit4:1;
// 		byte bit5:1;
// 		byte bit6:1;
// 		byte bit7:1;
// 	};
// 
// 	union UBytebit
// 	{
// 		byte	nByte;
// 		bytebit nBit;
// 	};
// 	// 调试代码，仅用于显示指定字节的所有位
// // 	if (nGroup == 2)
// // 	{
// // 		UBytebit UByte;
// // 		UByte.nByte = pBuf[65];
// // 		char szlog[MAX_STR_LEN] = {0};
// // 		char szByteString[32] = {0};
// // 		_snprintf(szByteString,32,"%d-%d-%d-%d-%d-%d-%d-%d",UByte.nBit.bit7,UByte.nBit.bit6,UByte.nBit.bit5,UByte.nBit.bit4,UByte.nBit.bit3,UByte.nBit.bit2,UByte.nBit.bit1,UByte.nBit.bit0);
// // 		_snprintf(szlog, MAX_STR_LEN-1, "group-%d; byteNum-65:%s; iSwitchGroup- %d ",nGroup,szByteString,iSwitchGroup);
// // 		pInstance->Showlog2Dlg(szlog);
// // 	}
// 
// 	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
// 	{
// 		int iBeforeVal = 0;
// 		bool bBol = false;
// 		PlcScreenAddrInfo sInfo;
// 		int iState = 0; //同一个屏幕两种及以上的模式都被置位或者该屏幕没有模式被置位
// 		int iMode = 0;
// 		std::map<int, unsigned short *>::iterator iteState =  m_MapScreenState.find(nGroup);
// 		if (iteState == m_MapScreenState.end())
// 		{
// 			char szlog[MAX_STR_LEN] = {0};
// 			_snprintf(szlog, MAX_STR_LEN-1, "not save group_%d old state", nGroup);
// 			pInstance->m_plog->TraceSWITCHInfo(szlog);
// 			return;
// 		}
// 	
// 		int iIPCPriority = (nGroup << 8)|iScreenId;
// 		if (pInstance->m_bEnablePriority)
// 		{
// 			PlcScreenAddrMap::iterator itePriority = pInstance->m_mapPriority.find(iIPCPriority);
// 			if (itePriority == pInstance->m_mapPriority.end())
// 			{
// 				pInstance->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
// 			}
// 		}
// 
// 		for(int nIndex = 0; nIndex <= iScreenModeMax; nIndex++)
// 		{
// 			int iPCSreenModeAddr =(iScreenId<<8)|nIndex;	
// 			PlcScreenAddrMap::iterator iteScreenAddr = pInstance->m_PlcScreenAddMap.find(iPCSreenModeAddr);
// 			if (iteScreenAddr == pInstance->m_PlcScreenAddMap.end())
// 			{   
// 				continue;
// 			}
// 			iAddr = iteScreenAddr->second.iAddr;
// 			iBit = iteScreenAddr->second.iBit;
// 			char ucBuffValue = pBuf[iAddr];
// 			iModeValue =  ( ucBuffValue >>  iBit) & 0x1;
// 	
// 			if(iModeValue == 1)
// 			{
// 				iState++;
// 				iMode = nIndex;
// 			}
// 			
// 			if (pInstance->m_bEnablePriority)
// 			{
// 				if (iModeValue && sInfo.iPriority <= iteScreenAddr->second.iPriority)	//筛选出最高优先级做响应
// 				{
// 					
// 					sInfo.iPriority = iteScreenAddr->second.iPriority;
// 				}
// 			}
// 
// 			iBeforeVal = iteState->second[iScreenId*32 + nIndex];
// 			if(iModeValue == iBeforeVal)//没有变化
// 			{
// 				continue;
// 			}
// 			nSwitchMode = nIndex;
// 			// 保留模式和字节位信息
// 			sInfo.iAddr = iteScreenAddr->second.iAddr;
// 			sInfo.iBit = iteScreenAddr->second.iBit;
// 			sInfo.iMode = nIndex;
// 			USHORT *pModeArray =  iteState->second;
// 		    iteState->second[iScreenId*32 + nIndex] = iModeValue;
// 			bBol = true;
// 
// 			if (pInstance->m_bEnablePriority)
// 			{
// 				PlcScreenAddrMap::iterator itePRI = pInstance->m_mapPriority.find(iIPCPriority);
// 				if (itePRI->second.iPriority == sInfo.iPriority && sInfo.iPriority != 0)	//如果最高优先级模式改变
// 				{
// 					continue;
// 				}
// 				itePRI->second.iPriority = sInfo.iPriority;	//保留最高优先级的模式
// 				itePRI->second.iAddr = sInfo.iAddr;
// 				itePRI->second.iBit = sInfo.iBit;
// 				itePRI->second.iMode = sInfo.iMode;
// 			}
// 
// 			UBytebit UByte;
// 			UByte.nByte = pBuf[65];
// 			//日志
// 			if (bBol)/*switch mode*/
// 			{
// 				bool bSwitch = false;
// 				if (pInstance->m_bEnablePriority )
// 				{
// 					if (sInfo.iPriority != 0)
// 						bSwitch = true;
// 				}
// 				else
// 				{
// 					bSwitch = true;
// 				}
// 				if (bSwitch)
// 				{
// 					char szlog[MAX_STR_LEN] = {0};
// 					_snprintf(szlog, MAX_STR_LEN-1, "group-%d; screen_id-%d; byteNum-%d; bitNum-%d; bitValue-%d-%d; iSwitchGroup- %d ",nGroup, iScreenId, sInfo.iAddr, sInfo.iBit, iBeforeVal, iteState->second[iScreenId*32 + sInfo.iMode],iSwitchGroup);
// 					pInstance->m_plog->TraceSWITCHInfo(szlog);
// 					pInstance->Showlog2Dlg(szlog);
// 					iteState->second[iScreenId*32 + 0] = 0;
// 					pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/
// 						iSwitchGroup,iScreenId,nSwitchMode,nGroup);
// 				}
// 			}
// 		}
// 		
// 		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
// 		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0))
// 		{
// 			char szlog[MAX_STR_LEN] = {0};
// 
// 			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
// 			pInstance->m_plog->TraceSWITCHInfo(szlog);
// 			pInstance->Showlog2Dlg(szlog);
// 
// 			iteState->second[iScreenId*32 + 0] = 1;
// 			pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/
// 				iSwitchGroup,iScreenId,0,nGroup);
// 		}	
// 	}
// }

void CClient::SwitchScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int iScreenNum = pInstance->m_PLCFormatInfo.nScreenNum;
	int iScreenModeMax = pInstance->m_PLCFormatInfo.nModeMax;
	int iAddr = 0;
	int iBit = 0;
	int iModeValue = 0;

	//=======2017/6/2 首次不进行状态切换
	bool firstSwitch;
	std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(nGroup);
	if(iteFirst!=pInstance->mGroupSwitchFirst.end())
	{
		if(iteFirst->second)
		{
			iteFirst->second = false;
			firstSwitch = true;
		}
		else
		{
			firstSwitch = false;
		}
	}

	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
	{
		int iBeforeVal = 0;
		bool bBol = false;
		PlcScreenAddrInfo sInfo;
		int iState = 0; //同一个屏幕两种及以上的模式都被置位或者该屏幕没有模式被置位
		int iMode = 0;
		std::map<int, unsigned short *>::iterator iteState =  m_MapScreenState.find(nGroup);
		if (iteState == m_MapScreenState.end())
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "not save group_%d old state", nGroup);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			return;
		}

		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = pInstance->m_mapPriority.find(iIPCPriority);
		if (itePriority == pInstance->m_mapPriority.end())
		{
			pInstance->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}

		for(int iScreenMode = 0; iScreenMode <= iScreenModeMax; iScreenMode++)
		{
			int iPCSreenModeAddr =(iScreenId<<8)|iScreenMode;	
			PlcScreenAddrMap::iterator iteScreenAddr = pInstance->m_PlcScreenAddMap.find(iPCSreenModeAddr);
			if (iteScreenAddr == pInstance->m_PlcScreenAddMap.end())
			{   
				//========2017/5/10 for特殊模式点恢复
				if(iScreenMode!=0)
				{
					iteState->second[iScreenId*32 + iScreenMode] = 0;
				}
				//==========
				continue;
			}
			iAddr = iteScreenAddr->second.iAddr;

			iBit = iteScreenAddr->second.iBit;
			unsigned char ucBuffValue = pBuf[iAddr];
			iModeValue =  ( ucBuffValue >>  iBit) & 0x1;

			if(iModeValue == 1)
			{
				iState++;
				iMode = iScreenMode;
			}

			//notify 优先级开关 2017/5/3 add  pInstance->m_bEnablePriority
			if (iModeValue)	//筛选出最高优先级做响应
			{
				if(pInstance->m_bEnablePriority)
				{
					if(sInfo.iPriority <= iteScreenAddr->second.iPriority)
					{
						sInfo.iAddr = iteScreenAddr->second.iAddr;
						sInfo.iBit = iteScreenAddr->second.iBit;
						sInfo.iMode = iScreenMode;
						sInfo.iPriority = iteScreenAddr->second.iPriority;
					}
				}
				else
				{
					sInfo.iAddr = iteScreenAddr->second.iAddr;
					sInfo.iBit = iteScreenAddr->second.iBit;
					sInfo.iMode = iScreenMode;
					sInfo.iPriority = iteScreenAddr->second.iPriority;
				}

			}

			if(iModeValue == iteState->second[iScreenId*32 + iScreenMode])//没有变化
			{
				continue;
			}

			bBol = true;
			iBeforeVal = iteState->second[iScreenId*32 + iScreenMode];
			iteState->second[iScreenId*32 + iScreenMode] = iModeValue;
		}

		PlcScreenAddrMap::iterator itePRI = pInstance->m_mapPriority.find(iIPCPriority);
		//notify 优先级开关 2017/5/3 add  pInstance->m_bEnablePriority
		if(pInstance->m_bEnablePriority)
		{
			if (itePRI->second.iPriority == sInfo.iPriority && sInfo.iPriority != 0 )	//如果最高优先级模式改变
			{
				continue;
			}
		}

		itePRI->second.iPriority = sInfo.iPriority;	//保留最高优先级的模式
		itePRI->second.iAddr = sInfo.iAddr;
		itePRI->second.iBit = sInfo.iBit;
		itePRI->second.iMode = sInfo.iMode;

		//日志
		if (bBol)/*switch mode*/
		{
			bool bSwitch = false;
			if (pInstance->m_bEnablePriority )
			{
				if (sInfo.iPriority != 0)
					bSwitch = true;
			}
			else
			{
				bSwitch = true;
			}
			if (bSwitch)
			{
				char szlog[MAX_STR_LEN] = {0};
				_snprintf(szlog, MAX_STR_LEN-1, "group-%d; screen_id-%d; byteNum-%d; bitNum-%d; bitValue-%d-%d",nGroup, iScreenId, sInfo.iAddr, sInfo.iBit, iBeforeVal, iteState->second[iScreenId*32 + sInfo.iMode]);
				pInstance->m_plog->TraceSWITCHInfo(szlog);
				pInstance->Showlog2Dlg(szlog);
				iteState->second[iScreenId*32 + 0] = 0;
				//=======2017/6/2 首次不进行状态切换
				if(!firstSwitch)
					pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/iSwitchGroup,iScreenId,sInfo.iMode,nGroup);
			}
		}

		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0))
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			pInstance->Showlog2Dlg(szlog);

			iteState->second[iScreenId*32 + 0] = 1;
			//=======2017/6/2 首次不进行状态切换
			if(!firstSwitch)
				pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/iSwitchGroup,iScreenId,0,nGroup);
			//pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/iSwitchGroup,iScreenId,0,nGroup);
		}	
	}
}
void CClient::IpcPtzOperation(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE","LEFT_UP","LEFT_DOWN","RIGHT_UP","RIGHT_DOWN","STOP"};
	int iPtzOld = 0;
	int iPtzNew = 0;
	CClientManager* pInstance = CClientManager::GetInstance();
	int iPTZOffset = 0;
	for (int iIPCIndex = 1; iIPCIndex < pInstance->m_PLCFormatInfo.nPtzNum; iIPCIndex++)
	{
		iPTZOffset = pInstance->m_PlcIPCAddrMap[iIPCIndex];
		char nCaramOperation = *(pBuf+iPTZOffset);
		char nCaramExt = *(pBuf+iPTZOffset+1);

		std::string strPTZCmd = "STOP";

		/*select and data change
		15 ----  14 ----  13 ---- 12 ---  11---  10 --- 9 lenwiper--- 8	Ready	
		7Select- 6Zoomin---5Zoomout--4panccw--3pancw- 2tiltdown- 1tiltup- 0autozoommode

		---->
		Select
		Wiper-Zoomin--ZoomOut--PanCCW--PanCW--TiltDown--TiltUp--AZoom
		*/		
		bool bStartPtz = false;
		bool bAuto = (1 == ((nCaramOperation)&0x01))?true:false;
		//========================首次选中ZOOM置0===================
		if((m_bZoomAuto[nGroup-1][iIPCIndex][0]!=bAuto)&&bAuto)
		{
			int iGroupIpcId = (nGroup<<8)|iIPCIndex;

			GroupIPCIDMap::iterator iteGroupIPCID = CClientManager::GetInstance()->m_GroupIpcId.find(iGroupIpcId);
			if (iteGroupIPCID == CClientManager::GetInstance()->m_GroupIpcId.end())
			{   
				return;
			} //获取ipc的puid
			std::string strPUID = iteGroupIPCID->second.strPUID;
			ZoomMualReset::iterator iteReset = pInstance->mZoomMualReset.find(nGroup);//查找是否置0
			if(iteReset!=pInstance->mZoomMualReset.end())
			{
				std::vector<std::string>::iterator iterPuid = std::find((iteReset->second).begin(),
					(iteReset->second).end(),strPUID);
				if(iterPuid!=(iteReset->second).end())
				{
					if(pInstance->mPtzCommandToCam)
						pInstance->sendCgiCommadZoom(strPUID,0,nGroup);
					else
						pInstance->IpcZoom2TVWALL(strPUID, 0, nGroup);
				}
			}
			Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(strPUID);
			if(itt!=m_GroupZoomState.end())
				itt->second = -1;
		}
		//=====================================================
		m_bZoomAuto[nGroup-1][iIPCIndex][0] = bAuto;
		//iPtzNew  = /*(nCaramOperation & 0x7f) | ((nCaramExt>>1 & 0x01) << 7)*/(nCaramOperation & 0x7E) | (nCaramExt>>1 & 0x01);
		iPtzNew = nCaramOperation&0x7F;
		iPtzOld = m_stPtzState[nGroup-1][iIPCIndex];

		if (bAuto)/*select AUTO*/
		{
			if(iPtzOld!=iPtzNew)
			{
				int iMaskPtz = 1;//雨刷暂时没有
				for (iMaskPtz= 1; iMaskPtz < 8; iMaskPtz++)
				{
					if ( 1 == ((iPtzNew>>iMaskPtz)& 0x01))/*start*/
					{
						strPTZCmd = strPtzBuffer[iMaskPtz];
						bStartPtz = true;
						break;					
					}
					if ( 1 == ((iPtzOld>>iMaskPtz)& 0x01))
					{
						strPTZCmd = strPtzBuffer[iMaskPtz];
						bStartPtz = false;
						break;
					}
				}
				if(pInstance->mPtzCommandToCam) //send to cam
				{
					bool rect = false;
					if (iMaskPtz != 8)//
					{
						TCHAR szText[1024] = {0};
						_stprintf(szText,"group[%d]-iSwithGroup[%d]-Camera[%d]-bit[%d]=1,bit[0] = %d.",nGroup,iSwitchGroup,iIPCIndex,iMaskPtz,((nCaramOperation)&0x01));
						pInstance->Showlog2Dlg(szText);
						rect = pInstance->PtzOpration2CAM(nGroup, iIPCIndex,bStartPtz,iMaskPtz, iSwitchGroup);	
						//ptz操作日志
						m_stPtzState[nGroup-1][iIPCIndex] = /*((iMaskPtz == 8) && (bStartPtz==false)) ? 0 :*/iPtzNew;
						char szlog[MAX_STR_LEN] = {0};
						if(rect)
							_snprintf(szlog, MAX_STR_LEN-1, "selectedByte-%d; iMaskPtz-%d; groupNum-%d; ipcIndex-%d; cmd-%s iPtzOld:%d iPtzNew:%d success",iPTZOffset, iMaskPtz, nGroup, iIPCIndex, strPTZCmd.c_str(),iPtzOld,iPtzNew);
						else
							_snprintf(szlog, MAX_STR_LEN-1, "selectedByte-%d; iMaskPtz-%d; groupNum-%d; ipcIndex-%d; cmd-%s iPtzOld:%d iPtzNew:%d failure",iPTZOffset, iMaskPtz, nGroup, iIPCIndex, strPTZCmd.c_str(),iPtzOld,iPtzNew);
						pInstance->m_plog->TracePTZInfo(szlog);
						pInstance->SendLog2Dlg(szlog);
					}
				}
				else //send to cctv
				{
					if (iMaskPtz != 8)//
					{
						TCHAR szText[1024] = {0};
						_stprintf(szText,"group[%d]-iSwithGroup[%d]-Camera[%d]-bit[%d]=1,bit[0] = %d.",nGroup,iSwitchGroup,iIPCIndex,iMaskPtz,((nCaramOperation)&0x01));
						pInstance->Showlog2Dlg(szText);
						pInstance->PtzOpration2TVALL(nGroup, iIPCIndex,strPTZCmd,bStartPtz, iSwitchGroup);	
						m_stPtzState[nGroup-1][iIPCIndex] = /*((iMaskPtz == 8) && (bStartPtz==false)) ? 0 :*/iPtzNew; 
						//ptz操作日志
						char szlog[MAX_STR_LEN] = {0};
						_snprintf(szlog, MAX_STR_LEN-1, "selectedByte-%d; iMaskPtz-%d; groupNum-%d; ipcIndex-%d; cmd-%s iPtzOld:%d iPtzNew:%d",iPTZOffset, iMaskPtz, nGroup, iIPCIndex, strPTZCmd.c_str(),iPtzOld,iPtzNew);
						pInstance->m_plog->TracePTZInfo(szlog);
					}
					//pInstance->m_plog->WriteFile(pBuf, szlog);//写读取到的数据
					//}
				}
			}
		}
		//需要写PLC "READY";
	}

}



void CClient::IpcPtzFollowOperation(int nGroup,const unsigned char *pBuf,int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int iPTZOffset = 0;	
	int nFeet = 0;//当前组只有一个feet会被选中
	int nHeight = 0;
	for(ZoomFeet2Addr::iterator ite = pInstance->m_zoomFeet2AddrMap.begin(); ite != pInstance->m_zoomFeet2AddrMap.end(); ite++)
	{
		char cFeet = *(pBuf+ite->second.iAddr);
		if((cFeet>>ite->second.iBit) & 0x01)
		{
			nFeet = ite->first;
			nHeight = (char)pBuf[pInstance->m_ZoomHeightAddr[0].iAddr]<<8 | (char)pBuf[pInstance->m_ZoomHeightAddr[0].iAddr + 1];
			break;
		}
	}
	int iMaxFeet = pInstance->m_iMaxFeet;
	int iMaxHeight = pInstance->m_iMaxHeight;
	if(0 < nFeet && nFeet <= iMaxFeet && -20 < nHeight && nHeight <= iMaxHeight)
	{
		PtzIpc2GroupMap::iterator itegroupMap = pInstance->m_PtzIpc2GroupMap.find(nGroup);
		if(itegroupMap==pInstance->m_PtzIpc2GroupMap.end())
			return;
		std::vector<std::string> puidV = itegroupMap->second;
		for(int i=0;i<puidV.size();i++)
		{
			std::string puid = puidV[i];
			IPCgroupID::iterator itegroupID = pInstance->m_IPCgroupID.find(puid);
			if(itegroupID==pInstance->m_IPCgroupID.end())
				continue;
			int ipcGroupIDget = itegroupID->second;
			int idFindInfo=0;
			((idFindInfo|=nGroup)<<=8)|=ipcGroupIDget;
			GroupIPCIDMap::iterator iteIPCInfoMap = pInstance->m_GroupIpcId.find(idFindInfo); //找相机IP
			if(iteIPCInfoMap == pInstance->m_GroupIpcId.end())
				continue;
			std::string ipcIp = iteIPCInfoMap->second.Ip;

			PtzPuid2Ptz::iterator ptzPresentPMapite = pInstance->m_ptzPuid2FeetHeightPresentPMap.find(puid);
			if(ptzPresentPMapite==pInstance->m_ptzPuid2FeetHeightPresentPMap.end())
				continue;
			int currentSetPtzPresentPValue= ptzPresentPMapite->second[nFeet*iMaxHeight+nHeight];//当前设置的预置点
			std::string postString;
			int currentPresentPosition;
			stPtzValue::iterator iteValue = m_stPtzPresentPosition.find(puid);	
			LoginDevInfo ipcDevice;
			ipcDevice.pchDVRIP = iteIPCInfoMap->second.Ip;
			ipcDevice.nDVRPort = 80;
			ipcDevice.userName = "root";
			ipcDevice.password = "pass";
			Vix_PtzCfgParam ptzParam;
			if(iteValue==m_stPtzPresentPosition.end()) //首次添加
			{
				if(currentSetPtzPresentPValue ==0)
				{
					ptzParam.presetpointName = "home";
					//postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?move=home";
					m_stPtzPresentPosition.insert(std::make_pair(puid,0));
				}
				else
				{
					ostringstream streamPresentP;
					streamPresentP<<currentSetPtzPresentPValue;
					//postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?gotoserverpresetname="+streamPresentP.str();
					//if(pInstance->sendCgiCommad(postString,nGroup))
					//{
					//	continue;
					//}			
					ptzParam.presetpointName = streamPresentP.str();
					m_stPtzPresentPosition.insert(std::make_pair(puid,currentSetPtzPresentPValue));
				}	
				char logInfo[MAX_STR_LEN];
				if(pInstance->PtzCmdCtrl(ipcDevice,PTZ_PointMove,ptzParam))
				{
					m_stPtzPresentPosition.insert(std::make_pair(puid,0));
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call success group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				else
				{
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call failure group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				pInstance->m_plog->TraceInfo(logInfo);
				pInstance->SendLog2Dlg(logInfo);
				continue;
			}
			else
			{	
				currentPresentPosition= iteValue->second;
				if(currentPresentPosition==currentSetPtzPresentPValue)
					continue;
				ostringstream streamPresentP;
				streamPresentP<<currentSetPtzPresentPValue;
				//postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?gotoserverpresetname="+streamPresentP.str();
				ptzParam.presetpointName = streamPresentP.str();
				char logInfo[MAX_STR_LEN];
				if(pInstance->PtzCmdCtrl(ipcDevice,PTZ_PointMove,ptzParam))
				{
					m_stPtzPresentPosition.insert(std::make_pair(puid,0));
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call success group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				else
				{
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call failure group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				pInstance->m_plog->TraceInfo(logInfo);
				pInstance->SendLog2Dlg(logInfo);
				iteValue->second = currentSetPtzPresentPValue;
			}
			//if(pInstance->sendCgiCommad(postString,nGroup))
			//{
			//	continue;
			//}				
			//iteValue->second = currentSetPtzPresentPValue;
			//char szlog[MAX_STR_LEN] = {0};
			//_snprintf(szlog, MAX_STR_LEN-1, "ptz-ipc_%s, ip_%s,ptzFllow,move to present position _%d",puid.c_str(),ipcIp.c_str(),currentSetPtzPresentPValue);
			//pInstance->m_plog->TraceInfo(szlog);
			//pInstance->Showlog2Dlg(szlog);
		}
	}
}
void CClient::IpcPtzOperationModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	char* strPtzBuffer[16]={/*, "ZOOM",*/"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE","WIPER"};
	int iPtzOld[7] = {0};
	int iPtzNew[7] = {0};

	CClientManager* pInstance = CClientManager::GetInstance();
	int iPTZOffset = 0;
	for (int iIPCIndex = 1; iIPCIndex < pInstance->m_ModBusFormatInfo.nPtzNum; iIPCIndex++)
	{
		ModbusIPCAddrMap::iterator it = pInstance->m_ModbusIPCAddrMap.find(iIPCIndex);
		if (it == pInstance->m_ModbusIPCAddrMap.end())
		{
			return;
		}
		iPTZOffset = it->second;

		char nCaramOperation = *(pBuf+iPTZOffset);

		std::string strPTZCmd = "STOP";

		bool bStartPtz = false;
		bool bAuto = (1 == nCaramOperation)?true:false;
		m_bZoomAuto[nGroup-1][iIPCIndex][0] = bAuto;
		for (int i = 0; i < 6; i++)
		{
			iPtzNew[i] = *(pBuf+iPTZOffset+i+1);
			iPtzOld[i] = m_stModbusPtzState[nGroup-1][iIPCIndex][i];
		}
		iPtzNew[6] = *(pBuf+iPTZOffset+9);
		iPtzOld[6] = m_stModbusPtzState[nGroup-1][iIPCIndex][6];

		bool bolTrack = false;
		if (bAuto)/*select AUTO*/
		{
			int i = 0;
			for (i = 0; i < 7; i++)
			{
				if (iPtzNew[i] != iPtzOld[i])
				{
					bolTrack = true;
					if ( 1 == iPtzNew[i])/*start*/
					{
						strPTZCmd = strPtzBuffer[i];
						bStartPtz = true;
						break;					
					}
					if ( 1 == iPtzOld[i])
					{
						strPTZCmd = strPtzBuffer[i];
						bStartPtz = false;
						break;
					}

				}
			}
			if (i != 7)//
			{
				pInstance->PtzOpration2TVALL(nGroup, iIPCIndex,strPTZCmd,bStartPtz, iSwitchGroup);
			}
			for (int ii = 0; ii < 7; ii++)
			{
				m_stModbusPtzState[nGroup-1][iIPCIndex][ii] = /*((iMaskPtz == 8) && (bStartPtz==false)) ? 0 :*/iPtzNew[ii]; 
			}
			if (bolTrack)
			{
				//ptz操作日志
				char szlog[MAX_STR_LEN] = {0};
				_snprintf(szlog, MAX_STR_LEN-1, "selectedByte-%d; iMaskPtz-%d; groupNum-%d; ipcIndex-%d; cmd-%s",iPTZOffset, i, nGroup, iIPCIndex, strPTZCmd.c_str());
				pInstance->m_plog->TracePTZInfo(szlog);
			}
		}
		//需要写PLC "READY";
	}
}

void CClient::IpcZoomOperation(int nGroup ,const unsigned char *pBuf, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(ZoomAutoManual(nGroup, pBuf))//判断对应的zoom摄像机是auto_manual, manual下该工能失效
	{
		int nFeet = 0;//当前组只有一个feet会被选中
		int nHeigthGroups = pInstance->m_ZoomHeightAddr.size();
		int *pHeightArray = new int [nHeigthGroups];
		shared_ptr<int>HeightPtr = shared_ptr<int>(pHeightArray);
		for(ZoomFeet2Addr::iterator ite = pInstance->m_zoomFeet2AddrMap.begin(); ite != pInstance->m_zoomFeet2AddrMap.end(); ite++)
		{
			char cFeet = *(pBuf+ite->second.iAddr);
			if((cFeet>>ite->second.iBit) & 0x01)
			{
				nFeet = ite->first;
				for (int i = 0; i < nHeigthGroups;i ++)
				{
					pHeightArray[i] = (char)pBuf[pInstance->m_ZoomHeightAddr[i].iAddr]<<8 | (char)pBuf[pInstance->m_ZoomHeightAddr[i].iAddr + 1];
					char szlog[MAX_STR_LEN] = {0};
					//_snprintf(szlog, MAX_STR_LEN-1, "readData:group:%d; height:%d; feet:%d",nGroup, pHeightArray[i], nFeet);
					//pInstance->m_plog->TraceZOOMInfo(szlog);
					//pInstance->Showlog2Dlg(szlog);
				}
				break;
			}
		}
		//======判断是否为双吊具模式 2017/4/22
		int DMode=0;
		if(pInstance->mDoubleSpreaderAddr.iAddress!=0)
		{
			DMode|= *(pBuf+pInstance->mDoubleSpreaderAddr.iAddress);
			DMode>>=pInstance->mDoubleSpreaderAddr.iBit;
			DMode&=0x01;
		}

		ZoomIpc2GroupMap::iterator ite =  pInstance->m_ZoomIpc2GroupMap.find(nGroup);
		if (ite !=  pInstance->m_ZoomIpc2GroupMap.end())
		{
			for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
			{
				std::string sPuid = *it;
				if (AutoZoom(nGroup, sPuid))
				{
					//zoom操作日志
					//char szlog[MAX_STR_LEN] = {0};
					//_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s continue",sPuid.c_str());
					//pInstance->Showlog2Dlg(szlog);
					continue;
				}
				for (int i = 0; i < nHeigthGroups;i ++)
				{
					int iValue = GetValueFromZoomMap(sPuid, nFeet,i, pHeightArray[i],DMode);//ivaue为-1,该zoom_ipc没有配置feet_height
					//====motify 2017/4/22 For版本兼容 ，双吊具下找不到时，在原模式下再查找一次？
					if(DMode==1&&iValue==-1)
						iValue = GetValueFromZoomMap(sPuid, nFeet,i, pHeightArray[i],0);
					if (iValue != -1)
					{
						Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
						if(pInstance->mPtzCommandToCam) //send to cam  
						{
							if (itt != m_GroupZoomState.end())
							{
								if (itt->second != iValue)
								{
									itt->second = iValue;
									int rect =pInstance->sendCgiCommadZoom(sPuid,iValue,nGroup);
									//zoom操作日志
									char szlog[MAX_STR_LEN] = {0};
									//_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeightArray[i], nFeet);
									if(rect)
										_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM  ipc_id:%s value:%d group:%d; height:%d; feet:%d success",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
									else
										_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM  ipc_id:%s value:%d group:%d; height:%d; feet:%d failure",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
									pInstance->m_plog->TraceZOOMInfo(szlog);
									pInstance->Showlog2Dlg(szlog);
								}
							}
							else
							{
								m_GroupZoomState.insert(std::make_pair(sPuid,iValue));
								//zoom操作日志
								int rect = pInstance->sendCgiCommadZoom(sPuid,iValue,nGroup);
								char szlog[MAX_STR_LEN] = {0};
								if(rect)
									_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
								else
									_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d failure\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
								pInstance->m_plog->TraceZOOMInfo(szlog);
								pInstance->Showlog2Dlg(szlog);
							}
						}
						else  // send to tvwall
						{
							if (itt != m_GroupZoomState.end())
							{
								if (itt->second != iValue)
								{
									itt->second = iValue;
									pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
									//zoom操作日志
									char szlog[MAX_STR_LEN] = {0};
									//	_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeightArray[i], nFeet);
									_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
									pInstance->m_plog->TraceZOOMInfo(szlog);
									pInstance->Showlog2Dlg(szlog);
								}
							}
							else
							{
								m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
								pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
								//zoom操作日志
								char szlog[MAX_STR_LEN] = {0};
								//_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeightArray[i], nFeet);
								_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeightArray[i], nFeet);
								pInstance->m_plog->TraceZOOMInfo(szlog);
								pInstance->Showlog2Dlg(szlog);
							}
						}
					}
				}
			}
		}
	}
}

void CClient::SwitchScreenHeight(int nGroup,const unsigned char *pBuf, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int HoistHeight = (char)pBuf[pInstance->m_ZoomHeightAddr[0].iAddr]<<8 | 
		(char)pBuf[pInstance->m_ZoomHeightAddr[0].iAddr + 1];

	switchWithHeightFlag::iterator iteFlag = pInstance->m_switchWithHeigt.find(nGroup);
	if(iteFlag!=pInstance->m_switchWithHeigt.end()&&iteFlag->second)
	{
		oldScreenModeStateH statusCurrent;
		ScreenModeUse currentMode;
		for(heightScreenModeInfo::iterator iteMode = pInstance->m_heightScrrenModeInfo.begin();
			iteMode!=pInstance->m_heightScrrenModeInfo.end();iteMode++)
		{
			if(iteMode->group!=nGroup)
				continue;
			// 高度值在切屏范围内，生成新的切屏状态
			if(iteMode->minHeight<=HoistHeight && HoistHeight<iteMode->maxHeight)
			{// 两端闭区间改为左闭右开区间，不然会在从高度在iteMode->maxHeight之内变化时，会导致不切屏，因为区间有重叠
				// xionggao.lee @2017.01.16
				currentMode.mode = iteMode->mode;
				currentMode.screenID = iteMode->screenID;
				statusCurrent.push_back(currentMode);
			}
		}
		oldScreenModeMap::iterator iteMap = pInstance->m_oldScreenModeMap.find(nGroup);
		if(iteMap != pInstance->m_oldScreenModeMap.end()) 
		{//存在已有状态，进行对比
			for(oldScreenModeStateH::iterator iteStatus = statusCurrent.begin();
				iteStatus!=statusCurrent.end();iteStatus++)
			{
				bool exitFlag = false;
				for(oldScreenModeStateH::iterator iteStatusOld = iteMap->second.begin();
					iteStatusOld!=iteMap->second.end();iteStatusOld++)
				{
					if(iteStatus->mode == iteStatusOld->mode && 
						iteStatus->screenID == iteStatusOld->screenID)
						exitFlag = true;
				}
				if(!exitFlag)
				{
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "switch with height ,iSwitchGroup-%d , group-%d, screen_id-%d, mode-%d",
						iSwitchGroup, nGroup, iteStatus->screenID,iteStatus->mode);
					pInstance->m_plog->TraceSWITCHInfo(szlog);
					pInstance->Showlog2Dlg(szlog);
					pInstance->ScreenSwitch2TVWALL(iSwitchGroup,iteStatus->screenID,iteStatus->mode,nGroup);
				}
			}
			iteMap->second=statusCurrent;
		}
		else//不存在状态，切屏、保存
		{
			pInstance->m_oldScreenModeMap.insert(std::make_pair(nGroup,statusCurrent));
			for(oldScreenModeStateH::iterator switchIte=statusCurrent.begin();switchIte!=statusCurrent.end();
				switchIte++)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "switch with height ,iSwitchGroup-%d , group-%d, screen_id-%d, mode-%d",
					iSwitchGroup, nGroup, switchIte->screenID,switchIte->mode);
				pInstance->m_plog->TraceSWITCHInfo(szlog);
				pInstance->Showlog2Dlg(szlog);
				pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/
					iSwitchGroup,switchIte->screenID,switchIte->mode,nGroup);
			}
		}
	}
}
bool CClient::AutoZoom(int nGroup, string strPUID)
{
	bool bBol = false;
	int iIpcNum = 0;
	CClientManager* pInstance = CClientManager::GetInstance();
	if (pInstance->m_bBolModbus)
		iIpcNum = pInstance->m_ModBusFormatInfo.nPtzNum;
	else
		iIpcNum = pInstance->m_PLCFormatInfo.nPtzNum;

	for (int nIPCIndex = 1; nIPCIndex < iIpcNum; nIPCIndex++)
	{
		int iGroupIpcId = (nGroup<<8)|nIPCIndex;

		GroupIPCIDMap::iterator iteGroupIPCID = pInstance->m_GroupIpcId.find(iGroupIpcId);
		if (iteGroupIPCID == pInstance->m_GroupIpcId.end())
		{   
			continue;
		} 
		//获取ipc的puid
		std::string strID = iteGroupIPCID->second.strPUID;
		if (strID == strPUID)
		{
			if (m_bZoomAuto[nGroup-1][nIPCIndex][0])
			{
				bBol = true;
				break;
			}
			else
			{
				bBol = false;
				break;
			}
		}
	}

	return bBol;
}
// modbus Zoom变换使用第一组高度
void CClient::IpcZoomOperationModbus(int nGroup ,const unsigned char *pBuf, const unsigned short *buf, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(ZoomAutoManual(nGroup, pBuf))//判断对应的zoom摄像机是auto_manual, manual下该工能失效
	{
		int nFeet = 0;//当前组只有一个feet会被选中
		int nHeight = 0;

		for (ModbusZoomFeet2Addr::iterator it = pInstance->m_ModbuszoomFeet2AddrMap.begin(); it != pInstance->m_ModbuszoomFeet2AddrMap.end(); it++)
		{
			char ifee = *(pBuf+it->second);
			if(ifee)
			{
				nFeet = it->first;
				nHeight = buf[pInstance->m_iModbusZoomHeightAddr];
				//char szlog[MAX_STR_LEN] = {0};
				//_snprintf(szlog, MAX_STR_LEN-1, "readData:group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
				//pInstance->m_plog->TraceZOOMInfo(szlog);
				break;
			}
		}

		ZoomIpc2GroupMap::iterator ite =  pInstance->m_ZoomIpc2GroupMap.find(nGroup);
		if (ite !=  pInstance->m_ZoomIpc2GroupMap.end())
		{
			for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
			{
				std::string sPuid = *it;
				if (AutoZoom(nGroup, sPuid))
				{
					continue;
				}
				if(sceneZoom(pBuf,sPuid))
				{
					continue;
				}
				// modbus Zoom变换使用第一组高度
				int iValue = GetValueFromZoomMap(sPuid, nFeet,0, nHeight,0);//ivaue为-1,该zoom_ipc没有配置feet_height
				if (iValue != -1)
				{
					Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
					if (itt != m_GroupZoomState.end())
					{
						if (itt->second != iValue)
						{
							itt->second = iValue;
							pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
							//zoom操作日志
							char szlog[MAX_STR_LEN] = {0};
							_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
							pInstance->m_plog->TraceZOOMInfo(szlog);
							pInstance->Showlog2Dlg(szlog);
						}
					}
					else
					{
						m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
						pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
						//zoom操作日志
						char szlog[MAX_STR_LEN] = {0};
						_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
						pInstance->m_plog->TraceZOOMInfo(szlog);
						pInstance->Showlog2Dlg(szlog);
					}
				}
			}
		}
	}
}
//===========根据场景选择是否自动变焦 2017/1/19
bool CClient::sceneZoom(const unsigned char*pBuf,std::string puid)
{
	if(CClientManager::GetInstance()->m_ZoomChoose==0)
		return false;
	else
	{
		int sceneGet = pBuf[CClientManager::GetInstance()->m_ZoomChooseAddr];
		if (sceneGet==1)//车道
			return false;
		else
		{
			zoomSceneMap::iterator it = CClientManager::GetInstance()->m_zoomCameraC.find(puid);
			if(it == CClientManager::GetInstance()->m_zoomCameraC.end())
				return false;
			else
			{
				int retn = it->second;
				if(retn == 0)
					return false;
				else if (retn ==1)
					return true;
			}
		}
	}
	return false;
}

int CClient::GetValueFromZoomMap(std::string sPuid, int iFeet, int nHeightGroup,int iHeight,int mode)
{
	int nRet = -1;
	CClientManager* pInstance = CClientManager::GetInstance();
	int iMaxFeet = pInstance->m_iMaxFeet;
	int iMaxHeight = pInstance->m_iMaxHeight;
	int iMinHeight = pInstance->m_iMinHeight;
	//===2017/4/22 增加双吊具模式
	map<INT ,ZoomPuid2ZoomPtr>  ZoomFind;
	if(mode ==1)
		ZoomFind = pInstance->m_zoomPuid2FeetHeightMap2;
	else
		ZoomFind = pInstance->m_zoomPuid2FeetHeightMap;

	ZoomPuid2Zoom::iterator itFindCamera = ZoomFind[nHeightGroup]->find(sPuid);
	if(0 < iFeet && iFeet <= iMaxFeet && /*-19 < iHeight &&*/ iHeight <= iMaxHeight)
	{
		if (itFindCamera != ZoomFind[nHeightGroup]->end())
		{
			FeetHeightZoomValueMap::iterator itFindFeet = itFindCamera->second->find(iFeet);
			if (itFindFeet != itFindCamera->second->end())
				nRet = itFindFeet->second->pZoomValueArray[iHeight - iMinHeight];
		}
	}
	else
	{
		if (itFindCamera != ZoomFind[nHeightGroup]->end())
		{
			FeetHeightZoomValueMap::iterator itFindFeet = itFindCamera->second->find(iFeet);
			if (itFindFeet != itFindCamera->second->end())
				nRet = itFindFeet->second->nDefaultZoom;
		}
	}
	return nRet;
}
//true:auto  false:manual
bool CClient::ZoomAutoManual(int nGroup ,const unsigned char *pBuf)
{
	return true;
}

void CClient::CloseAllIpcInGroup(int iGroup, int iMode)//mode 0:隐藏提示框 1:关闭视频，显示提示框 2:只关闭视频
{
	CClientManager* pInstance = CClientManager::GetInstance();
	std::map<std::string, TvWallInfo> mapTvWallInfo;
	for(Group2PcMap::iterator ite = pInstance->m_Group2PcMap.begin();
		ite != pInstance->m_Group2PcMap.end(); ite++)
	{
		if(iGroup == ite->first>>8)
		{
			TvWallInfo Tmp;
			Tmp.iTVSSCREENID = ite->second.iTVSSCREENID;
			Tmp.Ip = ite->second.Ip;
			Tmp.Port = ite->second.Port;
			char szStr[MAX_STR_LEN] = {0};
			_snprintf(szStr, MAX_STR_LEN-1, "%s_%d", ite->second.Ip.c_str(), ite->second.Port);
			mapTvWallInfo.insert(std::make_pair(szStr, Tmp));
		}
	}
	std::string strPromat = pInstance->m_TvWallPromat;
	//给对应组发送close命令
	for(std::map<std::string, TvWallInfo>::iterator it = mapTvWallInfo.begin(); it != mapTvWallInfo.end(); it++)
	{
		std::string sRet;
		sRet.append("{");
		SetValue(sRet, "key_id", pInstance->m_KeyId);
		SetValue(sRet, "command", "COMMON", FALSE);
		SetValue(sRet, "sub_cmd", "CHANGE_MODE", FALSE);
		SetValue(sRet, "mode", iMode, FALSE);
		SetValue(sRet, "msg", strPromat.c_str(), FALSE);
		SetjsonEnd(sRet);
		pInstance->SendInfo2TVWALLServer(sRet, it->second.Ip, it->second.Port);
	}
}

wchar_t* CClient::AnsiToUnicode(const char* buf)
{
	int len = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	if (len == 0) 
		return L"";

	std::vector<wchar_t> unicode(len);
	::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], len);

	return &unicode[0];
}

char* CClient::UnicodeToUtf8(const wchar_t* buf)
{
	int len = ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
	if (len == 0) return "";

	std::vector<char> utf8(len);
	::WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], len, NULL, NULL);

	return &utf8[0];
}

int CClient::textBarDataSend(int nGroup,const unsigned char*pBuff,int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	TextBarMessage messageSend;
	if(textBarSpreaderMessage(nGroup,pBuff,messageSend.mSpreaderMsg))
		return 1;
	if(textBarHeightMessage(nGroup,pBuff,messageSend.mHeightMsg))
		return 2;
	if(textBarLiftModeMessage(nGroup,pBuff,messageSend.mLiftMsg))
		return 3;
	textBarInfoMap::iterator iter = pInstance->mTextBarInfoMap.find(nGroup);
	//pInstance->textBarSendToTVwall(nGroup,messageSend,iSwitchGroup);

	if(iter == pInstance->mTextBarInfoMap.end())
	{
		pInstance->mTextBarInfoMap.insert(std::make_pair(nGroup,messageSend));
		pInstance->textBarSendToTVwall(nGroup,messageSend,iSwitchGroup);
	}
	else 
	{
		int rect =0;
		for(int i=0;i<SPREADER_COUNT;i++)
		{
			if(messageSend.mSpreaderMsg[i].SpeederFeet!=iter->second.mSpreaderMsg[i].SpeederFeet||messageSend.mSpreaderMsg[i].SpreaderLanded!=iter->second.mSpreaderMsg[i].SpreaderLanded||
				messageSend.mSpreaderMsg[i].SpreaderLocked!=iter->second.mSpreaderMsg[i].SpreaderLocked||messageSend.mSpreaderMsg[i].SpreaderUnlocked!=iter->second.mSpreaderMsg[i].SpreaderUnlocked||
				messageSend.mSpreaderMsg[i].TwinMode!=iter->second.mSpreaderMsg[i].TwinMode)
				rect= 1;
		}
		if(int(messageSend.mHeightMsg.HositPosition*10)!=int(iter->second.mHeightMsg.HositPosition*10)||int(messageSend.mHeightMsg.PHoistPosition*10)!=int(iter->second.mHeightMsg.PHoistPosition*10)||
			int(messageSend.mHeightMsg.PTrolleyPosition*10)!=int(iter->second.mHeightMsg.PTrolleyPosition*10)||int(messageSend.mHeightMsg.TargetDistance*10)!=int(iter->second.mHeightMsg.TargetDistance*10)||
			int(messageSend.mHeightMsg.TrolleyPosition*10)!=int(iter->second.mHeightMsg.TrolleyPosition*10))
			rect = 1;
		if(messageSend.mLiftMsg.ScrInDSMD!=iter->second.mLiftMsg.ScrInDSMD||messageSend.mLiftMsg.ScrInPSMD!=iter->second.mLiftMsg.ScrInPSMD||
			messageSend.mLiftMsg.ScrInSSMD!=iter->second.mLiftMsg.ScrInSSMD)
			rect = 1;
		if(rect!=0)
		{
			iter->second = messageSend;
			pInstance->textBarSendToTVwall(nGroup,messageSend,iSwitchGroup);
		}
		//if(messageSend.mSpreaderMsg[0].)
	}
	//CClientManager* pInstance = CClientManager::GetInstance();
	//if(pInstance->mTrolleyHeightAddr.addressReady&&pInstance->mSpreaderAddr.addressReady)
	//{
	//	TextBarMessage mTbarMessage;
	//	mTbarMessage.SpeederFeet=0;
	//	for(ZoomFeet2Addr::iterator ite = pInstance->m_zoomFeet2AddrMap.begin(); ite != pInstance->m_zoomFeet2AddrMap.end(); ite++)
	//	{
	//		char cFeet = *(pBuff+ite->second.iAddr);
	//		if((cFeet>>ite->second.iBit) & 0x01)
	//		{
	//			mTbarMessage.SpeederFeet = ite->first;
	//			break;
	//		}
	//	}
	//	mTbarMessage.HoistPosition=0;
	//	mTbarMessage.HoistPosition = (char)pBuff[pInstance->m_ZoomHeightAddr[0].iAddr]<<8 | (char)pBuff[pInstance->m_ZoomHeightAddr[0].iAddr + 1];

	//	char cLanded = *(pBuff+pInstance->mSpreaderAddr.landedAddress);
	//	if(cLanded>>pInstance->mSpreaderAddr.landeAddressBit & 0x01)
	//	{
	//		mTbarMessage.SpreaderLanded = 1;
	//	}
	//	else
	//	{
	//		mTbarMessage.SpreaderLanded = 0;
	//	}
	//	char cLocked = *(pBuff+pInstance->mSpreaderAddr.lockedAddress);
	//	if(cLocked>>pInstance->mSpreaderAddr.lockedBit & 0x01)
	//	{
	//		mTbarMessage.SpreaderLocked = 1;
	//	}
	//	else
	//	{
	//		mTbarMessage.SpreaderLocked = 0;
	//	}
	//	char cTwinMode = *(pBuff+pInstance->mSpreaderAddr.twinModeAddress);
	//	if(cTwinMode>>pInstance->mSpreaderAddr.twinModeBit & 0x01)
	//	{
	//		mTbarMessage.TwinMode = 1;
	//	}
	//	else
	//	{
	//		mTbarMessage.TwinMode = 0;
	//	}
	//	mTbarMessage.TrolleyPosition =0;
	//	int trolleyPosition = 0;
	//	for(int iOffset = 0;iOffset<pInstance->mTrolleyHeightAddr.length;iOffset++)
	//	{
	//		trolleyPosition <<= 8;
	//		trolleyPosition |= (char)pBuff[pInstance->mTrolleyHeightAddr.heightAddress+iOffset];
	//	}
	//	mTbarMessage.TrolleyPosition = trolleyPosition;
	//	//mTbarMessage.TrolleyPosition = (char)pBuff[pInstance->mTrolleyHeightAddr.heightAddress]<<8 | (char)pBuff[pInstance->mTrolleyHeightAddr+ 1];
	//	pInstance->textBarSendToTVwall(nGroup,mTbarMessage,iSwitchGroup);
	//}
	//else
	//	return 1;
	return 0;
}

int CClient::textBarSpreaderMessage(int nGroup,const unsigned char*pBuf,SpreaderMessage* mMsg)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(!pInstance->mSpreaderAddr->addressReady)//send 0 if address is not complete
	{
		for(int i=0;i<SPREADER_COUNT;i++)
		{
			mMsg[i].SpeederFeet = 0;
			mMsg[i].SpreaderLanded=0;
			mMsg[i].SpreaderLocked=0;
			mMsg[i].SpreaderUnlocked=0;
			mMsg[i].TwinMode=0;
		}
		return 1;
	}
	for(int mSpreader=0;mSpreader<SPREADER_COUNT;mSpreader++)
	{
		mMsg[mSpreader].SpeederFeet =0;
		for(std::map<int,AddressBoolValue>::iterator iter=pInstance->mSpreaderAddr[mSpreader].mfeet.begin();
			iter!=pInstance->mSpreaderAddr[mSpreader].mfeet.end();iter++)
		{
			char cFeet = *(pBuf+iter->second.iAddress);
			if((cFeet>>iter->second.iBit)&0x01)
			{
				mMsg[mSpreader].SpeederFeet = iter->first;
				break;
			}
		}
		char cLanded = *(pBuf+pInstance->mSpreaderAddr[mSpreader].mland.iAddress);
		if((cLanded>>pInstance->mSpreaderAddr[mSpreader].mland.iBit)&0x01)
		{
			mMsg[mSpreader].SpreaderLanded=1;
		}
		else
		{
			mMsg[mSpreader].SpreaderLanded=0;
		}
		char cLocked = *(pBuf+pInstance->mSpreaderAddr[mSpreader].mlock.iAddress);
		if((cLocked>>pInstance->mSpreaderAddr[mSpreader].mlock.iBit) & 0x01)
		{
			mMsg[mSpreader].SpreaderLocked=1;
		}
		else
		{
			mMsg[mSpreader].SpreaderLocked=0;
		}
		char cTwinMode = *(pBuf+pInstance->mSpreaderAddr[mSpreader].mtwinMode.iAddress);
		if((cTwinMode>>pInstance->mSpreaderAddr[mSpreader].mtwinMode.iBit) & 0x01)
		{
			mMsg[mSpreader].TwinMode=1;
		}
		else
		{
			mMsg[mSpreader].TwinMode=0;
		}
		char cUnlocked = *(pBuf+pInstance->mSpreaderAddr[mSpreader].munlocked.iAddress);
		if((cUnlocked>>pInstance->mSpreaderAddr[mSpreader].munlocked.iBit) & 0x01)
		{
			mMsg[mSpreader].SpreaderUnlocked=1;
		}
		else
		{
			mMsg[mSpreader].SpreaderUnlocked=0;
		}
	}
	return 0;
}

int CClient::textBarHeightMessage(int nGroup,const unsigned char*pBuf,HeightMessage& mMsg)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(!pInstance->mHeightAddr.addressReady) //send 0 if address is not complete
	{
		mMsg.HositPosition=0;
		mMsg.PHoistPosition=0;
		mMsg.PTrolleyPosition=0;
		mMsg.TargetDistance=0;
		return 1;
	}
	int dataLength = 4;
	int32 trolleyPosition = 0;
	// trolleyPosition
	for(int iOffset=0;iOffset<dataLength;iOffset++)
	{
		trolleyPosition<<=8;
		trolleyPosition|=(unsigned char)pBuf[pInstance->mHeightAddr.TrolleyPositionAddress+iOffset];
	}
	mMsg.TrolleyPosition = trolleyPosition*0.001;//   /10000  trolleyPosition
	//hoistPosition
	int32 hoistPosition =0;
	for(int iOffset=0;iOffset<dataLength;iOffset++)
	{
		hoistPosition<<=8;
		hoistPosition|=(unsigned char)pBuf[pInstance->mHeightAddr.HoistPositionAddress+iOffset];
	}
	mMsg.HositPosition =hoistPosition*0.001; //  /1000  hoistPosition
	//targetDistance
	int32 targetDistance=0;
	for(int iOffset=0;iOffset<dataLength;iOffset++)
	{
		targetDistance<<=8;
		targetDistance|=(unsigned char)pBuf[pInstance->mHeightAddr.TargetDistanceAddress+iOffset];
	}
	mMsg.TargetDistance = targetDistance*0.01; //  /100 targetDistance
	//phoistPosition
	int32 phoistPosition=0;
	for(int iOffset=0;iOffset<dataLength;iOffset++)
	{
		phoistPosition<<=8;
		phoistPosition|=(unsigned char)pBuf[pInstance->mHeightAddr.PHoistPositionAddress+iOffset];
	}
	mMsg.PHoistPosition = phoistPosition*0.001; //  /1000 phoistPosition
	//ptrolleyPosition
	int32 ptrolleyPosition=0;
	for(int iOffset=0;iOffset<dataLength;iOffset++)
	{
		ptrolleyPosition<<=8;
		ptrolleyPosition|=(unsigned char)pBuf[pInstance->mHeightAddr.PTrolleyPositionAddress+iOffset];
	}
	mMsg.PTrolleyPosition=ptrolleyPosition*0.001;    //  /1000 ptrolleyPosition
	return 0;
}

int CClient::textBarLiftModeMessage(int nGroup,const unsigned char*pBuf,LiftModeMessage& mMsg)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(!pInstance->mLiftModeAddr.addressReady) //send 0 if address is not complete
	{
		mMsg.ScrInDSMD=0;
		mMsg.ScrInPSMD=0;
		mMsg.ScrInSSMD=0;
		return 1;
	}
	char ssmd= *(pBuf+pInstance->mLiftModeAddr.ScrInSSMD.iAddress);
	if((ssmd>>pInstance->mLiftModeAddr.ScrInSSMD.iBit)&0X01)
	{
		mMsg.ScrInSSMD=1;
	}
	else
	{
		mMsg.ScrInSSMD=0;
	}
	char dsmd = *(pBuf+pInstance->mLiftModeAddr.ScrInDSMD.iAddress);
	if((dsmd>>pInstance->mLiftModeAddr.ScrInDSMD.iBit)&0X01)
	{
		mMsg.ScrInDSMD =1;
	}
	else
	{
		mMsg.ScrInDSMD=0;
	}
	char psmd = *(pBuf+pInstance->mLiftModeAddr.ScrInPSMD.iAddress);
	if((psmd>>pInstance->mLiftModeAddr.ScrInPSMD.iBit)&0x01)
	{
		mMsg.ScrInPSMD=1;
	}
	else
	{
		mMsg.ScrInPSMD=0;
	}
	return 0;
}

//2017/5/3 特殊点置位，切换特定屏幕、特定模式
int CClient::FreeModeSwitch(int nGroup,const unsigned char*pBuf,int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	freeModeMap::iterator iterFreeMode = pInstance->mFreeModeMap.find(nGroup);
	int rect =0;
	if(iterFreeMode!=pInstance->mFreeModeMap.end())
	{
		for(int iFree=0;iFree<iterFreeMode->second.size();iFree++)
		{
			unsigned char cBit = *(pBuf+iterFreeMode->second[iFree].modeAddr.iAddress);
			cBit=(cBit>>(iterFreeMode->second[iFree].modeAddr.iBit))&0x01;
			if(cBit)
			{
				rect =1 ;
				//=== 检查屏幕状态进行切换，没有考虑优先级
				std::map<int, unsigned short *>::iterator iteState =  m_MapScreenState.find(nGroup);
				if (iteState == m_MapScreenState.end())
				{
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "not save group_%d old state", nGroup);
					pInstance->m_plog->TraceSWITCHInfo(szlog);
					return 0;
				}
				if (1==iteState->second[iterFreeMode->second[iFree].screenID*32 + iterFreeMode->second[iFree].mode])//不需要切换
					continue;
				else //切换并改变状态
				{
					int iScreenModeMax = pInstance->m_PLCFormatInfo.nModeMax;
					int iBeforeVal=0;
					for(int iScreenMode = 0; iScreenMode <= iScreenModeMax; iScreenMode++)
					{
						if(iScreenMode==iterFreeMode->second[iFree].mode)
						{
							iBeforeVal=iteState->second[iterFreeMode->second[iFree].screenID*32 + iterFreeMode->second[iFree].mode];
							iteState->second[iterFreeMode->second[iFree].screenID*32 + iterFreeMode->second[iFree].mode]=1;
						}
						else
							iteState->second[iterFreeMode->second[iFree].screenID*32 + iScreenMode]=0;
					}
					//切换
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "freen mode group-%d; screen_id-%d; byteNum-%d; bitNum-%d; bitValue-%d-%d",nGroup, 
						iterFreeMode->second[iFree].screenID,iterFreeMode->second[iFree].modeAddr.iAddress,
						iterFreeMode->second[iFree].modeAddr.iBit,iBeforeVal, iteState->second[iterFreeMode->second[iFree].screenID*32 + iterFreeMode->second[iFree].mode]);
					pInstance->m_plog->TraceSWITCHInfo(szlog);
					pInstance->Showlog2Dlg(szlog);
					pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/iSwitchGroup,iterFreeMode->second[iFree].screenID,
						iterFreeMode->second[iFree].mode,nGroup);
				}
			}
		}
	}
	return rect;
}

void CClient::PresetPointCall(int nGroup,const unsigned char*pBuff,int iSwitchGroup/*=0*/)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(pInstance->mPresetpointCallAddr.iAddress==0&&pInstance->mPresetpointCallAddr.iBit==0)
	{
		return;
	}	
	ipcPresetPointMsgMap::iterator iterPresetPointCall = pInstance->mIpcPresetPointMsgMap.find(nGroup);
	if(iterPresetPointCall==pInstance->mIpcPresetPointMsgMap.end())
		return;
	char cCall = *(pBuff+pInstance->mPresetpointCallAddr.iAddress);
	if((cCall>>pInstance->mPresetpointCallAddr.iBit) & 0x01)
	{
		std::map<int,bool>::iterator iter = pInstance->mPresetPointCallResetMap.find(nGroup);
		if(iter!=pInstance->mPresetPointCallResetMap.end()) //首次置1 记录状态，复位使用
		{
			if(iter->second)
				iter->second= false;
		}
		ipcPresetPointMsgVec ipcPresetPointCallOperate;
		ipcPresetPointCallOperate = iterPresetPointCall->second;
		for(int i=0;i<ipcPresetPointCallOperate.size();i++)
		{
			if(ipcPresetPointCallOperate[i].pointStatuse)
			{
				int iGroupIpcId = (nGroup<<8)|ipcPresetPointCallOperate[i].ipcIndex;
				GroupIPCIDMap::iterator iteIPCInfoMap = pInstance->m_GroupIpcId.find(iGroupIpcId); //找相机IP
				if(iteIPCInfoMap==pInstance->m_GroupIpcId.end())
					continue;
				LoginDevInfo ipcDevice;
				ipcDevice.pchDVRIP = iteIPCInfoMap->second.Ip;
				ipcDevice.nDVRPort = 80;
				ipcDevice.userName = "root";
				ipcDevice.password = "pass";
				Vix_PtzCfgParam ptzParam;
				ptzParam.presetpointName = ipcPresetPointCallOperate[i].presetPointName;
				char logInfo[MAX_STR_LEN];
				if(pInstance->PtzCmdCtrl(ipcDevice,PTZ_PointMove,ptzParam))
				{
					iterPresetPointCall->second[i].pointStatuse = false;
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call success group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				else
				{
					if(iterPresetPointCall->second[i].erroTime++>5)
						iterPresetPointCall->second[i].pointStatuse = false;
					else	
						continue;
					sprintf_s(logInfo,sizeof(logInfo)-1,"Preset point call failure group:%d,ip:%s,preset point name:%s",nGroup,ipcDevice.pchDVRIP.c_str(),ptzParam.presetpointName.c_str());
				}
				pInstance->m_plog->TraceInfo(logInfo);
				pInstance->SendLog2Dlg(logInfo);
			}
		}
	}
	else
	{
		std::map<int,bool>::iterator iter = pInstance->mPresetPointCallResetMap.find(nGroup);
		if(iter==pInstance->mPresetPointCallResetMap.end())
			return;
		if(!iter->second)//复位
		{
			ipcPresetPointMsgMap::iterator iterPresetPointCall = pInstance->mIpcPresetPointMsgMap.find(nGroup);
			if(iterPresetPointCall!=pInstance->mIpcPresetPointMsgMap.end())
			{
				for(int i=0;i<iterPresetPointCall->second.size();i++)
				{
					iterPresetPointCall->second[i].pointStatuse = true;
					iterPresetPointCall->second[i].erroTime=0;
				}
			}
			iter->second = true;
		}
	}
}




//2017/6/8 modbus 塞内加尔
void CClient::IpcZoomOperationOnlyModbus(int nGroup ,int height, int feet, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int nFeet = 0;//当前组只有一个feet会被选中
	int nHeight = 0;
	nFeet = feet;
	nHeight =height;
	//char szlog[MAX_STR_LEN] = {0};
	//_snprintf(szlog, MAX_STR_LEN-1, "readData:group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
	//pInstance->SendLog2Dlg(szlog);
	//pInstance->m_plog->TraceZOOMInfo(szlog);

	ZoomIpc2GroupMap::iterator ite =  pInstance->m_ZoomIpc2GroupMap.find(nGroup);
	if (ite !=  pInstance->m_ZoomIpc2GroupMap.end())
	{
		for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
		{
			std::string sPuid = *it;
			// modbus Zoom变换使用第一组高度
			int iValue = GetValueFromZoomMap(sPuid, nFeet,0, nHeight,0);//ivaue为-1,该zoom_ipc没有配置feet_height
			if (iValue != -1)
			{
				Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
				if (itt != m_GroupZoomState.end())
				{
					if (itt->second != iValue)
					{
						itt->second = iValue;
						pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
						//zoom操作日志
						char szlog[MAX_STR_LEN] = {0};
						_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d; zoom value:%d",nGroup, nHeight, nFeet,iValue);
						pInstance->m_plog->TraceZOOMInfo(szlog);
						pInstance->Showlog2Dlg(szlog);
					}
				}
				else
				{
					m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
					pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
					//zoom操作日志
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d; zoom value:%d",nGroup, nHeight, nFeet,iValue);
					pInstance->m_plog->TraceZOOMInfo(szlog);
					pInstance->Showlog2Dlg(szlog);
				}
			}
		}
	}
}

void CClient::ReadOpcDataProcess()
{
	CClientManager* pInstance = CClientManager::GetInstance();
	if(pInstance->mConnectOpc)
	{
		for(OpcValueItemNameMap::iterator iterRos = pInstance->mOpcRosMap.begin();iterRos!=pInstance->mOpcRosMap.end();iterRos++)
		{
			unsigned int iGroup = iterRos->first;
			//unsigned int usSwitchOver = iterRos->first;
			Item* itemRos = opcItemFind(iterRos->second);
			if(itemRos==NULL)
			{
				char rosItemError[MAX_STR_LEN];
				sprintf_s(rosItemError,MAX_STR_LEN-1,"can not find ros set %s",iterRos->second.c_str());
				pInstance->SendLog2Dlg(rosItemError);
				continue;
			}
			//---读要绑定的操作台 如果为直连不进行切换 直接赋值
			unsigned int usSwitchOver=0;
			if(pInstance->opcGroupConnectAlways(iGroup))
				usSwitchOver = iGroup;
			else
				usSwitchOver = pInstance->mOpcGroup->read_item(itemRos);
			//unsigned int usSwitchOver = 1;
			//unsigned int iGroup = pInstance->mOpcGroup->read_item(iterRos);
			if (m_stGroupSwitchState[iGroup]==0&&0 == usSwitchOver )		
				continue;
			if (m_stGroupSwitchState[iGroup] != usSwitchOver)  //对应的组号和切换位不相等，查看是否有变化
			{
				if (0 == usSwitchOver)
				{
					CloseAllIpcInGroup(m_stGroupSwitchState[iGroup], 2);//组号设成0关闭分组 2017/4/13
					m_stGroupSwitchState[iGroup] = usSwitchOver;
					//=======2017/6/2 首次不进行状态切换
					//std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(iGroup);
					//if(iteFirst!=pInstance->mGroupSwitchFirst.end())
					//{
					//	if(iteFirst->second)
					//	{
					//		iteFirst->second = false;
					//		m_stGroupSwitchState[iGroup]= usSwitchOver;
					//	}
					//}
					//continue;
				}
				//=======2017/6/2 首次不进行状态切换
				//std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(iGroup);
				//if(iteFirst!=pInstance->mGroupSwitchFirst.end())
				//{
				//	if(iteFirst->second)
				//	{
				//		m_stGroupSwitchState[iGroup]= usSwitchOver;
				//		continue;
				//	}
				//}
				ClearPriority(iGroup);
				std::map<int, unsigned short *>::iterator ite = m_MapScreenState.find(m_stGroupSwitchState[iGroup]);//初始化一下切屏对方的old值
				if (ite != m_MapScreenState.end())
				{
					memset(ite->second, 0, 32*32);
				}
				else
				{
					unsigned short *num = new  unsigned short[32*32];
					memset(num, 0, 32*32);
					m_MapScreenState.insert(std::make_pair(m_stGroupSwitchState[iGroup], num));
				}

				m_stGroupSwitchState[iGroup] = usSwitchOver;
				std::map<int, unsigned short *>::iterator iteState = m_MapScreenState.find(iGroup);//初始化一下切屏的old值
				if (iteState != m_MapScreenState.end())
				{
					memset(iteState->second, 0, 32*32);
				}
				else
				{
					unsigned short *num = new  unsigned short[32*32];
					memset(num, 0, 32*32);
					m_MapScreenState.insert(std::make_pair(iGroup, num));
				}
			}

			//=====读取 模式
			//step 1.  先找当前ARMG  usSwitchOver
			//step 2.  再找此ARMG 所在的OPC Server分组
			OpcValueItemNameMap::iterator iterArmg = pInstance->mOpcRmgMap.find(usSwitchOver);
			if(iterArmg==pInstance->mOpcRmgMap.end())
			{
				char armgError[MAX_STR_LEN];
				sprintf_s(armgError,MAX_STR_LEN-1,"Can not find armg: %d",usSwitchOver);
				pInstance->SendLog2Dlg(armgError);
				continue;
			}
			//step 3.  找当前的mode
			int mode = 0;
			for(OpcValueItemNameMap::iterator iterMode = pInstance->mOpcModeMap.begin();iterMode != pInstance->mOpcModeMap.end();iterMode++)
			{
				std::string modeStr = iterArmg->second+"."+iterMode->second;
				OpcItemMap::iterator iterModeResult = pInstance->mOpcItemMap.find(modeStr);
				if(iterModeResult==pInstance->mOpcItemMap.end())
					continue;
				if(pInstance->mOpcGroup->read_item(iterModeResult->second))
				{
					mode = iterMode->first;
					break;
				}
			}
			//mode = 1;
			OpcSwitchScreen(iGroup, mode, usSwitchOver);

			//======读取height和feet 调整zoom值
			//--step1:读取feet
			int feet = 0;
			OpcValueItemNameMap currentFeetMap;
			if(pInstance->opcGroupConnectAlways(iGroup))
				currentFeetMap = pInstance->mOpcRccsFeetMap;
			else
				currentFeetMap = pInstance->mOpcFeetMap;
			for(OpcValueItemNameMap::iterator iterFeet = currentFeetMap.begin();iterFeet !=currentFeetMap.end();iterFeet++)
			{
				std::string feetStr = iterArmg->second+"."+iterFeet->second;
				OpcItemMap::iterator iterFeetResult = pInstance->mOpcItemMap.find(feetStr);
				if(iterFeetResult==pInstance->mOpcItemMap.end())
					continue;
				if(pInstance->mOpcGroup->read_item(iterFeetResult->second))
				{
					feet = iterFeet->first;
					break;
				}
			}
			//--step2：读height
			int height =0;
			std::string heightStr;
			if(pInstance->opcGroupConnectAlways(iGroup)) //直连 为 rccs
				heightStr = iterArmg->second+"."+pInstance->mOpcRccsHeight;
			else
			    heightStr = iterArmg->second+"."+pInstance->mOpcHeight;
			OpcItemMap::iterator iterHeight = pInstance->mOpcItemMap.find(heightStr);
			if(iterHeight!=pInstance->mOpcItemMap.end())
			{
				height = pInstance->mOpcGroup->read_item(iterHeight->second);
			}
			//对高度进行缩放 到mm->m
			height = height/100;
			OpcIpcZoomOperation(iGroup,height,feet,usSwitchOver);
			//IpcZoomOperation(iGroup,pBuff, usSwitchOver); //zoom值变
		} //ros
	}//connect 
}



Item* CClient::opcItemFind(std::string name)
{
	OpcItemMap::iterator iterResult = CClientManager::GetInstance()->mOpcItemMap.find(name);
	if(iterResult!=CClientManager::GetInstance()->mOpcItemMap.end())
	{
		return iterResult->second;
	}
	else
	{
		return NULL;
	}
}

void CClient::OpcSwitchScreen(int nGroup, int mode, int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	int iScreenNum = pInstance->m_PLCFormatInfo.nScreenNum;
	int iScreenModeMax = pInstance->m_PLCFormatInfo.nModeMax;
	int iModeValue = 0;

	//=======2017/6/2 首次不进行状态切换
	bool firstSwitch = false;
	//bool firstSwitch;
	//std::map<int,bool>::iterator iteFirst = pInstance->mGroupSwitchFirst.find(nGroup);
	//if(iteFirst!=pInstance->mGroupSwitchFirst.end())
	//{
	//	if(iteFirst->second)
	//	{
	//		iteFirst->second = false;
	//		firstSwitch = true;
	//	}
	//	else
	//	{
	//		firstSwitch = false;
	//	}
	//}
	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
	{
		int iBeforeVal = 0;
		bool bBol = false;
		int iState = 0; //同一个屏幕两种及以上的模式都被置位或者该屏幕没有模式被置位
		int iMode = 0;
		std::map<int, unsigned short *>::iterator iteState =  m_MapScreenState.find(nGroup);
		if (iteState == m_MapScreenState.end())
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "not save group_%d old state", nGroup);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			return;
		}
		for(int iScreenMode = 1; iScreenMode <= iScreenModeMax; iScreenMode++)
		{
			if (iScreenMode==mode)
				iModeValue = 1;
			else
				iModeValue = 0;
			if(iModeValue == 1)
			{
				iState++;
				iMode = iScreenMode;
			}
			if(iModeValue == iteState->second[iScreenId*32 + iScreenMode])//没有变化
			{
				continue;
			}
			bBol = true;
			iBeforeVal = iteState->second[iScreenId*32 + iScreenMode];
			iteState->second[iScreenId*32 + iScreenMode] = iModeValue;
		}
		//char szlog[MAX_STR_LEN] = {0};
		//_snprintf(szlog, MAX_STR_LEN-1, "group-%d;  screen_id-%d; mode-%d",nGroup, iScreenId, iMode);
		//pInstance->Showlog2Dlg(szlog);
		//日志
		if (bBol)/*switch mode*/
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "group-%d; iSwitchGroup-%d； screen_id-%d; mode-%d",nGroup, iSwitchGroup,iScreenId, iMode);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			pInstance->Showlog2Dlg(szlog);
			iteState->second[iScreenId*32 + 0] = 0;
			//=======2017/6/2 首次不进行状态切换
			if(!firstSwitch)
				pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/nGroup,iScreenId,iMode,iSwitchGroup);//屏幕切换上墙 与之前的切换方式usSwitchOver 与 iGroup 相反
		}
		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0 ))
		{
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
			pInstance->m_plog->TraceSWITCHInfo(szlog);
			pInstance->Showlog2Dlg(szlog);
			iteState->second[iScreenId*32 + 0] = 1;
			//=======2017/6/2 首次不进行状态切换
			if(!firstSwitch)
				pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/nGroup,iScreenId,0,iSwitchGroup);//屏幕切换上墙 与之前的切换方式usSwitchOver 与 iGroup 相反
			//pInstance->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/iSwitchGroup,iScreenId,0,nGroup);
		}	
	}
}

void CClient::OpcIpcZoomOperation(int nGroup ,int height, int feet,int iSwitchGroup)
{
	CClientManager* pInstance = CClientManager::GetInstance();
	//if(ZoomAutoManual(nGroup, pBuf))//判断对应的zoom摄像机是auto_manual, manual下该工能失效
	if(1)
	{
		int nFeet = feet;//当前组只有一个feet会被选中
		int pHeight = height;
		ZoomIpc2GroupMap::iterator ite =  pInstance->m_ZoomIpc2GroupMap.find(nGroup);
		if (ite !=  pInstance->m_ZoomIpc2GroupMap.end())
		{
			for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
			{
				std::string sPuid = *it;
				if (AutoZoom(nGroup, sPuid))
				{
					//zoom操作日志
					//char szlog[MAX_STR_LEN] = {0};
					//_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s continue",sPuid.c_str());
					//pInstance->Showlog2Dlg(szlog);
					continue;
				}
				int iValue = GetValueFromZoomMap(sPuid, nFeet,0, pHeight,0);//ivaue为-1,该zoom_ipc没有配置feet_height
				if (iValue != -1)
				{
					Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
					if(pInstance->mPtzCommandToCam) //send to cam  
					{
						if (itt != m_GroupZoomState.end())
						{
							if (itt->second != iValue)
							{
								itt->second = iValue;
								int rect =pInstance->sendCgiCommadZoom(sPuid,iValue,nGroup);
								//zoom操作日志
								char szlog[MAX_STR_LEN] = {0};
								//_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeight, nFeet);
								if(rect)
									_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM  ipc_id:%s value:%d group:%d; height:%d; feet:%d success",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
								else
									_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM  ipc_id:%s value:%d group:%d; height:%d; feet:%d failure",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
								pInstance->m_plog->TraceZOOMInfo(szlog);
								pInstance->Showlog2Dlg(szlog);
							}
						}
						else
						{
							m_GroupZoomState.insert(std::make_pair(sPuid,iValue));
							//zoom操作日志
							int rect = pInstance->sendCgiCommadZoom(sPuid,iValue,nGroup);
							char szlog[MAX_STR_LEN] = {0};
							if(rect)
								_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
							else
								_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d failure\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
							pInstance->m_plog->TraceZOOMInfo(szlog);
							pInstance->Showlog2Dlg(szlog);
						}
					}
					else  // send to tvwall
					{
						if (itt != m_GroupZoomState.end())
						{
							if (itt->second != iValue)
							{
								itt->second = iValue;
								pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
								//zoom操作日志
								char szlog[MAX_STR_LEN] = {0};
								//	_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeight, nFeet);
								_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
								pInstance->m_plog->TraceZOOMInfo(szlog);
								pInstance->Showlog2Dlg(szlog);
							}
						}
						else
						{
							m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
							pInstance->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
							//zoom操作日志
							char szlog[MAX_STR_LEN] = {0};
							//_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, pHeight, nFeet);
							_snprintf(szlog, MAX_STR_LEN-1, " PTZ SETZOOM ipc_id:%s value:%d success\n group:%d; height:%d; feet:%d",sPuid.c_str(),iValue,nGroup, pHeight, nFeet);
							pInstance->m_plog->TraceZOOMInfo(szlog);
							pInstance->Showlog2Dlg(szlog);
						}
					}
				}
			}
		}
	}
}