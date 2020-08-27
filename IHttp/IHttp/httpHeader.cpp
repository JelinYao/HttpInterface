#include "stdafx.h"
#include "httpHeader.h"



CHttpHeader::CHttpHeader()
{

}

CHttpHeader::CHttpHeader(const char* pHeader)
	:m_uReturnValue(0)
{
	Revolse(std::string(pHeader));
}

CHttpHeader::CHttpHeader(const std::string& strHeader)
	: m_uReturnValue(0)
{
	Revolse(strHeader);
}

CHttpHeader::~CHttpHeader(void)
{
}

std::string CHttpHeader::GetValue(const std::string& strKey)
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor = m_headers.find(strKey);
	if (itor != m_headers.end())
		strResult = itor->second;
	return strResult;
}

void CHttpHeader::addHeader(const std::string& key, const std::string& value)
{
	if (key.empty() || value.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(key, value));
}

void CHttpHeader::setUserAgent(const std::string& userAgent)
{
	if (userAgent.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(HEADER_USER_AGENT, userAgent));
}

void CHttpHeader::setHost(const std::string& host)
{
	if (host.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(HEADER_HOST, host));
}

void CHttpHeader::setRange(__int64 range)
{
	if (range < 0) {
		return;
	}
	char buffer[64] = { 0 };
	sprintf_s(buffer, "bytes=%i64d-", range);
	m_headers.insert(std::make_pair(HEADER_RANGE, std::string(buffer)));
}

std::string CHttpHeader::toString(HttpRequest type)
{
	//填充默认值
	if (m_httpVersion.empty()) {
		m_httpVersion.assign(default_http_version);
	}
	if (m_headers.find(HEADER_USER_AGENT) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_USER_AGENT, default_user_agent));
	}
	if (m_headers.find(HEADER_CONNECTION) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_USER_AGENT, default_http_version));
	}
	if (m_headers.find(HEADER_ACCEPT) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_ACCEPT, default_accept));
	}
	if (m_headers.find(HEADER_CONNECTION) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_CONNECTION, default_connection));
	}
	if (m_headers.find(HEADER_ACCEPT_LANGUAGE) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_ACCEPT_LANGUAGE, default_language));
	}
	std::string header((type == HttpPost) ? "POST " : "GET ");
	header += m_requestPath;
	header.append(" ");
	header += m_httpVersion;
	header.append(http_newline);
	for (auto itor = m_headers.begin(); itor != m_headers.end(); ++itor) {
		header += itor->first;
		header.append(": ");
		header += itor->second;
		header.append(http_newline);
	}
	header.append(http_newline);
	header.append(http_newline);
	return std::move(header);
}

std::string CHttpHeader::toHttpHeaders()
{
	if (m_headers.find(HEADER_USER_AGENT) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_USER_AGENT, default_user_agent));
	}
	if (m_headers.find(HEADER_CONNECTION) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_USER_AGENT, default_http_version));
	}
	if (m_headers.find(HEADER_ACCEPT) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_ACCEPT, default_accept));
	}
	if (m_headers.find(HEADER_CONNECTION) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_CONNECTION, default_connection));
	}
	if (m_headers.find(HEADER_ACCEPT_LANGUAGE) == m_headers.end()) {
		m_headers.insert(std::make_pair(HEADER_ACCEPT_LANGUAGE, default_language));
	}
	std::string header;
	for (auto itor = m_headers.begin(); itor != m_headers.end(); ++itor) {
		header += itor->first;
		header.append(": ");
		header += itor->second;
		header.append(http_newline);
	}
	return std::move(header);
}

bool CHttpHeader::Revolse(const std::string& strHeader)
{
	int nStartPos = 0, nFindPos = 0, nLineIndex = 0;
	std::string strLine, strKey, strValue;
	do
	{
		nFindPos = strHeader.find("\r\n", nStartPos);
		if (-1 == nFindPos)
			strLine = strHeader.substr(nStartPos, strHeader.size() - nStartPos);
		else
		{
			strLine = strHeader.substr(nStartPos, nFindPos - nStartPos);
			nStartPos = nFindPos + 2;
		}
		if (0 == nLineIndex)//第一行
		{
			m_httpVersion = strLine.substr(0, 8);
			int nSpace1 = strLine.find(" ");
			int nSpace2 = strLine.find(" ", nSpace1 + 1);
			m_uReturnValue = atoi(strLine.substr(nSpace1 + 1, nSpace2 - nSpace1 - 1).c_str());
			m_strContent = strLine.substr(nSpace2 + 1, strLine.size() - nSpace2 - 1);
			nLineIndex++;
			continue;
		}
		int nSplit = strLine.find(": ");
		strKey = strLine.substr(0, nSplit);
		strValue = strLine.substr(nSplit + 2, strLine.size() - nSplit - 2);
		std::pair<std::string, std::string> data;
		data.first = strKey;
		data.second = strValue;
		m_headers.insert(std::move(data));
		nLineIndex++;
	} while (nFindPos != -1);
	return true;
}