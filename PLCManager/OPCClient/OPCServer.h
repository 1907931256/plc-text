#ifndef H_OPCSERVER_H
#define H_OPCSERVER_H

#include "OPCWrap.h"
#include "IAdvsink.h"		//Class CAdviseSink

//#include <afxwin.h>
//#include <Windows.h>


class COPCServer;
class COPCGroup;

//*************************************************
class Item : public CObject
{
DECLARE_SERIAL(Item)

public:
	Item() : quality(0), hServerHandle(0) 
	{
		CoFileTimeNow(&timestamp);	//获取当前时间
		hServerHandle = 0;
		quality = OPC_QUALITY_UNCERTAIN;
		access_rights = OPC_READABLE;
		active = false;
		native_type = VT_EMPTY;
		value.vt = VT_EMPTY;
		description = _T("");

		server = 0;	//服务器指针为空
		group = 0;
	}

	~Item(){}
	

	void Serialize(CArchive& ar);

	Item& operator= (Item& item){
	}

public:
	CString		name;
	CString		access_path;
	CString		description;
	COleVariant	value;
	VARTYPE		native_type;
	WORD		quality;
	FILETIME	timestamp;
	OPCHANDLE	hServerHandle;
	
	WORD		access_rights;
	BOOL		active;

	COPCServer*	server;
	COPCGroup*	group;
};
typedef CTypedPtrList<CObList, Item*> ItemList;

//*************************************************
class COPCGroup : public CObject
{
DECLARE_SERIAL(COPCGroup)

public:
	COPCGroup(COPCServer* server = 0);
	COPCGroup(COPCServer* server, COPCGroup& group);
	
	~COPCGroup();

	virtual void	Serialize(CArchive& ar);

public:
	Item* add_item(Item* item);
	inline CString		get_name(){return group_name;}
	inline CString		set_name(CString name = _T("Group")){
		group_name = name;
		return group_name;
	}
	inline CString		set_name(LPCSTR name = NULL){
		if(name){
			group_name = name;
		}

		return group_name;
	}

	inline unsigned int	get_update_rate(){ return update_rate;}
	inline unsigned int	set_update_rate(unsigned int rate = 1000){
		update_rate = rate;
		return update_rate;
	}

	inline float		get_dead_band(){ return dead_band;}
	inline float		set_dead_band(float band = 0.0){
		dead_band = band;
		return dead_band;
	}

	inline unsigned int	get_time_bias(){return time_bias;}
	inline unsigned int	set_time_bias(unsigned int bias = 0){
		time_bias = bias;
		return time_bias;
	}

	inline BOOL		get_active(){ return active;}
	inline BOOL		set_active(BOOL act = FALSE){
		active = act;
		return active;
	}
	
	inline unsigned long	get_local_id(){ return local_id;}
	inline unsigned long	set_local_id(unsigned long id = 0){
		local_id = id;
		return local_id;
	}

	ItemList&	get_items(){ return items;}
	
	COPCGroup& operator= (COPCGroup& group){
		if(this != &group){
			group_name = group.get_name();
			update_rate = group.get_update_rate();
			dead_band = group.get_dead_band();
			time_bias = group.get_time_bias();
			active = group.get_active();
			local_id = group.get_local_id();
			group_handle = 0;

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
		
		return *this;
	}
	COPCServer* get_parent() const;

public:
	HTREEITEM	tree_item;
protected:
	CString		group_name;
	unsigned int	update_rate;
	float		dead_band;
	unsigned int	time_bias;
	unsigned long	local_id;
	BOOL		active;
	
public:
	COPCServer*	parent;
	Item*		current_item;
	ItemList	items;

	//OPC 1.0
	DWORD			connection1;
	DWORD			connection2;
	OPCCallback*		call_back;
	DWORD			call_back_connection;
	OPCHANDLE		group_handle;
	CAdviseSink*		advise_sink;
	bool			use_cp;			//uses connection point	

public:
	DWORD add_items(DWORD item_count, Item* items);
	void write_item(BOOL async, CString value, Item* item = NULL);
	//void read_item(Item* item = NULL);
	int read_item(bool& readStatus,Item* item = NULL);
	int read_item(Item* item = NULL);
	void remove_item();
	
	OPCGroupStateMgt	opc_group;
};
typedef CTypedPtrList<CPtrList, COPCGroup*> GroupList;
//*************************************************
class OPCServerInfo
{
public:
	OPCServerInfo( LPOLESTR ProgID, LPOLESTR Description, CLSID& clsid )
		: m_ProgID(ProgID), m_Description(Description), m_clsid(clsid) 
	{
		m_NodeName = _T("127.0.0.1");
	}
	
	OPCServerInfo(OPCServerInfo& info){
		m_ProgID = info.m_ProgID;
		m_Description = info.m_Description;
		m_NodeName = info.m_NodeName;
		m_clsid = info.m_clsid;
	}

	OPCServerInfo(){
		m_NodeName = _T("127.0.0.1");
	}
	
	OPCServerInfo& operator= (OPCServerInfo& info){
		if(this != &info){
			m_ProgID = info.m_ProgID;
			m_Description = info.m_Description;
			m_NodeName = info.m_NodeName;
			m_clsid = info.m_clsid;
		}

		return *this;
	}

public:
	CString		m_NodeName;
	CString		m_ProgID;
	CString		m_Description;
	CLSID		m_clsid;
};
typedef CTypedPtrList<CPtrList, OPCServerInfo*> ServerInfoList;

//**************************************************
class COPCServer : public CObject
{
DECLARE_SERIAL(COPCServer)

public:
	COPCGroup* get_current_group();
	COPCGroup* add_group(COPCGroup* group);
	OPCServerInfo* const get_server_info();
	HRESULT GetErrorString(
		HRESULT		dwError,
		LCID		dwLocale,
		LPWSTR*		ppString);

	bool is_connected();
	bool disconnect();
	bool connect(OPCServerInfo* info = 0);
	OPCServerInfo* SetServerInfo(OPCServerInfo* info = 0);
	COPCServer();
	~COPCServer();

public:
	void remove_group(COPCGroup* group = 0);
	void Serialize(CArchive& ar);
	COPCGroup* set_current_group(COPCGroup* group);
	OPCServer& get_opc_server();
	HTREEITEM		tree_item;
	GroupList		groups;

	//OPCHANDLE		group_handle;
	//OPCGroupStateMgt	opc_group;
	OPCServer		opc_server;
protected:
	OPCServerInfo*		server_info;

	COPCGroup*		current_group;

	
protected:
	//OPC 2.0
	OPCShutdown*		shut_down;
	DWORD			shut_down_connection;

	DWORD			transaction_id;
	

};
typedef CTypedPtrList<CPtrList, COPCServer*> ServerList;

#endif//H_OPCSERVER_H