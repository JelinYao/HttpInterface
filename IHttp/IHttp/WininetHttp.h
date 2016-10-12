#pragma once

//每次读取的字节数
#define READ_BUFFER_SIZE		4096


class CWininetHttp
	: public IWininetHttp
	, public IHttp
{
public:
	CWininetHttp(void);
	virtual ~CWininetHttp(void);
	virtual string	Request(LPCSTR pUrl, HttpRequest type, LPCSTR pPostData = NULL, LPCSTR pHeader=NULL);
	virtual string	Request(LPCWSTR pUrl, HttpRequest type, LPCSTR pPostData = NULL, LPCWSTR pHeader=NULL);
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath);
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);
	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam)		{ m_pCallback = pCallback; m_lpParam = pParam; }
	virtual HttpInterfaceError GetErrorCode()										{ return m_error;	}
	virtual	void	FreeInstance()													{ delete this;		}
	
protected:
	//关闭句柄
	void	ReleaseHandle(HINTERNET& hInternet);
	void	Release();
private:
	HINTERNET	m_hSession;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
};
