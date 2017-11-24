#include "stdafx.h"
#include "opcserver.h"
//#include "opcclientdoc.h"

#define FULL_TEST

//extern OPCClientDoc* theDoc;
// The OPC data formats
UINT OPCSTMFORMATDATA = RegisterClipboardFormat(_T("OPCSTMFORMATDATA"));
UINT OPCSTMFORMATDATATIME = RegisterClipboardFormat(_T("OPCSTMFORMATDATATIME"));
UINT OPCSTMFORMATWRITECOMPLETE = RegisterClipboardFormat(_T("OPCSTMFORMATWRITECOMPLETE"));

IMPLEMENT_SERIAL(Item, CObject,0)
IMPLEMENT_SERIAL(COPCGroup, CObject, 0)
IMPLEMENT_SERIAL(COPCServer, CObject, 0)

//************************************************************
//************************************************************
//************************************************************
//class Item

void Item::Serialize(CArchive& ar)
{
	if(ar.IsStoring())
	{
		ar<<name;		//名称
		ar<<access_path;	//访问路径
		ar<<description;
	}
	else
	{
		ar>>name;
		ar>>access_path;
		ar>>description;
	}
}
//************************************************************
//默认构造函数
COPCGroup::COPCGroup(COPCServer* server)
	:group_name(_T("Group1")), 
	update_rate(1000),
	dead_band(0.0),
	time_bias(0),
	active(false),
	local_id(0),
	group_handle(0),
	parent(server)
{
	connection1 = 0;
	connection2 = 0;
	call_back_connection = 0;
	use_cp = false;
	
	tree_item = 0;

	advise_sink = new CAdviseSink;
	ASSERT(advise_sink);
	advise_sink->AddRef();
	
	call_back = new OPCCallbackObject;
	call_back->AddRef();

	current_item = NULL;
	while(!items.IsEmpty())
		delete items.RemoveHead();
}

//拷贝构造函数
COPCGroup::COPCGroup(COPCServer* server, COPCGroup& group)
{
	group_name = group.get_name();
	update_rate = group.get_update_rate();
	dead_band = group.get_dead_band();
	time_bias = group.get_time_bias();
	active = group.get_active();
	local_id = group.get_local_id();
	group_handle = 0;
	
	parent = group.parent;
	tree_item = 0;

	connection1 = 0;
	connection2 = 0;
	call_back_connection = 0;
	use_cp = false;
	
	advise_sink = new CAdviseSink;
	ASSERT(advise_sink);
	advise_sink->AddRef();
	
	call_back = new OPCCallbackObject;
	call_back->AddRef();
	
	current_item = NULL;
	while(!items.IsEmpty())
		delete items.RemoveHead();
	
	ItemList& item_list = group.get_items();
	POSITION item_pos = item_list.GetHeadPosition();
	while(item_pos){
		Item* item = item_list.GetNext(item_pos);
		Item* new_item = new Item;
		
		if(!new_item || !item)
			continue;
		
		CoFileTimeNow(&new_item->timestamp);
		new_item->name = item->name;
		new_item->access_path = item->access_path;
		new_item->description = item->description;
		new_item->hServerHandle = item->hServerHandle;
		new_item->quality = item->quality;
		new_item->access_rights = item->access_rights;
		new_item->active = item->active;
		new_item->native_type = item->native_type;
		new_item->value = item->value;
		new_item->server = parent;
		new_item->group = this;
		
		items.AddTail(new_item);
	}
}

COPCGroup::~COPCGroup(){

	//删除每个点
	HRESULT hr = S_OK;
	if(opc_group.IsOk()){
		if(use_cp)
			hr = AtlUnadvise(opc_group, IID_IOPCDataCallback, call_back_connection);
		else{
			DataObject data_object;
			hr = data_object.Attach(opc_group);
			if(SUCCEEDED(hr)){
				if(connection1)
					hr = data_object.DUnadvise(connection1);
				if(connection2)
					hr = data_object.DUnadvise(connection2);

				data_object.Detach();
			}
		}
		
		int count = items.GetCount();
		if(count > 0){
			OPCItemMgt item_mgt;
			hr = item_mgt.Attach(opc_group);
			if(SUCCEEDED(hr)){
				HRESULT* errors = NULL;
				OPCHANDLE* handles = new OPCHANDLE[count];
				ASSERT(handles);

				POSITION item_pos = items.GetHeadPosition();
				for(int index = 0; item_pos; index++){
					Item* item = items.GetNext(item_pos);
					ASSERT(item);

					handles[index] = item->hServerHandle;
				}
				hr = item_mgt.RemoveItems(count, handles, &errors);
				if(SUCCEEDED(hr))
					CoTaskMemFree(errors);

				delete[] handles;

				item_mgt.Detach();
			}
		}
	}	
	if(parent && parent->is_connected()){
		hr = parent->opc_server.RemoveGroup(group_handle, FALSE);
		if(FAILED(hr))
		{
			//	theDoc->ReportError(_T("Remove Group Failed: "), hr);
		}

	}
		

	current_item = NULL;
	while(!items.IsEmpty())
		delete items.RemoveHead();

	call_back->Release();
	advise_sink->Release();

	opc_group.Detach();
}

void COPCGroup::Serialize(CArchive& ar){
	if(ar.IsStoring()){
		ar << group_name;
		ar << update_rate;
		ar << dead_band;
		ar << time_bias;
		ar << local_id;
		ar << active;

		int item_count = items.GetCount();
		ar << item_count;
		POSITION item_pos = items.GetHeadPosition();
		while(item_pos){
			Item* item = items.GetNext(item_pos);
			if(item)
				item->Serialize(ar);
		}
	}
	else{
		ar >> group_name;
		ar >> update_rate;
		ar >> dead_band;
		ar >> time_bias;
		ar >> local_id;
		ar >> active;
		
		active = TRUE;

		
		if(parent && parent->is_connected())
			parent->add_group(this);
		else{
			delete this;
			return;
		}

		while(!items.IsEmpty())
			delete items.RemoveHead();

		int item_count = 0;
		ar >> item_count;
		while(item_count --){
			Item* item = new Item;
			if(item){
				item->Serialize(ar);
				item->active = TRUE;

				//items.AddTail(item);
				add_item(item);
			}
		}
	}
}

Item* COPCGroup::add_item(Item *item)
{
	USES_CONVERSION;
	//CWaitCursor wait;

	if(!(item && parent)){
		delete item;
		return 0;
	}
	
	OPCItemMgt item_mgt;
	HRESULT hr = item_mgt.Attach(opc_group);
	if(FAILED(hr)){
		delete item;
		return 0;
	}
	
	current_item = item;		//当前点

	OPCITEMDEF	item_def;
	item_def.szItemID = T2OLE(item->name.GetBuffer(0));
	item_def.szAccessPath = T2OLE(item->access_path.GetBuffer(0));
	item_def.pBlob = NULL;
	item_def.dwBlobSize = 0;
	item_def.bActive = item->active;
	item_def.hClient = (OPCHANDLE)item;
	item_def.vtRequestedDataType = item->native_type;
	
	//添加点
	OPCITEMRESULT*	results;
	HRESULT*	errors;
	hr = item_mgt.AddItems(1, &item_def, &results, &errors);
	if(FAILED(hr)){
		//theDoc->ReportError(_T("Add Items: "), hr);
		delete item;

		return 0;
	}
	item->hServerHandle = results->hServer;
	item->native_type = results->vtCanonicalDataType;
	
	HRESULT item_result = errors[0];
	if(results->pBlob != NULL)
		CoTaskMemFree(results->pBlob);
	
	CoTaskMemFree(results);
	CoTaskMemFree(errors);
	
	if(FAILED(item_result)){
		//theDoc->ReportError(_T("Add Items: "), hr);
		delete item;
		return 0;
	}

	items.AddTail(item);		//保存点
	
	//读取初始值
	OPCSyncIO sync_io;
	hr = sync_io.Attach(opc_group);
	if(SUCCEEDED(hr)){
		OPCITEMSTATE* item_state = NULL;
		hr = sync_io.Read(
			OPC_DS_CACHE,
			1,
			&item->hServerHandle,
			&item_state,
			&errors);
		if(SUCCEEDED(hr)){
			ASSERT(item_state->hClient == (OPCHANDLE)item);
			item->quality = item_state->wQuality;
			item->value = item_state->vDataValue;
			
			VariantClear(&item_state->vDataValue);
			CoTaskMemFree(item_state);
			CoTaskMemFree(errors);
		}
		else{
		//	theDoc->ReportError(_T("SysncIO Read: "), hr);
			return 0;
		}

		sync_io.Detach();
	}
	
	return item;
}


DWORD COPCGroup::add_items(DWORD item_count, Item *item_arr)
{
	USES_CONVERSION;
	CWaitCursor wait;

	DWORD dwCount = 0;

	for(DWORD i = 0; i < item_count; i++){
		Item* item = item_arr + i;
		
		if(!(item && parent)){
			delete item;
			item = 0;
			continue;
		}
		
		OPCItemMgt item_mgt;
		HRESULT hr = item_mgt.Attach(opc_group);
		if(FAILED(hr)){
			delete item;
			item = 0;
			continue;
		}
		
		current_item = item;		//当前点
		
		OPCITEMDEF	item_def;
		item_def.szItemID = T2OLE(item->name.GetBuffer(0));
		item_def.szAccessPath = T2OLE(item->access_path.GetBuffer(0));
		item_def.pBlob = NULL;
		item_def.dwBlobSize = 0;
		item_def.bActive = item->active;
		item_def.hClient = (OPCHANDLE)item;
		item_def.vtRequestedDataType = item->native_type;
		
		//添加点
		OPCITEMRESULT*	results;
		HRESULT*	errors;
		hr = item_mgt.AddItems(1, &item_def, &results, &errors);
		if(FAILED(hr)){
			//theDoc->ReportError(_T("Add Items: "), hr);
			delete item;
			item = 0;
			continue;
		}
		item->hServerHandle = results->hServer;
		item->native_type = results->vtCanonicalDataType;
		
		HRESULT item_result = errors[0];
		if(results->pBlob != NULL)
			CoTaskMemFree(results->pBlob);
		
		CoTaskMemFree(results);
		CoTaskMemFree(errors);
		
		if(FAILED(item_result)){
			//theDoc->ReportError(_T("Add Items: "), hr);
			delete item;
			item = 0;
			continue;
		}
		
		items.AddTail(item);		//保存点
		
		//读取初始值
		OPCSyncIO sync_io;
		hr = sync_io.Attach(opc_group);
		if(SUCCEEDED(hr)){
			OPCITEMSTATE* item_state = NULL;
			hr = sync_io.Read(
				OPC_DS_CACHE,
				1,
				&item->hServerHandle,
				&item_state,
				&errors);
			if(SUCCEEDED(hr)){
				ASSERT(item_state->hClient == (OPCHANDLE)item);
				item->quality = item_state->wQuality;
				item->value = item_state->vDataValue;
				
				VariantClear(&item_state->vDataValue);
				CoTaskMemFree(item_state);
				CoTaskMemFree(errors);
			}
			else{
				//theDoc->ReportError(_T("SysncIO Read: "), hr);
				continue;
			}
			
			sync_io.Detach();
		}

		dwCount++;
	}

	return dwCount;
}
//****************************************************
COPCServer::COPCServer(){
	current_group = 0;
	server_info = 0;

	shut_down = new OPCShutdownObject;
	shut_down->AddRef();
	shut_down_connection = 0;

	tree_item = 0;
}

COPCServer::~COPCServer(){
	if(is_connected())
		disconnect();
	
	shut_down->Release();

	current_group = NULL;
	while(!groups.IsEmpty())
		delete groups.RemoveHead();

	if(server_info){
		delete server_info;
		server_info = 0;
	}
}


OPCServerInfo* COPCServer::SetServerInfo(OPCServerInfo *info)
{
	if(is_connected()){
		delete info;
		info = 0;
		
		return 0;
	}

	if(info){
		if(server_info)
			delete server_info;

		server_info = info;
	}

	return info;
}

//连接OPCServer
bool COPCServer::connect(OPCServerInfo *info)
{
	USES_CONVERSION;

	if(is_connected())
		return true;
	
	//CWaitCursor wait_cursor;

	if(info)
		server_info = info;
	else
		info = server_info;
	
	bool use_node = true;
	if(info->m_NodeName.IsEmpty()
		|| _tcsicmp(_T(""), info->m_NodeName) == 0
		|| _tcsicmp(_T("localhost"), info->m_NodeName) == 0
		|| _tcsicmp(_T("127.0.0.1"), info->m_NodeName) ==0 ){
			use_node = false;
		}
	if(use_node){	//是否同为本机
		TCHAR sz_local_host[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dw_host_size = sizeof(sz_local_host);
		if(GetComputerName(sz_local_host, &dw_host_size)){
			if(_tcsicmp(sz_local_host, info->m_NodeName) == 0)
				use_node = false;
		}
	}
	
	//建立OPC对象
	HRESULT hr = S_OK;
	LPUNKNOWN punknown = NULL;
	
	if(!use_node){	//本地OPCServer
		hr = CoCreateInstance(
			info->m_clsid,
			NULL,
			CLSCTX_ALL,
			IID_IUnknown,
			(LPVOID*)&punknown);
		
		if(FAILED(hr) || punknown == NULL){	//创建失败
			CString msg;
			msg.Format(_T("CoCreateInstance %s failed:"), info->m_ProgID.GetBuffer(0));
			//theDoc->ReportError(msg, hr);

			return false;
		}
	}
	else{		//远程对象
		COSERVERINFO	si;
		MULTI_QI	qi;

		si.dwReserved1 = 0;
		si.pAuthInfo = NULL;
		si.pwszName = T2OLE(info->m_NodeName.GetBuffer(0));
		si.dwReserved2 = 0;

		qi.pIID = &IID_IOPCServer;
		qi.pItf = NULL;
		qi.hr = 0;
		
		hr = CoCreateInstanceEx(
			info->m_clsid,
			NULL,
			CLSCTX_REMOTE_SERVER,
			&si,
			1,
			&qi);
		if(FAILED(hr)){
			CString msg(_T("CoCreateInstanceEx failed: "));
			//theDoc->ReportError(msg, hr);

			return false;
		}
		if(FAILED(qi.hr) || (qi.pItf == NULL)){
			CString msg(_T("MULTI_QI failed: "));
			//theDoc->ReportError(msg, qi.hr);

			return false;
		}

		punknown = qi.pItf;
	}

	hr = opc_server.Attach(punknown);
	punknown->Release();	//释放IUnknown接口
	punknown = NULL;

	if(FAILED(hr)){
		CString msg(_T("OPC Server Attach failed:"));
		//if(theDoc)
		//	theDoc->ReportError(msg, hr);
	}

	//是否实现了IShutdown接口
	hr = AtlAdvise(
		opc_server,
		shut_down->GetUnknown(),
		IID_IOPCShutdown,
		&shut_down_connection
		);
	if(FAILED(hr)){
		CString msg;
		msg.Format(_T("%s: %s can't support IOPCShutdown interface."),
			info->m_NodeName,
			info->m_ProgID);
		//if(theDoc)
		//	theDoc->ReportError(msg.GetBuffer(0));
	}
	
	//添加所有组
	/*
	POSITION group_pos = groups.GetHeadPosition();
	while(group_pos){
		COPCGroup* group = groups.GetNext(group_pos);
		if(!group)
			continue;

		group = add_group(group);
		if(group){		//添加所有点
			POSITION item_pos = group->items.GetHeadPosition();
			while(item_pos){
				Item* item = group->items.GetNext(item_pos);
				if(!item)
					continue;

				item = group->add_item(item);
			}
		}
	}
	*/
	return true;
}

//断开OPCServer
bool COPCServer::disconnect()
{
	if(!is_connected())
		return false;

	HRESULT hr = S_OK;
	if(shut_down_connection){
		hr = AtlUnadvise(
			opc_server,
			IID_IOPCShutdown,
			shut_down_connection);
		shut_down_connection = 0;

		if(FAILED(hr)){
			CString msg(_T("AtlUnadvise IOPCShutdown failed."));
			//if(theDoc)
			//	theDoc->ReportError(msg.GetBuffer(0), hr);
		}
	}

	//断开所有Group,并删除
	/*.....*/
	
	current_group =  NULL;
	while(!groups.IsEmpty())
		delete groups.RemoveHead();

	opc_server.Detach();
	
	return true;
}

//连接状态
bool COPCServer::is_connected()
{
	return opc_server.IsOk() ? true : false;
}

//获取错误信息
HRESULT COPCServer::GetErrorString(
				   HRESULT	dwError,
				   LCID		dwLocale,
				   LPWSTR*	ppString)
{
	return opc_server.GetErrorString(dwError, dwLocale, ppString);
}

OPCServerInfo* const COPCServer::get_server_info()
{
	return server_info;
}

//添加点
COPCGroup* COPCServer::add_group(COPCGroup *group)
{
	USES_CONVERSION;

	if(!group)
		return 0;
	
	OPCServerInfo* info = server_info;
	//add group
	LPCWSTR	group_name = T2OLE(group->get_name().GetBuffer(0));
	BOOL	active = group->get_active();
	ULONG	update_rate = group->get_update_rate();
	long	time_bias = group->get_time_bias();
	float	dead_band = group->get_dead_band();
	DWORD	local_id = group->get_local_id();
	DWORD	rate;
	
	HRESULT hr = opc_server.AddGroup(
		L"Mago",
		active,
		update_rate,
		(OPCHANDLE)group,
		&time_bias,
		&dead_band,
		local_id,
		&group->group_handle,
		&rate,
		IID_IOPCGroupStateMgt,
		group->opc_group);

	if(FAILED(hr)){
		CString msg;
		msg.Format(_T("Add Group: %s failed."), group_name);
		//theDoc->ReportError(msg.GetBuffer(0), hr);

		return 0;
	}

#ifdef FULL_TEST
	IOPCGroupStateMgt* pTest=NULL;
	hr = opc_server.GetGroupByName( 
		L"Mago", 
		IID_IOPCGroupStateMgt, 
		(LPUNKNOWN*)&pTest );
	if( SUCCEEDED(hr) )
	{
		ASSERT( pTest == (IOPCGroupStateMgt*)group->opc_group );	// should get the same
		hr = pTest->SetName( group_name );				// set new name
		pTest->Release();
		if( FAILED(hr) )
		{
			//theDoc->ReportError( _T("IOPCGroupStateMgt::SetName: "), hr );
		}
		else
		{
			// should now go by this new name
			hr = opc_server.GetGroupByName( 
				group_name, 
				IID_IOPCGroupStateMgt, 
				(LPUNKNOWN*)&pTest );
			if( SUCCEEDED(hr) )
			{
				ASSERT( pTest == (IOPCGroupStateMgt*)group->opc_group );
				pTest->Release();
			}
		}
	}
#endif//FULL_TEST

	//OPC 2.0 connection point
	hr = AtlAdvise(
		group->opc_group,
		group->call_back->GetUnknown(),
		IID_IOPCDataCallback,
		&group->call_back_connection);
	if(SUCCEEDED(hr))
		group->use_cp = true;
		
	if(!group->use_cp){		//不支持连接点
		CString msg;
		msg.Format(_T("%s: %s can't support IOPCDataCallback interface."),
			info->m_NodeName,
			info->m_ProgID);
		//theDoc->ReportError(msg.GetBuffer(0), hr);
		
		//opc 1.0 data advise format
		FORMATETC format_etc;
		format_etc.tymed =  TYMED_HGLOBAL;
		format_etc.ptd = NULL;
		format_etc.dwAspect = DVASPECT_CONTENT;
		format_etc.lindex = -1;
		
		// IAdviseSink is an interface on OUR object that is passed to
		// the server for callbacks
		IAdviseSink *pAdviseSink = NULL;
		hr = group->advise_sink->QueryInterface(IID_IAdviseSink, (LPVOID *)&pAdviseSink);
		if( FAILED(hr) )
		{
			msg = _T("IAdviseSink: ");
			//theDoc->ReportError(msg.GetBuffer(0), hr);

			group->opc_group.Detach();
			//opc_server.Detach();
			return 0;
		}
		
		// Get an IDataObject interface on the group
		DataObject dataObject;
		hr = dataObject.Attach( group->opc_group );
		if(FAILED(hr) || !dataObject.IsOk() )
		{
			//some servers don't do this, so don't quit altogether
			msg = _T("IDataObject not supported by this server\nNo data notifications will take place");
			//theDoc->ReportError(msg.GetBuffer(0));
			
			return 0;
		}
		
		// Register our IAdvise with the group
		// Need to register both formats: data change, and write complete
		format_etc.cfFormat = OPCSTMFORMATWRITECOMPLETE ;
		hr = dataObject.DAdvise(&format_etc,
			ADVF_PRIMEFIRST,    // ADVF flag
			pAdviseSink,
			&group->connection2);
		if( FAILED(hr) )
		{
			msg = _T("IDataObject::DAdvise: : ");
			//theDoc->ReportError(msg.GetBuffer(0), hr);
			
			return 0;
		}
		
			
		#ifdef DATATIMEFORMAT
			format_etc.cfFormat = OPCSTMFORMATDATATIME ;
		#else
			format_etc.cfFormat = OPCSTMFORMATDATA ;
		#endif // DATATIMEFORMAT

		hr = dataObject.DAdvise(&format_etc,
			ADVF_PRIMEFIRST,    // ADVF flag
			pAdviseSink,
			&group->connection1);
		pAdviseSink->Release();
		if( FAILED(hr) )
		{
			msg = _T("IDataObject::DAdvise: : ");
			//theDoc->ReportError(msg.GetBuffer(0), hr);
			
			return 0;
		}
	}
	
	groups.AddTail(group);
	current_group = group;

	//theDoc->UpdateAllViews(NULL, ADD_GROUP, (CObject*)group);
	return current_group;
}

//获取当前组
COPCGroup* COPCServer::get_current_group()
{
	return current_group;
}

//获取父服务器
COPCServer* COPCGroup::get_parent() const
{
	return parent;
}

OPCServer& COPCServer::get_opc_server()
{
	return opc_server;
}

COPCGroup* COPCServer::set_current_group(COPCGroup *group)
{
	if(group)
		current_group = group;
	
	return group;
}

//保存
void COPCServer::Serialize(CArchive &ar)
{
	CWaitCursor wait_cursor;
	//Lock wait(&item_cs);

	if(ar.IsStoring()){
		if(server_info){
			ar << server_info->m_ProgID;
			ar << server_info->m_NodeName;
			ar << server_info->m_Description;
			ar << server_info->m_clsid.Data1;
			ar << server_info->m_clsid.Data2;
			ar << server_info->m_clsid.Data3;
			
			int times = sizeof(server_info->m_clsid.Data4) / sizeof(BYTE);
			for(int i = 0; i < times; i++){
				ar << server_info->m_clsid.Data4[i];
			}
		}

		int group_count = groups.GetCount();
		ar << group_count;
		POSITION group_pos = groups.GetHeadPosition();
		while(group_pos){
			COPCGroup* group = groups.GetNext(group_pos);
			if(group)
				group->Serialize(ar);
		}
	}
	else{
		if(server_info)
			delete server_info;

		server_info = new OPCServerInfo;
		if(server_info){
			ar >> server_info->m_ProgID;
			ar >> server_info->m_NodeName;
			ar >> server_info->m_Description;
			ar >> server_info->m_clsid.Data1;
			ar >> server_info->m_clsid.Data2;
			ar >> server_info->m_clsid.Data3;

			int times = sizeof(server_info->m_clsid.Data4) / sizeof(BYTE);
			for(int i = 0; i < times; i++){
				ar >> server_info->m_clsid.Data4[i];
			}
		}

		if(!is_connected())		//连接OPC Server
			connect();		

		while(!groups.IsEmpty())
			delete groups.RemoveHead();
		
		int group_count = 0;
		ar >> group_count;
		while(group_count--){
			COPCGroup* group = new COPCGroup;
			if(group){
				group->parent = this;
				group->Serialize(ar);

				//add_group(group);
				//groups.AddTail(group);
			}	
		}
	}
}

//删除点
void COPCGroup::remove_item()
{
	if(current_item == NULL)
		return;

	OPCItemMgt item_mgt;
	HRESULT hr = item_mgt.Attach(opc_group);
	if(SUCCEEDED(hr)){
		HRESULT* errors = NULL;
		hr = item_mgt.RemoveItems(
			1,
			&current_item->hServerHandle,
			&errors);
		if(SUCCEEDED(hr))
			CoTaskMemFree(errors);

	//	EnterCriticalSection(&item_cs);
		
		POSITION pos = items.Find(current_item);
		if(pos)
			items.RemoveAt(pos);
		
		delete current_item;
		current_item = NULL;
		
	//	LeaveCriticalSection(&item_cs);
	

		//theDoc->SetModifiedFlag(TRUE);
		//theDoc->UpdateAllViews(NULL, UPDATE_GROUP, (CObject*)this);
	}
}

void COPCServer::remove_group(COPCGroup *group)
{
//	Lock wait(&item_cs);
	if(group == NULL){
		group = current_group;
		current_group = 0;
	}

	POSITION group_pos = groups.Find(group);
	if(group_pos){
		groups.RemoveAt(group_pos);
		delete group;
	}

	//theDoc->UpdateAllViews(NULL, UPDATE_SERVER, (CObject*)this);
}

//读取点
//void COPCGroup::read_item(Item *item)
int COPCGroup::read_item(bool& readStatus,Item *item)
{
	readStatus = true;
	//OPCITEMRESULT*	results;
	HRESULT*	errors;
	HRESULT hr;
	int result=0;

	OPCSyncIO sync_io;
	hr = sync_io.Attach(opc_group);
	if(SUCCEEDED(hr)){
		OPCITEMSTATE* item_state = NULL;
		hr = sync_io.Read(
			OPC_DS_CACHE,
			1,
			&item->hServerHandle,
			&item_state,
			&errors);
		if(SUCCEEDED(hr)){
			ASSERT(item_state->hClient == (OPCHANDLE)item);
			item->quality = item_state->wQuality;
			item->value = item_state->vDataValue;

			//CString errorLog;
			//CString valueGet = item->value.boolVal ? _T("1") : _T("0");
			//
			//errorLog.Format(_T("SysncIO Read %s success value: "),item->name);
			//errorLog = errorLog + valueGet + _T("\n");
			//TRACE(errorLog);

			switch(item->value.vt){
				case VT_I2:
					//strType = _T("int");
					//strValue.Format(_T("%d"), item->value.iVal);
					result = item->value.intVal;
					break;
				case VT_UI1:
					//strType = _T("uint");
					//strValue.Format(_T("%d"),item->value.bVal);
					result = item->value.intVal;
					break;
				case VT_I4:
					//strType = _T("long");
					//strValue.Format(_T("%d"), item->value.lVal);
					result = item->value.intVal; //数据丢失！！！
					break;
				case VT_R4:
					//strType = _T("float");
					//strValue.Format(_T("%f"), item->value.fltVal);
					break;
				case VT_R8:
					//strType = _T("double");
					//strValue.Format(_T("%f"), item->value.dblVal);
					break;
				case VT_BOOL:
					//strType = _T("bool");
					//strValue = item->value.boolVal ? _T("1") : _T("0");
					result = item->value.boolVal ? 1 : 0;
					break;
				case VT_BSTR:
					//strType = _T("string");
					//strValue = item->value.bstrVal;
					break;
				default:
					//strType = _T("unknown");
					//strValue = _T("");
					break;
			}

			VariantClear(&item_state->vDataValue);
			CoTaskMemFree(item_state);
			CoTaskMemFree(errors);
		}
		else{
			readStatus = false;
		//	CString errorLog;
		//	errorLog.Format(_T("SysncIO Read %s failed: %s"),item->name,hr);
		//	TRACE(errorLog);
		
			
			//	theDoc->ReportError(_T("SysncIO Read: "), hr);
			//return 0;
		}

		sync_io.Detach();
	}
	return result;
}

//读取点
//void COPCGroup::read_item(Item *item)
int COPCGroup::read_item(Item *item)
{

	//OPCITEMRESULT*	results;
	HRESULT*	errors;
	HRESULT hr;
	int result=0;

	OPCSyncIO sync_io;
	hr = sync_io.Attach(opc_group);
	if(SUCCEEDED(hr)){
		OPCITEMSTATE* item_state = NULL;
		hr = sync_io.Read(
			OPC_DS_CACHE,
			1,
			&item->hServerHandle,
			&item_state,
			&errors);
		if(SUCCEEDED(hr)){
			ASSERT(item_state->hClient == (OPCHANDLE)item);
			item->quality = item_state->wQuality;
			item->value = item_state->vDataValue;

			//CString errorLog;
			//CString valueGet = item->value.boolVal ? _T("1") : _T("0");
			//
			//errorLog.Format(_T("SysncIO Read %s success value: "),item->name);
			//errorLog = errorLog + valueGet + _T("\n");
			//TRACE(errorLog);

			switch(item->value.vt){
				case VT_I2:
					//strType = _T("int");
					//strValue.Format(_T("%d"), item->value.iVal);
					result = item->value.intVal;
					break;
				case VT_UI1:
					//strType = _T("uint");
					//strValue.Format(_T("%d"),item->value.bVal);
					result = item->value.intVal;
					break;
				case VT_I4:
					//strType = _T("long");
					//strValue.Format(_T("%d"), item->value.lVal);
					result = item->value.intVal; //数据丢失！！！
					break;
				case VT_R4:
					//strType = _T("float");
					//strValue.Format(_T("%f"), item->value.fltVal);
					break;
				case VT_R8:
					//strType = _T("double");
					//strValue.Format(_T("%f"), item->value.dblVal);
					break;
				case VT_BOOL:
					//strType = _T("bool");
					//strValue = item->value.boolVal ? _T("1") : _T("0");
					result = item->value.boolVal ? 1 : 0;
					break;
				case VT_BSTR:
					//strType = _T("string");
					//strValue = item->value.bstrVal;
					break;
				default:
					//strType = _T("unknown");
					//strValue = _T("");
					break;
			}

			VariantClear(&item_state->vDataValue);
			CoTaskMemFree(item_state);
			CoTaskMemFree(errors);
		}
		else{

				CString errorLog;
			//	errorLog.Format(_T("SysncIO Read %s failed: %s"),item->name,hr);
			//	TRACE(errorLog);


			//	theDoc->ReportError(_T("SysncIO Read: "), hr);
			//return 0;
		}

		sync_io.Detach();
	}
	return result;
}

void COPCGroup::write_item(BOOL async, CString value, Item *item)
{

}

