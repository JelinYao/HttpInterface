#pragma once
#include <string>
#include <map>
using std::wstring;
using std::string;
using std::map;
#include <WinSock2.h>
#include "httpHeader.h"

class CHttpSocket
	: public ISocketHttp
{
public:
	CHttpSocket();
	virtual ~CHttpSocket();
	virtual bool DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath);
	virtual void SetDownloadCallback(IHttpCallback* pCallback, void* pParam);
	virtual HttpInterfaceError GetErrorCode() { return m_paramsData.errcode; }
	virtual	LPCWSTR	GetIpAddr()const { return m_strIpAddr.c_str();	}
	virtual	void FreeInstance() { delete this; }
	virtual bool DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);
	virtual void AddHeader(LPCSTR key, LPCSTR value);

protected:
	bool	InitSocket(const string& strHostName, const WORD sPort);

private:
	SOCKET	m_socket;
	wstring	m_strIpAddr;
	CHttpHeader m_header;
	HttpParamsData m_paramsData;
};