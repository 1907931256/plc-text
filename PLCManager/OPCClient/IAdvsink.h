#ifndef IADVSINK_H
#define IADVSINK_H

#include "ConnectionPoint.h"

//********************************************************
class CAdviseSink : public IAdviseSink
{
public:
	CAdviseSink();
	~CAdviseSink(void);
	
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	
	STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
	STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
	STDMETHODIMP_(void)  OnRename(LPMONIKER);
	STDMETHODIMP_(void)  OnSave(void);
	STDMETHODIMP_(void)  OnClose(void);
	
	STDMETHODIMP_(void)  OnWriteComplete(LPFORMATETC, LPSTGMEDIUM);
protected:
	ULONG               m_cRef;
};


#endif //IADVSINK_H