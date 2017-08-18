//#include "stdafx.h"
#include <afxadv.h>

#include "IAdvsink.h"
#include "OPCServer.h"
//#include "OPCClientDoc.h"

//extern OPCClientDoc* theDoc;

CAdviseSink::CAdviseSink()
{
	m_cRef=0;
	return;
}

CAdviseSink::~CAdviseSink(void)
{
	return;
}

/*
* CAdviseSink::QueryInterface
* CAdviseSink::AddRef
* CAdviseSink::Release
*
* Purpose:
*  IUnknown members for CAdviseSink object.
*/

STDMETHODIMP CAdviseSink::QueryInterface(REFIID riid, void** ppv)
{
	*ppv=NULL;
	
	if (IID_IUnknown==riid || IID_IAdviseSink==riid)
		*ppv=this;
	
	if (NULL!=*ppv)
        {
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
        }
	
	return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CAdviseSink::AddRef(void)
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) CAdviseSink::Release(void)
{
	if (0 != --m_cRef)
		return m_cRef;
	
	delete this;
	return 0;
}



/*
* CAdviseSink::OnDataChange
*
* Purpose:
*  Notifes the advise sink that data changed in a data object.
*  On this message you may request a new data rendering and update
*  your displays as necessary.  Any data sent to this function is
*  owned by the caller, not by this advise sink.
*
*  All Advise Sink methods are asynchronous and therefore we
*  should attempt no synchronous calls from within them to an EXE
*  object.  If we do, we'll get RPC_E_CALLREJECTED.
*
* Parameters:
*  pFEIn           LPFORMATETC describing format that changed
*  pSTM            LPSTGMEDIUM providing the medium in which the
*                  data is provided.
*
* Return Value:
*  None
*/

STDMETHODIMP_(void) CAdviseSink::OnDataChange(LPFORMATETC pFE
					      , LPSTGMEDIUM pSTM)
{
	// Verify the format follows the OPC spec
	if( TYMED_HGLOBAL != pFE->tymed )
		return;
	if( pSTM->hGlobal == 0 )
		return;

/*
#ifdef DATATIMEFORMAT
	if( OPCSTMFORMATDATATIME != pFE->cfFormat )
#else
	if( OPCSTMFORMATDATA != pFE->cfFormat )
#endif // DATATIMEFORMAT
*/
	
	{
		OnWriteComplete(pFE, pSTM);
		return;
	}
	
	// It must be a data advise
	const BYTE* buffer = (BYTE*)GlobalLock( pSTM->hGlobal );
	if( !buffer )
		return;
	
	const OPCGROUPHEADER* pHeader = (OPCGROUPHEADER*)buffer;
	// check pHeader->hClientGroup
	// pHeader->dwTransactionID
	// pHeader->hrStatus
	int offset = sizeof(OPCGROUPHEADER);

// for each item in the data stream, get the value and quality
#ifdef DATATIMEFORMAT
		for( DWORD index=0; index<pHeader->dwItemCount; index++, offset += sizeof(OPCITEMHEADER1) )
		{
			const OPCITEMHEADER1* pItemHeader = (OPCITEMHEADER1*)&buffer[offset];
			Item* pItem = (Item*)pItemHeader->hClient;
			pItem->timestamp = pItemHeader->ftTimeStampItem;
#else
			for( DWORD index=0; index<pHeader->dwItemCount; index++, offset += sizeof(OPCITEMHEADER2) )
			{
				const OPCITEMHEADER2* pItemHeader = (OPCITEMHEADER2*)&buffer[offset];
				Item* pItem = (Item*)pItemHeader->hClient;
#endif // DATATIMEFORMAT
				
				VARIANT* pValue = (VARIANT*)&buffer[pItemHeader->dwValueOffset];
				// Strings and arrays are packed in the stream
				// requiring unpacking
				if( pValue->vt == VT_BSTR )
				{
					pValue->bstrVal = (BSTR)((BYTE*)pValue + sizeof(VARIANT) + sizeof(DWORD));
				}
				else if( (pValue->vt & VT_ARRAY) == VT_ARRAY )
				{
					pValue->parray = (SAFEARRAY*)((BYTE*)pValue + sizeof(VARIANT));
					pValue->parray->pvData = ((BYTE*)pValue->parray + sizeof(SAFEARRAY));
					if( pValue->vt == (VT_ARRAY|VT_BSTR) )
					{
						BSTR* pStrings = (BSTR*)pValue->parray->pvData;
						LONG lBound=0;
						LONG uBound=0;
						SafeArrayGetLBound(pValue->parray, 1, &lBound);
						SafeArrayGetUBound(pValue->parray, 1, &uBound);
						ULONG dataSize = pValue->parray->rgsabound[0].cElements * pValue->parray->cbElements;
						BSTR bstr = (BSTR)((BYTE*)pValue->parray->pvData + dataSize + sizeof(DWORD));
						
						for( LONG index=0; index<=uBound-lBound; index++ )
						{
							pStrings[index] = bstr;
							ULONG len = SysStringByteLen( bstr )+sizeof(DWORD)+sizeof(WCHAR);
							bstr = (BSTR)((BYTE*)bstr + len);
						}
					}
				}
				pItem->value = pValue;
				pItem->quality = pItemHeader->wQuality;
			}
			
			GlobalUnlock( pSTM->hGlobal );
			
			//PostMessage(theDoc->data_hwnd, WM_DATACHANGE, 0, 0);
			
			return;
		}
		
		
		
STDMETHODIMP_(void) CAdviseSink::OnWriteComplete(LPFORMATETC pFE
	, LPSTGMEDIUM pSTM)
{
	const BYTE* buffer = (BYTE*)GlobalLock( pSTM->hGlobal );
	if( !buffer )
		return;
	
	const OPCGROUPHEADERWRITE* pHeader = (OPCGROUPHEADERWRITE*)buffer;
	if( FAILED(pHeader->hrStatus) )
	{
		//PostMessage(theDoc->data_hwnd, WM_DATAWRITE, pHeader->hrStatus, 0);
	}
	else
	{
		int offset = sizeof(OPCGROUPHEADERWRITE);
		// for each item in the data stream, access its data
		for( DWORD index=0; index<pHeader->dwItemCount; index++, offset += sizeof(OPCITEMHEADERWRITE) )
		{
			const OPCITEMHEADERWRITE* pItemHeader = (OPCITEMHEADERWRITE*)&buffer[offset];
			if( FAILED(pItemHeader->dwError) )
			{
				//PostMessage(theDoc->data_hwnd, WM_DATAWRITE, pItemHeader->dwError, 0);
			}
		}
	}
	GlobalUnlock( pSTM->hGlobal );
	
	return;
}


/*
* CAdviseSink::OnViewChange
* CAdviseSink::OnRename
* CAdviseSink::OnSave
* CAdviseSink::OnClose
*
* Unimplemented members
*/

STDMETHODIMP_(void) CAdviseSink::OnViewChange(DWORD dwAspect
	, LONG lindex)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnRename(LPMONIKER pmk)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnSave(void)
{
	return;
}

STDMETHODIMP_(void) CAdviseSink::OnClose(void)
{
	return;
}




		
