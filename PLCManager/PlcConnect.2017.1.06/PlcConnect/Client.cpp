//#include "HttpConfig.h"
#include <afx.h>
#include "Client.h"
#include "value.h"
#include "reader.h"
#include "writer.h"
#include "NetUDP.h"

CClientManager * CClientManager::m_pThis = NULL;

CClientManager::CClientManager()
{
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

	CIniFile iniFile(GetCurrentPath() + "\\softset.ini");
	int  nLevel = 3;
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
	memset(m_lHeartBeetTime, 0, 50*sizeof(long));
}

CClientManager::~CClientManager()
{
	if (!m_bBolModbus)
	{
		StopReConnect();

		StopGetPlcData();
	}
	else
	{
		StopGetPlcDataModbus();

		StopReConnectModbus();

		stopKeepAlive2Modbus();
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

	for (std::map<std::string, int*>::iterator ite = m_zoomPuid2FeetHeightMap.begin(); ite != m_zoomPuid2FeetHeightMap.end(); ite++)
	{
		delete []ite->second;
	}
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
	int nSwitch;
	iniFile.ReadInt("PLCORMODBUS", "Switch", nSwitch);
	if(2 == nSwitch)
	{
		m_bBolModbus = TRUE;
		//Modbus
		InitModbusData();
		StartReConnectModbus();//开启定时重连Modbus线程
		StartGetModbusData(); //开启50ms读取modbus数据的线程
		//StartKeepAlive2TvWall();//开启TVWALL心跳线程
		StartKeepAlive2Modbus();//开启MODBUS心跳线程
	}
	else
	{
		m_bBolModbus = FALSE;
		//PLC
		InitData();
		StartGetPlcData(); //开启50ms读取plc_server数据的线程
		StartReConnect();//开启定时重连线程
		//StartKeepAlive2TvWall();//开启TVWALL心跳线程	
	}
	
}
void CClientManager::InitData()
{
	CIniFile iniFile(GetCurrentPath() + "\\config\\General.ini");
////////////////////////////////////////////////////////////////////////////////
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
		 stScreenAddrInfo.iPriority = atoi(vectorPLCScreenAdd[4].c_str())&0xff;
		 int iScreenModeMap = iPlcScreenId<<8|iSCREENMode;/* 高８位，低８位*/
		 
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
	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHight", m_iMaxHeight);
	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxFeet", m_iMaxFeet);


	std::string strHeight;
	iniFile.ReadString("PLCZOOMHEIGHTADDR", "HEIGHT", strHeight);
	std::vector<std::string> HeightAddrVec;
	StringSplit(strHeight, ",", HeightAddrVec);
	m_ZoomHeightAddr.iAddr = atoi(HeightAddrVec[0].c_str());
	m_ZoomHeightAddr.iBit = atoi(HeightAddrVec[1].c_str());

	iniFile.ReadInt("SWITCHGROUPADDR", "SwitchGroup", m_SwitchGroupAddr);


	iniFile.ReadInt("SENDSWITCH", "Value", m_nSendSwitch);
	std::vector<std::string> SendSwitchArry;
	iniFile.ReadSectionString("THIRDADDR", SendSwitchArry);
	
	m_MapThirdAddr.empty();
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
		m_ModBusFormatInfo.nOffSet1 = atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum1 = atoi(vecotr[1].c_str());
	}
	if (sOffSet2 != "")
	{
		std::vector<std::string> vecotr;
		StringSplit(sOffSet2, ",", vecotr);
		m_ModBusFormatInfo.nOffSet2 = atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum2 = atoi(vecotr[1].c_str());
	}
	if (sOffSet3 != "")
	{
		std::vector<std::string> vecotr;
		StringSplit(sOffSet3, ",", vecotr);
		m_ModBusFormatInfo.nOffSet3 = atoi(vecotr[0].c_str());
		m_ModBusFormatInfo.nByteNum3 = atoi(vecotr[1].c_str());
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
		ModbusInfo.addr1 = atoi(strDbData[1].c_str());
		ModbusInfo.addr3 = atoi(strDbData[2].c_str());
		ModbusInfo.addr4 = atoi(strDbData[3].c_str());
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
	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxHight", m_iMaxHeight);
	iniFile.ReadInt("ZOOMFEETHEIGHTMAX", "MaxFeet", m_iMaxFeet);


	iniFile.ReadInt("MODBUSZOOMHEIGHTADDR", "HEIGHT", m_iModbusZoomHeightAddr);

	iniFile.ReadInt("SWITCHGROUPADDR", "SwitchGroup", m_iModbusSwitchGroupAddr);
	
	//初始化分组配置文件
	InitGroupData();
}

//初始化分组信息
void CClientManager::InitGroupData()
{
	int iSwitchGroup = 0;
	CFileFind filefind;
	std::string strFilePath(GetCurrentPath() + _T("\\config\\*.*"));
	CString sFilePath = strFilePath.c_str();

	m_Group2PcMap.empty();
	m_TvWallIp2CutMapInfo.empty(); //ip和port为key，value为该tv_wall各个屏幕的预案
	m_GroupPCScreen.empty();
	m_GroupIpcId.empty();
	m_MapFreeCutScreen.empty();
	//zoom
	m_ZoomIpc2GroupMap.empty();		
	m_zoomPuid2FeetHeightMap.empty();
	m_PuidDefaultZoomMap.empty();

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
				/////////////////////== 2016/12/18 锁头跟随==//////////////////////////////////////////////////////
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
// 						std::vector<std::string> feet_height_Vec;
// 						StringSplit2(feetHeightVec[i], "-", feet_height_Vec);
// 						int iFeet = atoi(feet_height_Vec[0].c_str());
// 						int iLowHeight = atoi(feet_height_Vec[1].c_str());
// 						int iHighHeight = atoi(feet_height_Vec[2].c_str());
// 						int iPtzPresentPValue = atoi(feet_height_Vec[3].c_str());
						// 直接使用sscanf大字符串获取相关值
						// 李雄高 2017.01.04
						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iPtzPresentPValue = 0;
						if (sscanf(feetHeightVec[i].c_str(),"",&iFeet,&iLowHeight,&iHighHeight,&iPtzPresentPValue) == 4)
						for (int j = iLowHeight; j <= iHighHeight; j++)
						{
							numPresentP[iFeet*m_iMaxHeight + j] = iPtzPresentPValue;
						}
					}		
				}
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
				std::vector<std::string> ZoomFeetHeightVector;
				iniFile.ReadSectionString("CAMERAZOOMHEIGHT", ZoomFeetHeightVector);
				for(std::vector<std::string>::iterator it = ZoomFeetHeightVector.begin(); it != ZoomFeetHeightVector.end(); it++)
				{
					std::vector<std::string> feetHeightVec;
					StringSplit(*it, ",", feetHeightVec);
					std::string sPuid = feetHeightVec[0];
					int iDefaultValue = atoi(feetHeightVec[1].c_str());
					m_PuidDefaultZoomMap.insert(std::make_pair(sPuid, iDefaultValue));

					int *num = new  int[(m_iMaxFeet+1)*(m_iMaxHeight+1)];
					memset(num, 0, (m_iMaxFeet+1)*(m_iMaxHeight+1));
					m_zoomPuid2FeetHeightMap.insert(std::make_pair(sPuid, num));

					for (int i = 2; i < feetHeightVec.size(); i++)
					{
						// 直接使用sscanf大字符串获取相关值
						// 李雄高 2017.01.04
						int iFeet = 0,iLowHeight = 0,iHighHeight = 0,iZoomValue = 0;
						if (sscanf(feetHeightVec[i].c_str(),"%d-%d-%d-%d",&iFeet,&iLowHeight,&iHighHeight,&iZoomValue) == 4)
						for (int j = iLowHeight; j <= iHighHeight; j++)
						{
							num[iFeet*m_iMaxHeight + j] = iZoomValue;
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
					/* Group<<8|INDEX ,此为唯一性*/
					int iFreeCutID = iGroup<<8|iIndex;

					CutScreenInfo CutScreen;
					unsigned short iOffSet = atoi(CutGroup[3].c_str()) & 0xff;
					CutScreen.OffSet = iOffSet;
					CutScreen.IPCIndex = 0;
					CutScreen.ScreenID = atoi(CutGroup[1].c_str()) & 0xff;
					CutScreen.monitor_ID = ++monitorID;
					CutScreen.SwitchGroup = iSwitchGroup;
					CutScreen.nGroup = 0;

					nScreenId = atoi(CutGroup[1].c_str()) & 0xff;
					
					m_MapFreeCutScreen.insert(std::make_pair(iFreeCutID, CutScreen));
				}
				//////m_GroupScreenMap//////////////////////
				sFreeCutInfo.CutNum = monitorID;
				sFreeCutInfo.nScreenID = nScreenId;

				m_GroupScreenMap.insert(std::make_pair(iSwitchGroup, sFreeCutInfo));//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
				m_nMaxIndex > monitorID ? m_nMaxIndex : m_nMaxIndex = monitorID;

			}//if (sFileName != "General.ini")
		}//while(bBool)
	}//if (filefind.FindFile(sFilePath, 0))
}

void CClientManager::SetIpcState(const char* pIpcStatelist)
{
	m_lockClient.acquire();
	for( Clientmap::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
	{
		iter->second->WriteIpcState(pIpcStatelist);
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
	sRetVec.append("\"maps\":[");			
				
	for (std::vector<WINDOWS_IPCID>::iterator ite_win = itPCSreenMode->second.winId.begin(); 
	    ite_win != itPCSreenMode->second.winId.end(); ite_win++)
	{
		sRetVec.append("{");
		SetValue(sRetVec, "ipc_id", ite_win->DeviceId.c_str(), FALSE);
		SetValue(sRetVec, "monitor_id", ite_win->winNum + 64*(iTVScreenID-1), FALSE);//edit by jeckean,64 nocheck
		SetValue(sRetVec, "stream_type", ite_win->stream_type, TRUE);	//主副码流
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
				break;
			}
			continue;
		}
	}
}

//=============2016/12/18 锁头跟随===================================//
int CClientManager::sendCgiCommad(const std::string command,int nGroup)
{
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl!=NULL)
	{

		curl_easy_setopt(curl,CURLOPT_URL,command.c_str());
		curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
		curl_easy_setopt(curl,CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
		curl_easy_setopt(curl,CURLOPT_USERNAME,"root");
		curl_easy_setopt(curl,CURLOPT_PASSWORD,"pass");
		curl_easy_setopt(curl,CURLOPT_HEADER,1);
		res = curl_easy_perform(curl);
		if(res !=CURLE_OK)
		{
			char szlog[MAX_STR_LEN]={0};
			_snprintf(szlog,MAX_STR_LEN-1,"group_%d, send cgi command failure! commad_%s ，error_%s",
				nGroup,command.c_str(),curl_easy_strerror(res));
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
			curl_easy_cleanup(curl); 
			curl_global_cleanup();
			return 1;
		}	
	}
	curl_easy_cleanup(curl); 
	curl_global_cleanup();
	return 0;
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
				break;
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

void CClientManager::Showlog2Dlg(const char *pStr, int itype)
{
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
		m_lockClient.acquire();
		DealKeepAlive();
		m_lockClient.release();
		Sleep(5000);
		//MySleep(5000);
	}
}
//处理tv_wall的状态，当tv_wall重启成功时，发送最新的对应切屏和视频信息
void CClientManager::DealKeepAlive()
{
	int iIndex = 1;
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
							CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
							CClientManager::GetInstance()->Showlog2Dlg(szlog);

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
				CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
			}
		}
		iIndex++;
		delete HttpKeepAlive;
    }
}

////当TVWALL断线重连时发送最新的自由切屏信息
void CClientManager::SendOldFreeCutScreeninfo(std::string strIpPort)
{
	std::string strIP = strIpPort.substr(0, strIpPort.find(','));
	std::string sPort = strIpPort.substr(strIpPort.find(",")+1);
	int iPort = atoi(sPort.c_str());
	
	for (CutScreenMap::iterator ite = m_MapFreeCutScreen.begin(); ite != m_MapFreeCutScreen.end(); ite++)
	{
		if (ite->second.strIP == strIP && ite->second.iPort == iPort)
		{
			std::map<int, FreeCutInfo>::iterator iteGroupScreen = m_GroupScreenMap.find(ite->second.nGroup);
			if (iteGroupScreen == CClientManager::GetInstance()->m_GroupScreenMap.end())
			{
				return;
			}
			if (ite->second.monitor_ID <= iteGroupScreen->second.CutNum)	//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
			{
				std::string sRetVec;
				sRetVec.append("{");
				SetValue(sRetVec, "key_id", CClientManager::GetInstance()->m_KeyId);
				SetValue(sRetVec, "command", "MAP_IPC", FALSE);
				sRetVec.append("\"maps\":[");

				sRetVec.append("{");
				SetValue(sRetVec, "ipc_id", ite->second.sIPCPuid.c_str(), FALSE);
				SetValue(sRetVec, "monitor_id", ite->second.monitor_ID + 64*(ite->second.iTVScreenID-1), FALSE);
				SetValue(sRetVec, "stream_type", ite->second.stream_type, TRUE);
				sRetVec.append("},");

				sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
				sRetVec.append("]}");
				CClientManager::GetInstance()->SendInfo2TVWALLServer(sRetVec, strIP, iPort);
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
	if (!CClientManager::GetInstance()->m_bBolModbus)
	{
		int nGroup = CClientManager::GetInstance()->m_GroupDBMap.size();
		for (std::map<int, int>::iterator ite = CClientManager::GetInstance()->m_GroupDBMap.begin();
			ite !=  CClientManager::GetInstance()->m_GroupDBMap.end(); ite++)
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
		for (std::map<int, ModBusDB>::iterator ite = CClientManager::GetInstance()->m_GroupModBusDBMap.begin();
			ite !=  CClientManager::GetInstance()->m_GroupModBusDBMap.end(); ite++)
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
	if (m_bConnect)
	{
		for (std::map<int, ModBusDB>::iterator ite = CClientManager::GetInstance()->m_GroupModBusDBMap.begin();
			ite !=  CClientManager::GetInstance()->m_GroupModBusDBMap.end(); ite++)
		{
			int iIpcByteOffset = ite->second.addr4 - 40000;
			uint16 a = CClientManager::GetInstance()->m_usHeartBeet;
			int res = modbus_write_registers(m_ctx, iIpcByteOffset,1, &a);
		}

		CClientManager::GetInstance()->m_usHeartBeet++;
		if (CClientManager::GetInstance()->m_usHeartBeet > 255)
		{
			CClientManager::GetInstance()->m_usHeartBeet = 0;
		}
	}
	
}
//PLC login
int CClient::Login(/*const char *Ip, int nPort,  int nRack, int nSlot*/)
{
	if (m_bConnect)
	{
		return 1;
	}
	_fds.rfd = openSocket(m_nPort, m_strIp.c_str());
	//errno=0;    
	//int opt=1;
	//int res=setsockopt((SOCKET)(fds.rfd), SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
	//LOG3("setsockopt %s %d\n", strerror(errno),res);
	_fds.wfd=_fds.rfd;
	int useProtocol = daveProtoISOTCP;
    
	if (_fds.rfd>0) 
	{ 
		CClientManager::GetInstance()->m_plog->TraceInfo("openSocket successful");
		_di =daveNewInterface(_fds,"IF1",0, useProtocol, daveSpeed187k);
		daveSetTimeout(_di, 5000000);
		daveInitAdapter(_di);
		int Mpi = 2;
		_dc =daveNewConnection(_di,Mpi,m_nRack, m_nSlot);  // insert your rack and slot here
		char szconnlog[256] = {0};
		_snprintf(szconnlog, 255, "daveNewConnection: %d, Mpi=%d, nrack=%d, slot=%d", _dc, Mpi, m_nRack, m_nSlot);
        CClientManager::GetInstance()->m_plog->TraceInfo(szconnlog);
		if (0==daveConnectPLC(_dc))
		{
			m_bConnect = TRUE;

			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect plc success，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg("connect plc success", CONNECT_PLC_SUC);

			return 1;
		}
		if (GetTickCount() - m_nCurrentTime >= 30000)
		{
			m_nCurrentTime = GetTickCount();
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect plc failed，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg("connect plc failed", CONNECT_PLC_ERR);
		}
	}
	else
	{
		if (GetTickCount() - m_nCurrentTime >= 30000)
		{
			m_nCurrentTime = GetTickCount();
			CClientManager::GetInstance()->Showlog2Dlg("openSocket failed", CONNECT_PLC_ERR);
			CClientManager::GetInstance()->m_plog->TraceInfo("openSocket failed");
		}
	}
	//CClientManager::GetInstance()->Showlog2Dlg(1, "openSocket failed");
    return -1;
}
int CClient::LoginModbus(/*const char *Ip, int nPort,  int nRack, int nSlot*/)
{
	if (m_bConnect)
	{
		return 1;
	}

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

				CClientManager::GetInstance()->m_plog->TraceInfo("modbus new tcp successful");
				CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			}
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
			modbus_close(m_ctx);
			modbus_free(m_ctx); //释放连接
		}
		else
		{
			CClientManager::GetInstance()->m_plog->TraceInfo("modbus new tcp successful");
			m_bConnect = TRUE;
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "connect modbus slave success，ip: %s, port: %d", m_strIp.c_str(), m_nPort);
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
			return 1;
		}

	}
	else
		CClientManager::GetInstance()->m_plog->TraceInfo("modbus new tcp failed");

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
	if (iState != m_stModbusIpcOnlineState[iGroup][iIndex]) //默认状态都不在线0, 2:表示在线
	{
		std::map<int, ModBusDB>::iterator ite = CClientManager::GetInstance()->m_GroupModBusDBMap.find(iGroup);
		if(ite != CClientManager::GetInstance()->m_GroupModBusDBMap.end())
		{
			m_stModbusIpcOnlineState[iGroup][iIndex] = iState;
			int iIpcByteOffset = ite->second.addr4 - 40000 + CClientManager::GetInstance()->m_iModbusIpcStateOffSet+iIndex-1;
			uint16 a = iState == 2 ? 0x0001 : 0x0000;
			int res = modbus_write_registers(m_ctx, iIpcByteOffset,1, &a);

			//std::string str =  res >= 0 ? "success" : "failed";
			//char szlog[MAX_STR_LEN] = {0};
			//_snprintf(szlog, MAX_STR_LEN-1, "write group:%d, ipc:%d, state:%d %s", iGroup, iIndex, iState, str.c_str());
			//CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
		}
	}
}

//DB 100-165表示33个摄像头信息 online or not
void CClient::CheckWriteIpcState(int iGroup, int iIndex, int iState)
{
	if (iState != m_stIpcOnlineState[iGroup][iIndex]) //默认状态都不在线0, 2:表示在线
	{
		std::map<int, int>::iterator ite = CClientManager::GetInstance()->m_GroupDBMap.find(iGroup);
		if (ite != CClientManager::GetInstance()->m_GroupDBMap.end())
		{
			m_stIpcOnlineState[iGroup][iIndex] = iState;
			int iIpcByteOffset = CClientManager::GetInstance()->m_iIpcStateOffSet + (iIndex-1)*2;
			int a = iState == 2 ? 0x0100 : 0x0000;
			int res = daveWriteBytes(_dc, /*daveAnaOut*/daveDB, ite->second, iIpcByteOffset, 2, &a);//(DB501，第100字节始，2个b字节，值为a)

			//std::string str =  res >= 0 ? "success" : "failed";
			//char szlog[MAX_STR_LEN] = {0};
			//_snprintf(szlog, MAX_STR_LEN-1, "write group:%d, ipc:%d, state:%d %s", iGroup, iIndex, iState, str.c_str());
			//CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
		}
	}
}

void CClient::ReadPLCDataProcess() //连接成功的话就定时获取信息
{

	if (m_bConnect)
	{
		//按byte读取
		for (std::map<int, int>::iterator ite = CClientManager::GetInstance()->m_GroupDBMap.begin(); 
		ite != CClientManager::GetInstance()->m_GroupDBMap.end(); 
		ite++)
		{
			unsigned char buffer[256] = {0};
			//char iGroupSwitch[4] = {0};
			int nDB = ite->second;
			
			int nOffset = CClientManager::GetInstance()->m_PLCFormatInfo.nOffSet;
			int nByteNum = CClientManager::GetInstance()->m_PLCFormatInfo.nByteNum;
			int res = daveReadBytes(_dc, daveDB, nDB, nOffset, nByteNum,  buffer);
			if(res >= 0)
			{
				//char szlog[MAX_STR_LEN] = {0};//测试时写的数据
				//sprintf_s(szlog, MAX_STR_LEN, "group-%d;all_data",ite->first);
				//CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
				//CClientManager::GetInstance()->m_plog->WriteFile(buffer, szlog, nByteNum);//写读取到的数据

				int iGroup = ite->first; 
				unsigned char *pBuff = (unsigned char *)buffer;		//整体切屏幕  188bit buffer[188],
																	//根据该值确定显示到那个工作台的对应屏幕
				
				//发送给第三方平台
				if (0 != CClientManager::GetInstance()->m_nSendSwitch)
				{
					SendToThirdDevice(iGroup, pBuff, nDB, nOffset, nByteNum);
					if (1 == CClientManager::GetInstance()->m_nSendSwitch)
					{
						continue;
					}
				}

				unsigned int usSwitchOver = (pBuff[CClientManager::GetInstance()->m_SwitchGroupAddr])<<8 |
				                            (pBuff[CClientManager::GetInstance()->m_SwitchGroupAddr+1]);
				
				// 未绑定任何台号，不作处理 
				// by xionggao.lee @2017.01.02
				if (0 == usSwitchOver )		
					continue;

				if (m_stGroupSwitchState[iGroup] != usSwitchOver)  //对应的组号和切换位不相等，查看是否有变化
				{
					if (0 == usSwitchOver)
					{
					    m_stGroupSwitchState[iGroup] = usSwitchOver;
						continue;
					}
					ClearPriority(iGroup);
					CloseAllIpcInGroup(m_stGroupSwitchState[iGroup], 2);
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
				SwitchScreen(iGroup, pBuff, usSwitchOver);//屏幕切换上墙

				IpcPtzOperation(iGroup,pBuff, usSwitchOver);//ptz相关操作

				IpcZoomOperation(iGroup,pBuff, usSwitchOver); //zoom值变

				//===========add switch screen with height 添加随高度切屏=====================================
				SwitchScreenHeight(iGroup,pBuff,usSwitchOver);//随高度自动切屏
				//==============================================================================================
				
				//====================锁头跟随 2016/12/19=============================//
				IpcPtzFollowOperation(iGroup,pBuff,usSwitchOver);
				//====================================================================//
				/////////////////////自由切屏///////////////////////////////////////////
				FreeCutScreen(iGroup, pBuff, usSwitchOver);	//自由切屏
			}
			else
			{
				m_bConnect = FALSE;
				CClientManager::GetInstance()->Showlog2Dlg("read DB data failed, reConnect!", CONNECT_PLC_REC);
				return;
			}
		}
	}
}
void CClient::ClearPriority(int nGroup)	//清除优先级
{
	int iScreenNum = CClientManager::GetInstance()->m_PLCFormatInfo.nScreenNum;
	
	for(int iScreenId = 1; iScreenId <= iScreenNum; iScreenId++)
	{
	
		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = CClientManager::GetInstance()->m_mapPriority.find(iIPCPriority);
		if (itePriority != CClientManager::GetInstance()->m_mapPriority.end())
		{
			itePriority->second.iAddr = 0;
			itePriority->second.iBit = 0;
			itePriority->second.iMode = 0;
			itePriority->second.iPriority = 0;
		}
		else
		{
			PlcScreenAddrInfo sInfo;
			CClientManager::GetInstance()->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}
	}
}
void CClient::ReadModbusDataProcess() //连接成功的话就定时获取信息
{
	if (m_bConnect)
	{
		for (std::map<int, ModBusDB>::iterator ite = CClientManager::GetInstance()->m_GroupModBusDBMap.begin(); 
			ite != CClientManager::GetInstance()->m_GroupModBusDBMap.end(); 
			ite++)
		{
			CClientManager::GetInstance()->m_KeyId = m_iServerKeyID;
			int nGroup = ite->first;

			int nOffset1 = ite->second.addr1 - 10000;
			 
			int nByteNum1 = CClientManager::GetInstance()->m_ModBusFormatInfo.nByteNum1;

			int nOffset2 = ite->second.addr3 - 30000;
			int nByteNum2 = CClientManager::GetInstance()->m_ModBusFormatInfo.nByteNum2;

			int nOffset3 = ite->second.addr4 - 40000;
			int nByteNum3 = CClientManager::GetInstance()->m_ModBusFormatInfo.nByteNum3;

			unsigned short usSwitchOver;

			uint16 buff[256] = {0};
			
			int res = modbus_read_input_registers(m_ctx, nOffset2, nByteNum2+1, buff);
			if (-1 != res)
			{
				usSwitchOver = buff[CClientManager::GetInstance()->m_iModbusSwitchGroupAddr];

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
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
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
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
				return;
			}	
		}
	}
}
void CClient::SendToThirdDevice(int nGroup, const unsigned char *pBuff, int nDB, int nOffSet, int nByteNum)
{
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
	memcpy(ite->second, pBuff, nByteNum);

	/////////包头/////////////////
	typedef struct sendDATA{
		int   identification;	//协议标识
		short type;				//命令类型
		short DB;				//DB数据块号
		short seek;				//数据偏移量
		short num;				//序号
		int   size;				//数据长度
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

	for (std::map<int, ThirdADDR>::iterator it=CClientManager::GetInstance()->m_MapThirdAddr.begin(); it != CClientManager::GetInstance()->m_MapThirdAddr.end(); it++)
	{
		net.SetSendSocket(it->second.Port, it->second.strIP.c_str(), EUDP_Normal);
		if (-1 == net.SendTo(buffer, len))
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "error: sendto third device fail IP:%s , Port:%d", it->second.strIP, it->second.Port);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
		}
		net.DisConnect();
	}
	delete [] buffer;
}
void CClient::FreeCutScreenToTVwall(int nGroup, unsigned short iIPC, int iMapIndex,  int i, int iSwitchGroup)			//发送自由切屏信息给TVWALL
{
	std::map<int, CutScreenInfo>::iterator ite = CClientManager::GetInstance()->m_MapFreeCutScreen.find(iMapIndex);
	if (ite == CClientManager::GetInstance()->m_MapFreeCutScreen.end())
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
		GroupIPCIDMap::iterator iteGroupIpcId =  CClientManager::GetInstance()->m_GroupIpcId.find(iGroupIpcId);
		if (iteGroupIpcId == CClientManager::GetInstance()->m_GroupIpcId.end())
		{
			if (ite->second.SwitchGroup != iSwitchGroup && 0 != iSwitchGroup)
			{
				int iOldMapIndex = (ite->second.SwitchGroup << 8)| i;		//初始化一下切屏对方的old值
				std::map<int, CutScreenInfo>::iterator it = CClientManager::GetInstance()->m_MapFreeCutScreen.find(iOldMapIndex);
				if (it != CClientManager::GetInstance()->m_MapFreeCutScreen.end())
				{
					it->second.SwitchGroup = 0;
				}
				ite->second.SwitchGroup = iSwitchGroup;
			}
			if (ite->second.IPCIndex != iIPC)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "error: Configuration file not have about IPC-%d information! Group: %d, Offset: %d", iIPC, nGroup, ite->second.OffSet);
				CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
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
		
		Group2PcMap::iterator iteGroup2PC = CClientManager::GetInstance()->m_Group2PcMap.find(iGroupTVWall);
		if (iteGroup2PC == CClientManager::GetInstance()->m_Group2PcMap.end())
		{  
			if (ite->second.SwitchGroup != iSwitchGroup)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "error: Configuration file does not have about this Group: %d information!", iSwitchGroup);
				CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
				ite->second.SwitchGroup = iSwitchGroup;
			}
			bol = TRUE;
		}
		if (bol)
		{
			return;		//iteGroupIpcId == CClientManager::GetInstance()->m_GroupIpcId.end()
		}

		std::string sIPCPuid = iteGroupIpcId->second.strPUID;
		int stream_type = iteGroupIpcId->second.stream_type;	//主副码流

		std::string strIP = iteGroup2PC->second.Ip;

		int iPort = iteGroup2PC->second.Port;
		int iTVScreenID = iteGroup2PC->second.iTVSSCREENID;

		std::map<int, FreeCutInfo>::iterator iteGroupScreen = CClientManager::GetInstance()->m_GroupScreenMap.find(nGroup);
		if (iteGroupScreen == CClientManager::GetInstance()->m_GroupScreenMap.end())
		{
			return;
		}
		if (ite->second.monitor_ID <= iteGroupScreen->second.CutNum)	//考虑到四分屏模式切换到三分屏时，不应该发四个屏的信令
		{	
			std::string sRetVec;
			sRetVec.append("{");
			SetValue(sRetVec, "key_id", CClientManager::GetInstance()->m_KeyId);
			SetValue(sRetVec, "command", "MAP_IPC", FALSE);
			sRetVec.append("\"maps\":[");

			sRetVec.append("{");
			SetValue(sRetVec, "ipc_id", sIPCPuid.c_str(), FALSE);
			SetValue(sRetVec, "monitor_id", ite->second.monitor_ID + 64*(iTVScreenID-1), FALSE);
			SetValue(sRetVec, "stream_type", stream_type, TRUE);
			sRetVec.append("},");

			sRetVec = sRetVec.substr(0, sRetVec.length() - 1);
			sRetVec.append("]}");
			CClientManager::GetInstance()->SendInfo2TVWALLServer(sRetVec, strIP, iPort);
		}
		if (ite->second.SwitchGroup != iSwitchGroup && 0 != iSwitchGroup)
		{
			int iOldMapIndex = (ite->second.SwitchGroup << 8)| i;		//初始化一下切屏对方的old值
			std::map<int, CutScreenInfo>::iterator it = CClientManager::GetInstance()->m_MapFreeCutScreen.find(iOldMapIndex);
			if (it != CClientManager::GetInstance()->m_MapFreeCutScreen.end())
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
void CClient::FreeCutScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup)	//自由切屏功能
{
	for (int i = 1; i <= CClientManager::GetInstance()->m_nMaxIndex; i++)
	{
		int iMapIndex = (nGroup<<8)|i;
		std::map<int, CutScreenInfo>::iterator ite = CClientManager::GetInstance()->m_MapFreeCutScreen.find(iMapIndex);
		if (ite == CClientManager::GetInstance()->m_MapFreeCutScreen.end())
		{
			continue;
		}
		
		//unsigned short iIPC = (*(pBuf+ite->second.OffSet))&0xff;	
		unsigned short iIPC = (pBuf[ite->second.OffSet])<<8|(pBuf[ite->second.OffSet+1]);	

		FreeCutScreenToTVwall(nGroup, iIPC, iMapIndex, i, iSwitchGroup);
	}
}
void CClient::SwitchScreenModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	int iScreenNum = CClientManager::GetInstance()->m_ModBusFormatInfo.nScreenNum;
	int iScreenModeMax = CClientManager::GetInstance()->m_ModBusFormatInfo.nModeMax;
	int iModeValue = 0;
	int iAddr = 0;

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
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			return;
		}

		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = CClientManager::GetInstance()->m_mapPriority.find(iIPCPriority);
		if (itePriority == CClientManager::GetInstance()->m_mapPriority.end())
		{
			CClientManager::GetInstance()->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}

		for(int iScreenMode = 0; iScreenMode <= iScreenModeMax; iScreenMode++)
		{
			int iPCSreenModeAddr =(iScreenId<<8)|iScreenMode;	
			ModbusScreenAddrMap::iterator iteScreenAddr = CClientManager::GetInstance()->m_ModbusScreenAddMap.find(iPCSreenModeAddr);
			if (iteScreenAddr == CClientManager::GetInstance()->m_ModbusScreenAddMap.end())
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

			if (iModeValue && sInfo.iPriority <= iteScreenAddr->second.iPriority)	//筛选出最高优先级做响应
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

		PlcScreenAddrMap::iterator itePRI = CClientManager::GetInstance()->m_mapPriority.find(iIPCPriority);
		if (itePRI->second.iPriority == sInfo.iPriority && sInfo.iPriority != 0)	//如果最高优先级模式改变
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
			CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
			//CClientManager::GetInstance()->m_plog->WriteFile(pBuf, szlog);//写读取到的数据

			iteState->second[iScreenId*32 + 0] = 0;
			CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/iSwitchGroup,iScreenId,sInfo.iMode,nGroup);
		}

		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0))
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
			CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);

			iteState->second[iScreenId*32 + 0] = 1;
			CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/iSwitchGroup,iScreenId,0,nGroup);
		}	
	}
}

void CClient::SwitchScreen(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	int iScreenNum = CClientManager::GetInstance()->m_PLCFormatInfo.nScreenNum;
	int iScreenModeMax = CClientManager::GetInstance()->m_PLCFormatInfo.nModeMax;
    int iAddr = 0;
	int iBit = 0;
	int iModeValue = 0;

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
			CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
			return;
		}
	
		int iIPCPriority = (nGroup << 8)|iScreenId;
		PlcScreenAddrMap::iterator itePriority = CClientManager::GetInstance()->m_mapPriority.find(iIPCPriority);
		if (itePriority == CClientManager::GetInstance()->m_mapPriority.end())
		{
			CClientManager::GetInstance()->m_mapPriority.insert(std::make_pair(iIPCPriority, sInfo));
		}

		for(int iScreenMode = 0; iScreenMode <= iScreenModeMax; iScreenMode++)
		{
			int iPCSreenModeAddr =(iScreenId<<8)|iScreenMode;	
			PlcScreenAddrMap::iterator iteScreenAddr = CClientManager::GetInstance()->m_PlcScreenAddMap.find(iPCSreenModeAddr);
			if (iteScreenAddr == CClientManager::GetInstance()->m_PlcScreenAddMap.end())
			{   
				continue;
			}
			iAddr = iteScreenAddr->second.iAddr;

			iBit = iteScreenAddr->second.iBit;
			char ucBuffValue = pBuf[iAddr];
			iModeValue =  ( ucBuffValue >>  iBit) & 0x1;
	
			if(iModeValue == 1)
			{
				iState++;
				iMode = iScreenMode;
			}
			
			if (iModeValue && sInfo.iPriority <= iteScreenAddr->second.iPriority)	//筛选出最高优先级做响应
			{
				sInfo.iAddr = iteScreenAddr->second.iAddr;
				sInfo.iBit = iteScreenAddr->second.iBit;
				sInfo.iMode = iScreenMode;
				sInfo.iPriority = iteScreenAddr->second.iPriority;
			}

			if(iModeValue == iteState->second[iScreenId*32 + iScreenMode])//没有变化
			{
				continue;
			}
			
			bBol = true;
			iBeforeVal = iteState->second[iScreenId*32 + iScreenMode];
		    iteState->second[iScreenId*32 + iScreenMode] = iModeValue;
		}

		PlcScreenAddrMap::iterator itePRI = CClientManager::GetInstance()->m_mapPriority.find(iIPCPriority);
		if (itePRI->second.iPriority == sInfo.iPriority && sInfo.iPriority != 0)	//如果最高优先级模式改变
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
			_snprintf(szlog, MAX_STR_LEN-1, "group-%d; screen_id-%d; byteNum-%d; bitNum-%d; bitValue-%d-%d; iSwitchGroup- %d ",nGroup, iScreenId, sInfo.iAddr, sInfo.iBit, iBeforeVal, iteState->second[iScreenId*32 + sInfo.iMode],iSwitchGroup);
			CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
			//CClientManager::GetInstance()->m_plog->WriteFile(pBuf, szlog);//写读取到的数据

			iteState->second[iScreenId*32 + 0] = 0;
			CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId,iScreenMode,iSwitchGroup*/
				iSwitchGroup,iScreenId,sInfo.iMode,nGroup);
		}
		
		//相应屏幕的初始状态和和异常切屏数据，都切回到初始状态
		if ( (iState == 0 && iteState->second[iScreenId*32 + 0] == 0))
		{
			char szlog[MAX_STR_LEN] = {0};

			_snprintf(szlog, MAX_STR_LEN-1, "Back to the initial state ,iSwitchGroup-%d , group-%d, screen_id-%d", iSwitchGroup, nGroup, iScreenId);
			CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);

			iteState->second[iScreenId*32 + 0] = 1;
			CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/
				iSwitchGroup,iScreenId,0,nGroup);
		}	
	}
}

void CClient::IpcPtzOperation(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	char* strPtzBuffer[16]={"WIPER"/*, "ZOOM"*/,"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE"};
	int iPtzOld = 0;
	int iPtzNew = 0;

	int iPTZOffset = 0;
	for (int iIPCIndex = 1; iIPCIndex < CClientManager::GetInstance()->m_PLCFormatInfo.nPtzNum; iIPCIndex++)
	{
	    iPTZOffset = CClientManager::GetInstance()->m_PlcIPCAddrMap[iIPCIndex];
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
		m_bZoomAuto[nGroup-1][iIPCIndex][0] = bAuto;
		iPtzNew  = /*(nCaramOperation & 0x7f) | ((nCaramExt>>1 & 0x01) << 7)*/(nCaramOperation & 0x7E) | (nCaramExt>>1 & 0x01);
		iPtzOld = m_stPtzState[nGroup-1][iIPCIndex];
		
		if (bAuto)/*select AUTO*/
		{
		    if ( iPtzOld != iPtzNew)/*operation change, 每次只有一种动作开启*/
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
				if (iMaskPtz != 8)//
				{
				   CClientManager::GetInstance()->PtzOpration2TVALL(nGroup, iIPCIndex,strPTZCmd,bStartPtz, iSwitchGroup);
				}
				m_stPtzState[nGroup-1][iIPCIndex] = /*((iMaskPtz == 8) && (bStartPtz==false)) ? 0 :*/iPtzNew; 
				   //ptz操作日志
				   char szlog[MAX_STR_LEN] = {0};
				   _snprintf(szlog, MAX_STR_LEN-1, "selectedByte-%d; iMaskPtz-%d; groupNum-%d; ipcIndex-%d; cmd-%s",iPTZOffset, iMaskPtz, nGroup, iIPCIndex, strPTZCmd.c_str());
				   CClientManager::GetInstance()->m_plog->TracePTZInfo(szlog);
				   //CClientManager::GetInstance()->m_plog->WriteFile(pBuf, szlog);//写读取到的数据
				//}
			}
		}
		//需要写PLC "READY";
	}
}



void CClient::IpcPtzFollowOperation(int nGroup,const unsigned char *pBuf,int iSwitchGroup)
{
	int iPTZOffset = 0;	
	int nFeet = 0;//当前组只有一个feet会被选中
	int nHeight = 0;
	for(ZoomFeet2Addr::iterator ite = CClientManager::GetInstance()->m_zoomFeet2AddrMap.begin(); ite != CClientManager::GetInstance()->m_zoomFeet2AddrMap.end(); ite++)
	{
		char cFeet = *(pBuf+ite->second.iAddr);
		if((cFeet>>ite->second.iBit) & 0x01)
		{
			nFeet = ite->first;
			nHeight = (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr]<<8 | (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr + 1];
			break;
		}
	}
	int iMaxFeet = CClientManager::GetInstance()->m_iMaxFeet;
	int iMaxHeight = CClientManager::GetInstance()->m_iMaxHeight;
	if(0 < nFeet && nFeet <= iMaxFeet && 0 < nHeight && nHeight <= iMaxHeight)
	{
		PtzIpc2GroupMap::iterator itegroupMap = CClientManager::GetInstance()->m_PtzIpc2GroupMap.find(nGroup);
		if(itegroupMap==CClientManager::GetInstance()->m_PtzIpc2GroupMap.end())
			return;
		std::vector<std::string> puidV = itegroupMap->second;
		for(int i=0;i<puidV.size();i++)
		{
			std::string puid = puidV[i];
			IPCgroupID::iterator itegroupID = CClientManager::GetInstance()->m_IPCgroupID.find(puid);
			if(itegroupID==CClientManager::GetInstance()->m_IPCgroupID.end())
				continue;
			int ipcGroupIDget = itegroupID->second;
			int idFindInfo=0;
			((idFindInfo|=nGroup)<<=8)|=ipcGroupIDget;
			GroupIPCIDMap::iterator iteIPCInfoMap = CClientManager::GetInstance()->m_GroupIpcId.find(idFindInfo); //找相机IP
			if(iteIPCInfoMap == CClientManager::GetInstance()->m_GroupIpcId.end())
				continue;
			std::string ipcIp = iteIPCInfoMap->second.Ip;

			PtzPuid2Ptz::iterator ptzPresentPMapite = CClientManager::GetInstance()->m_ptzPuid2FeetHeightPresentPMap.find(puid);
			if(ptzPresentPMapite==CClientManager::GetInstance()->m_ptzPuid2FeetHeightPresentPMap.end())
				continue;
			int currentSetPtzPresentPValue= ptzPresentPMapite->second[nFeet*iMaxHeight+nHeight];//当前设置的预置点
			std::string postString;
			int currentPresentPosition;
			stPtzValue::iterator iteValue = m_stPtzPresentPosition.find(puid);
			if(iteValue==m_stPtzPresentPosition.end())
			{
				if(currentSetPtzPresentPValue ==0)
				{
					postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?move=home";
					m_stPtzPresentPosition.insert(std::make_pair(puid,0));
					continue;
				}
				else
				{
					ostringstream streamPresentP;
					streamPresentP<<currentSetPtzPresentPValue;
					postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?gotoserverpresetname="+streamPresentP.str();
					if(CClientManager::GetInstance()->sendCgiCommad(postString,nGroup))
					{
						continue;
					}			
					m_stPtzPresentPosition.insert(std::make_pair(puid,currentSetPtzPresentPValue));
					continue;
				}	
			}
			else
			{
				currentPresentPosition= iteValue->second;
				if(currentPresentPosition==currentSetPtzPresentPValue)
					continue;
				ostringstream streamPresentP;
				streamPresentP<<currentSetPtzPresentPValue;
				postString = "http://"+ipcIp+"/axis-cgi/com/ptz.cgi?gotoserverpresetname="+streamPresentP.str();
			}
			if(CClientManager::GetInstance()->sendCgiCommad(postString,nGroup))
			{
				continue;
			}				
			iteValue->second = currentSetPtzPresentPValue;
			char szlog[MAX_STR_LEN] = {0};
			_snprintf(szlog, MAX_STR_LEN-1, "ptz-ipc_%s, ip_%s,ptzFllow,move to present position _%d",puid.c_str(),ipcIp.c_str(),currentSetPtzPresentPValue);
			CClientManager::GetInstance()->m_plog->TraceInfo(szlog);
			CClientManager::GetInstance()->Showlog2Dlg(szlog);
		}
	}
}
void CClient::IpcPtzOperationModbus(int nGroup, const unsigned char *pBuf, int iSwitchGroup)
{
	char* strPtzBuffer[16]={/*, "ZOOM",*/"UP","DOWN","LEFT","RIGHT", "ZOOM_ADD", "ZOOM_REDUCE","WIPER"};
	int iPtzOld[7] = {0};
	int iPtzNew[7] = {0};

	int iPTZOffset = 0;
	for (int iIPCIndex = 1; iIPCIndex < CClientManager::GetInstance()->m_ModBusFormatInfo.nPtzNum; iIPCIndex++)
	{
		ModbusIPCAddrMap::iterator it = CClientManager::GetInstance()->m_ModbusIPCAddrMap.find(iIPCIndex);
		if (it == CClientManager::GetInstance()->m_ModbusIPCAddrMap.end())
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
				CClientManager::GetInstance()->PtzOpration2TVALL(nGroup, iIPCIndex,strPTZCmd,bStartPtz, iSwitchGroup);
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
				CClientManager::GetInstance()->m_plog->TracePTZInfo(szlog);
			}
		}
		//需要写PLC "READY";
	}
}

void CClient::IpcZoomOperation(int nGroup ,const unsigned char *pBuf, int iSwitchGroup)
{
    if(ZoomAutoManual(nGroup, pBuf))//判断对应的zoom摄像机是auto_manual, manual下该工能失效
    {
		int nFeet = 0;//当前组只有一个feet会被选中
		int nHeight = 0;
		for(ZoomFeet2Addr::iterator ite = CClientManager::GetInstance()->m_zoomFeet2AddrMap.begin(); ite != CClientManager::GetInstance()->m_zoomFeet2AddrMap.end(); ite++)
		{
			char cFeet = *(pBuf+ite->second.iAddr);
			if((cFeet>>ite->second.iBit) & 0x01)
			{
				nFeet = ite->first;
				nHeight = (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr]<<8 | (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr + 1];
				//char szlog[MAX_STR_LEN] = {0};
				//_snprintf(szlog, MAX_STR_LEN-1, "readData:group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
				//CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
				break;
			}
		}
		ZoomIpc2GroupMap::iterator ite =  CClientManager::GetInstance()->m_ZoomIpc2GroupMap.find(nGroup);
		if (ite !=  CClientManager::GetInstance()->m_ZoomIpc2GroupMap.end())
		{
			for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
			{
				std::string sPuid = *it;
				if (AutoZoom(nGroup, sPuid))
				{
					continue;
				}
				int iValue = GetValueFromZoomMap(sPuid, nFeet, nHeight);//ivaue为-1,该zoom_ipc没有配置feet_height
				if (iValue != -1)
				{
					Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
					if (itt != m_GroupZoomState.end())
					{
						if (itt->second != iValue)
						{
							itt->second = iValue;
							CClientManager::GetInstance()->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
							//zoom操作日志
							char szlog[MAX_STR_LEN] = {0};
							_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
							CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
							CClientManager::GetInstance()->Showlog2Dlg(szlog);
						}
					}
					else
					{
						m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
						CClientManager::GetInstance()->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
						//zoom操作日志
						char szlog[MAX_STR_LEN] = {0};
						_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
						CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
						CClientManager::GetInstance()->Showlog2Dlg(szlog);
					}
				}
			}
		}
    }
}
void CClient::SwitchScreenHeight(int nGroup,const unsigned char *pBuf, int iSwitchGroup)
{
	int switchHeight =0;
	switchHeight = (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr]<<8 | (char)pBuf[CClientManager::GetInstance()->m_ZoomHeightAddr.iAddr + 1];
	switchWithHeightFlag::iterator iteFlag = CClientManager::GetInstance()->m_switchWithHeigt.find(nGroup);
	if(iteFlag!=CClientManager::GetInstance()->m_switchWithHeigt.end()&&iteFlag->second)
	{
		oldScreenModeStateH statusCurrent;
		ScreenModeUse currentMode;
		for(heightScreenModeInfo::iterator iteMode = CClientManager::GetInstance()->m_heightScrrenModeInfo.begin();
			iteMode!=CClientManager::GetInstance()->m_heightScrrenModeInfo.end();iteMode++)
		{
			if(iteMode->group!=nGroup)
				continue;
			if(iteMode->minHeight<=switchHeight&&switchHeight<=iteMode->maxHeight)
			{			
				currentMode.mode = iteMode->mode;
				currentMode.screenID = iteMode->screenID;
				statusCurrent.push_back(currentMode);
			}
		}
		oldScreenModeMap::iterator iteMap = CClientManager::GetInstance()->m_oldScreenModeMap.find(nGroup);
		if(iteMap != CClientManager::GetInstance()->m_oldScreenModeMap.end()) //存在已有状态，进行对比
		{	

			for(oldScreenModeStateH::iterator iteStatus = statusCurrent.begin();iteStatus!=statusCurrent.end();
				iteStatus++)
			{
				bool exitFlag = false;	
				for(oldScreenModeStateH::iterator iteStatusOld = iteMap->second.begin();
					iteStatusOld!=iteMap->second.end();iteStatusOld++)
				{
					if(iteStatus->mode==iteStatusOld->mode&&iteStatus->screenID==iteStatusOld->screenID)
						exitFlag = true;
				}
				if(!exitFlag)
				{
					char szlog[MAX_STR_LEN] = {0};
					_snprintf(szlog, MAX_STR_LEN-1, "switch with height ,iSwitchGroup-%d , group-%d, screen_id-%d, mode-%d",
						iSwitchGroup, nGroup, iteStatus->screenID,iteStatus->mode);
					CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
					CClientManager::GetInstance()->Showlog2Dlg(szlog);
					CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/
						iSwitchGroup,iteStatus->screenID,iteStatus->mode,nGroup);
				}
			}
			iteMap->second=statusCurrent;
		}
		else//不存在状态，切屏、保存
		{
			CClientManager::GetInstance()->m_oldScreenModeMap.insert(std::make_pair(nGroup,statusCurrent));
			for(oldScreenModeStateH::iterator switchIte=statusCurrent.begin();switchIte!=statusCurrent.end();
				switchIte++)
			{
				char szlog[MAX_STR_LEN] = {0};

				_snprintf(szlog, MAX_STR_LEN-1, "switch with height ,iSwitchGroup-%d , group-%d, screen_id-%d, mode-%d",
					iSwitchGroup, nGroup, switchIte->screenID,switchIte->mode);
				CClientManager::GetInstance()->m_plog->TraceSWITCHInfo(szlog);
				CClientManager::GetInstance()->Showlog2Dlg(szlog);
				CClientManager::GetInstance()->ScreenSwitch2TVWALL(/*nGroup,iScreenId, 0,iSwitchGroup*/
					iSwitchGroup,switchIte->screenID,switchIte->mode,nGroup);
			}

		}
	}
}
bool CClient::AutoZoom(int nGroup, string strPUID)
{
	bool bBol = false;
	int iIpcNum = 0;
	if (CClientManager::GetInstance()->m_bBolModbus)
		iIpcNum = CClientManager::GetInstance()->m_ModBusFormatInfo.nPtzNum;
	else
		iIpcNum = CClientManager::GetInstance()->m_PLCFormatInfo.nPtzNum;

	for (int nIPCIndex = 1; nIPCIndex < iIpcNum; nIPCIndex++)
	{
		int iGroupIpcId = (nGroup<<8)|nIPCIndex;

		GroupIPCIDMap::iterator iteGroupIPCID = CClientManager::GetInstance()->m_GroupIpcId.find(iGroupIpcId);
		if (iteGroupIPCID == CClientManager::GetInstance()->m_GroupIpcId.end())
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
void CClient::IpcZoomOperationModbus(int nGroup ,const unsigned char *pBuf, const unsigned short *buf, int iSwitchGroup)
{
	if(ZoomAutoManual(nGroup, pBuf))//判断对应的zoom摄像机是auto_manual, manual下该工能失效
	{
		int nFeet = 0;//当前组只有一个feet会被选中
		int nHeight = 0;
	
		for (ModbusZoomFeet2Addr::iterator it = CClientManager::GetInstance()->m_ModbuszoomFeet2AddrMap.begin(); it != CClientManager::GetInstance()->m_ModbuszoomFeet2AddrMap.end(); it++)
		{
			char ifee = *(pBuf+it->second);
			if(ifee)
			{
				nFeet = it->first;
				nHeight = buf[CClientManager::GetInstance()->m_iModbusZoomHeightAddr];
				//char szlog[MAX_STR_LEN] = {0};
				//_snprintf(szlog, MAX_STR_LEN-1, "readData:group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
				//CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
				break;
			}
		}

		ZoomIpc2GroupMap::iterator ite =  CClientManager::GetInstance()->m_ZoomIpc2GroupMap.find(nGroup);
		if (ite !=  CClientManager::GetInstance()->m_ZoomIpc2GroupMap.end())
		{
			for (ZoomIpcVec::iterator it = ite->second.begin(); it != ite->second.end(); it++)
			{
				std::string sPuid = *it;
				if (AutoZoom(nGroup, sPuid))
				{
					continue;
				}
				int iValue = GetValueFromZoomMap(sPuid, nFeet, nHeight);//ivaue为-1,该zoom_ipc没有配置feet_height
				if (iValue != -1)
				{
					Puid2ZoomVaule::iterator itt = m_GroupZoomState.find(sPuid);
					if (itt != m_GroupZoomState.end())
					{
						if (itt->second != iValue)
						{
							itt->second = iValue;
							CClientManager::GetInstance()->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
							//zoom操作日志
							char szlog[MAX_STR_LEN] = {0};
							_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
							CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
							CClientManager::GetInstance()->Showlog2Dlg(szlog);
						}
					}
					else
					{
						m_GroupZoomState.insert(std::make_pair(sPuid, iValue));
						CClientManager::GetInstance()->IpcZoom2TVWALL(sPuid, iValue, iSwitchGroup);
						//zoom操作日志
						char szlog[MAX_STR_LEN] = {0};
						_snprintf(szlog, MAX_STR_LEN-1, "group:%d; height:%d; feet:%d",nGroup, nHeight, nFeet);
						CClientManager::GetInstance()->m_plog->TraceZOOMInfo(szlog);
						CClientManager::GetInstance()->Showlog2Dlg(szlog);
					}
				}
			}
		}
	}
}

int CClient::GetValueFromZoomMap(std::string sPuid, int iFeet, int iHeight)
{
	int nRet = -1;
	int iMaxFeet = CClientManager::GetInstance()->m_iMaxFeet;
	int iMaxHeight = CClientManager::GetInstance()->m_iMaxHeight;
	if(0 < iFeet && iFeet <= iMaxFeet && /*-19 < iHeight &&*/ iHeight <= iMaxHeight)
	{
		ZoomPuid2Zoom::iterator ite = CClientManager::GetInstance()->m_zoomPuid2FeetHeightMap.find(sPuid);
		if (ite != CClientManager::GetInstance()->m_zoomPuid2FeetHeightMap.end())
		{
			nRet = ite->second[iFeet*iMaxHeight + iHeight];
		}
	}
	else
	{
		PiudDefaultZoom::iterator ite = CClientManager::GetInstance()->m_PuidDefaultZoomMap.find(sPuid);
		if (ite != CClientManager::GetInstance()->m_PuidDefaultZoomMap.end())
		{
			nRet = ite->second;
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
	std::map<std::string, TvWallInfo> mapTvWallInfo;
	for(Group2PcMap::iterator ite = CClientManager::GetInstance()->m_Group2PcMap.begin();
		ite != CClientManager::GetInstance()->m_Group2PcMap.end(); ite++)
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
	std::string strPromat = CClientManager::GetInstance()->m_TvWallPromat;
	//给对应组发送close命令
	for(std::map<std::string, TvWallInfo>::iterator it = mapTvWallInfo.begin(); it != mapTvWallInfo.end(); it++)
	{
		std::string sRet;
		sRet.append("{");
		SetValue(sRet, "key_id", CClientManager::GetInstance()->m_KeyId);
		SetValue(sRet, "command", "COMMON", FALSE);
		SetValue(sRet, "sub_cmd", "CHANGE_MODE", FALSE);
		SetValue(sRet, "mode", iMode, FALSE);
		SetValue(sRet, "msg", strPromat.c_str(), FALSE);
		SetjsonEnd(sRet);
		CClientManager::GetInstance()->SendInfo2TVWALLServer(sRet, it->second.Ip, it->second.Port);
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