#pragma once
#include <map>
#include "httpHeader.h"


class CWininetHttp
	: public IWininetHttp
{
public:
	CWininetHttp(void);
	virtual ~CWininetHttp(void);
	virtual string	Request(LPCSTR pUrl, HttpRequest type, LPCSTR pPostData = NULL, LPCSTR pHeader=NULL);
	virtual string	Request(LPCWSTR pUrl, HttpRequest type, LPCSTR pPostData = NULL, LPCWSTR pHeader=NULL);
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath);
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);
	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam);
	virtual HttpInterfaceError GetErrorCode() { return m_paramsData.errcode; }
	virtual	void	FreeInstance() { delete this; }
	virtual void AddHeader(LPCSTR key, LPCSTR value);
	
protected:
	//¹Ø±Õ¾ä±ú
	void	ReleaseHandle(HINTERNET& hInternet);
	void	Release();

private:
	bool		m_bHttps;
	HINTERNET	m_hSession;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	CHttpHeader m_header;
	HttpParamsData m_paramsData;
};
