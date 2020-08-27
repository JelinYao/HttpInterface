#pragma once
#include <string>
#include <map>
#include "IHttpInterface.h"


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
	//转成http请求头列表
	std::string toHttpHeaders();

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