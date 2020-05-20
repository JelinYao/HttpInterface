#pragma once
#include <string>
#include <map>
using std::wstring;
using std::string;
using std::map;
#include <WinSock2.h>

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

protected:
	class CHttpHeader
	{
	public:
		CHttpHeader();
		CHttpHeader(const char* pHeader);
		CHttpHeader(const std::string& strHeader);
		virtual	~CHttpHeader(void);


		const std::string& getHttpVersion()const { return m_httpVersion; }
		void setHttpVersion(const std::string& version) { m_httpVersion = version; }
		void setRequestPath(const std::string& path) { m_requestPath = path; }

		const int GetReturnValue()const { return m_uReturnValue; }

		const char*	GetContent()const { return m_strContent.c_str(); }

		std::string	GetValue(const std::string& strKey);

		void addHeader(const std::string& key, const std::string& value);
		void setUserAgent(const std::string& userAgent);
		void setHost(const std::string& host);
		void setRange(__int64 range);

		//转成http请求头
		std::string toString(HttpRequest type);

	protected:
		//解析HTTP头结构
		bool Revolse(const std::string& strHeader);

	private:
		int	 m_uReturnValue;
		std::string m_httpVersion;
		std::string	m_strContent;
		std::string m_requestPath;
		std::map<std::string, std::string>	m_headers;
	};

protected:
	bool	InitSocket(const string& strHostName, const WORD sPort);

private:
	SOCKET	m_socket;
	wstring	m_strIpAddr;
	HttpParamsData m_paramsData;
};