//**************************************************************************
//
//  Copyright (c) FactorySoft, INC. 1996-1998, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  ConnectionPoint.cpp
//  $Author    :  Jim Hansen
//
//  Description:  Callback class for OPC Data Access 2.0
//
//
//**************************************************************************
//#include "stdafx.h"
#include "IAdvsink.h"
#include "ConnectionPoint.h"
#include "OPCServer.h"

//#include "OPCClientDoc.h"

CComModule _Module;  // just needed to keep ATL happy

//extern OPCClientDoc* theDoc;

//**************************************************************************
// Called by the server at the update rate when data changes
STDMETHODIMP OPCCallback::OnDataChange(
				       DWORD       Transid,
				       OPCHANDLE   grphandle,
				       HRESULT     masterquality,
				       HRESULT     mastererror,
				       DWORD       count,
				       OPCHANDLE * clienthandles,
				       VARIANT   * values,
				       WORD      * quality,
				       FILETIME  * time,
				       HRESULT   * errors)
{
	// If Transid != 0, this is a refresh
	for( DWORD index=0; index<count; index++ )
	{
		if( SUCCEEDED(errors[index]) )
		{
			Item* pItem = (Item*)clienthandles[index];
			if( pItem )
			{
				pItem->value = values[index];
				pItem->quality = quality[index];
				pItem->timestamp = time[index];
			}
		}
	}
	//PostMessage(theDoc->data_hwnd, WM_DATACHANGE, 0, 0);
	return S_OK;
}

//**************************************************************************
// Called by the server to complete an ASyncIO2::Read
STDMETHODIMP OPCCallback::OnReadComplete(
					 DWORD       Transid,
					 OPCHANDLE   grphandle,
					 HRESULT     masterquality,
					 HRESULT     mastererror,
					 DWORD       count,
					 OPCHANDLE * clienthandles,
					 VARIANT   * values,
					 WORD      * quality,
					 FILETIME  * time,
					 HRESULT   * errors)
{
	ASSERT( Transid == 3 ); // test only
	for( DWORD index=0; index<count; index++ )
	{
		if( SUCCEEDED(errors[index]) )
		{
			Item* pItem = (Item*)clienthandles[index];
			if( pItem )
			{
				pItem->value = values[index];
				pItem->quality = quality[index];
				pItem->timestamp = time[index];
			}
		}
	}
	//PostMessage(theDoc->data_hwnd, WM_DATACHANGE, 0, 0);
	return S_OK;
}


//**************************************************************************
// Called by the server to complete an ASyncIO2::Write
STDMETHODIMP OPCCallback::OnWriteComplete(
					  DWORD       Transid,
					  OPCHANDLE   grphandle,
					  HRESULT     mastererr,
					  DWORD       count,
					  OPCHANDLE * clienthandles,
					  HRESULT   * errors)
{
	ASSERT( Transid == 2 ); // test only
	for( DWORD index=0; index<count; index++ )
	{
		if( FAILED(errors[index]) )
		{
			Item* pItem = (Item*)clienthandles[index];
			//PostMessage(theDoc->data_hwnd, WM_DATAWRITE, errors[index], 0);
		}
	}
	return S_OK;
}


//**************************************************************************
// Called by the server to complete an ASyncIO2::Cancel
STDMETHODIMP OPCCallback::OnCancelComplete(
					   DWORD       transid,
					   OPCHANDLE   grphandle)
{
	return S_OK;
}

//**************************************************************************
// Called by 2.0 servers when shutting down.
STDMETHODIMP OPCShutdown::ShutdownRequest(LPCWSTR szReason)
{
	//reason = szReason;
	//PostMessage(theDoc->data_hwnd, WM_OPCSHUTDOWN, (WPARAM)reason.GetBuffer(0), 0);
	return S_OK;
}
