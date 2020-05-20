#include "stdafx.h"
#include "SocketHttp.h"
#pragma comment (lib, "ws2_32")
#include <Shlwapi.h>
#include "define.h"




/////////////////////////////////////////////////////////////////////////
CHttpSocket::CHttpHeader::CHttpHeader()
{

}

CHttpSocket::CHttpHeader::CHttpHeader(const char* pHeader)
	:m_uReturnValue(0)
{
	Revolse(std::string(pHeader));
}

CHttpSocket::CHttpHeader::CHttpHeader(const std::string& strHeader)
	: m_uReturnValue(0)
{
	Revolse(strHeader);
}

CHttpSocket::CHttpHeader::~CHttpHeader(void)
{
}

std::string CHttpSocket::CHttpHeader::GetValue(const std::string& strKey)
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor = m_headers.find(strKey);
	if (itor != m_headers.end())
		strResult = itor->second;
	return strResult;
}

void CHttpSocket::CHttpHeader::addHeader(const std::string& key, const std::string& value)
{
	if (key.empty() || value.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(key, value));
}

void CHttpSocket::CHttpHeader::setUserAgent(const std::string& userAgent)
{
	if (userAgent.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(HEADER_USER_AGENT, userAgent));
}

void CHttpSocket::CHttpHeader::setHost(const std::string& host)
{
	if (host.empty()) {
		return;
	}
	m_headers.insert(std::make_pair(HEADER_HOST, host));
}

void CHttpSocket::CHttpHeader::setRange(__int64 range)
{
	if (range < 0) {
		return;
	}
	char buffer[64] = { 0 };
	sprintf_s(buffer, "bytes=%i64d-", range);
	m_headers.insert(std::make_pair(HEADER_RANGE, std::string(buffer)));
}

std::string CHttpSocket::CHttpHeader::toString(HttpRequest type)
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
	std::string header((type == HttpPost)?"POST ":"GET ");
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
	return header;
}

bool CHttpSocket::CHttpHeader::Revolse(const std::string& strHeader)
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

/////////////////////////////////////////////////////////////////////////

CHttpSocket::CHttpSocket()
	: m_socket(INVALID_SOCKET)
{
	memset(&m_paramsData, 0, sizeof(HttpParamsData));
}

CHttpSocket::~CHttpSocket()
{
	if (INVALID_SOCKET != m_socket)
		closesocket(m_socket);
}

bool CHttpSocket::InitSocket(const string& strHostName, const WORD sPort)
{
	bool bResult = false;
	try
	{
		//查询域名对应的IP地址
		HOSTENT* pHostent = gethostbyname(strHostName.c_str());
		if (NULL == pHostent)
			throw HttpErrorQueryIP;
		char szIP[16] = { 0 };
		sprintf_s(szIP, "%d.%d.%d.%d",
			pHostent->h_addr_list[0][0] & 0x00ff,
			pHostent->h_addr_list[0][1] & 0x00ff,
			pHostent->h_addr_list[0][2] & 0x00ff,
			pHostent->h_addr_list[0][3] & 0x00ff);
		m_strIpAddr = A2U(szIP);
		if (INVALID_SOCKET != m_socket)
			closesocket(m_socket);
		//连接HTTP服务器
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == m_socket)
			throw HttpErrorSocket;
		int nSec = 1000 * 10;//10秒内没有数据则说明网络断开
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nSec, sizeof(int));
		sockaddr_in		addrServer;
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(sPort);
		addrServer.sin_addr.S_un.S_addr = inet_addr(szIP);
		if (SOCKET_ERROR == connect(m_socket, (SOCKADDR*)&addrServer, sizeof(addrServer)))
			throw HttpErrorConnect;
		bResult = true;
	}
	catch (HttpInterfaceError error)
	{
		m_paramsData.errcode = error;
	}
	catch (...)
	{

	}
	return bResult;
}

bool CHttpSocket::DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)
{
	bool bResult = false;
	FILE* fp = NULL;
	BYTE* pBuffer = NULL;
	try
	{
		wstring strHostName, strPage;
		u_short uPort = 80;
		MyParseUrlW(lpUrl, strHostName, strPage, uPort);
		if (uPort == 443) {
			//抱歉，socket方式暂时不支持HTTPS协议
			return false;
		}
		string host = U2A(strHostName);
		if (!InitSocket(host, uPort))
			throw L"";
		CHttpHeader header;
		header.setHost(host);
		//这里可能会重定向回来
	__request:
		header.setRequestPath(U2A(strPage));
		std::string strSend = header.toString(HttpGet);
		int nRet = send(m_socket, strSend.c_str(), strSend.size(), 0);
		if (SOCKET_ERROR == nRet)
			throw HttpErrorSend;
		int		nRecvSize = 0, nWriteSize = 0;
		double	nFileSize = 0, nLoadSize = 0;
		bool	bFilter = false;//HTTP返回头是否已经被过滤掉
		if (FileExistW(lpFilePath))
			DeleteFile(lpFilePath);
		_wfopen_s(&fp, lpFilePath, L"wb+");
		if (NULL == fp)
			throw HttpErrorCreateFile;
		pBuffer = (BYTE*)malloc(READ_BUFFER_SIZE + 1);
		do
		{
			if (m_paramsData.callback && m_paramsData.callback->IsNeedStop())
				throw HttpErrorUserCancel;
			nRecvSize = recv(m_socket, (char*)pBuffer, READ_BUFFER_SIZE, 0);
			if (SOCKET_ERROR == nRecvSize)
				throw HttpErrorSocket;
			if (nRecvSize>0)
			{
				pBuffer[nRecvSize] = '\0';
				if (!bFilter)
				{
					std::string str((char*)pBuffer);
					int nPos = str.find("\r\n\r\n");
					if (-1 == nPos)
						continue;
					std::string strHeader;
					strHeader.append((char*)pBuffer, nPos);
					CHttpHeader header(strHeader);
					int nHttpValue = header.GetReturnValue();
					if (404 == nHttpValue)//文件不存在
					{
						throw HttpError404;
					}
					if (nHttpValue>300 && nHttpValue<400)//重定向
					{
						wstring strReLoadUrl = A2U(header.GetValue(HEADER_LOCATION));
						if (strReLoadUrl.find(L"http://") != 0)
						{
							strPage = strReLoadUrl;
							goto __request;
						}
						if (INVALID_SOCKET != m_socket)
						{
							closesocket(m_socket);
							m_socket = INVALID_SOCKET;
						}
						//重定向前，清理掉本函数创建的资源
						free(pBuffer);
						pBuffer = NULL;
						fclose(fp);
						fp = NULL;
						return DownloadFile(strReLoadUrl.c_str(), lpFilePath);
					}
					nFileSize = atof(header.GetValue(HEADER_CONTENT_LENGTH).c_str());
					nWriteSize = nRecvSize - nPos - 4;
					if (nWriteSize>0)
					{
						fwrite(pBuffer + nPos + 4, nWriteSize, 1, fp);
						nLoadSize += nRecvSize;
						if (m_paramsData.callback)
							m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpLoading, nFileSize, nLoadSize);
					}
					if (nFileSize == nLoadSize)
					{
						if (m_paramsData.callback)
							m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFinished, nFileSize, nLoadSize);
						bResult = true;
						break;
					}
					bFilter = true;
					continue;
				}
				fwrite(pBuffer, nRecvSize, 1, fp);
				nLoadSize += nRecvSize;
				if (m_paramsData.callback)
					m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpLoading, nFileSize, nLoadSize);
				if (nLoadSize >= nFileSize)
				{
					bResult = true;
					if (m_paramsData.callback)
						m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFinished, nFileSize, nLoadSize);
					break;
				}
			}
		} while (nRecvSize>0);
	}
	catch (HttpInterfaceError error)
	{
		m_paramsData.errcode = error;
		if (m_paramsData.callback)
			m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFialed, 0, 0);
	}
	catch (...)
	{

	}
	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	if (!bResult)//下载不成功，删除不完整的文件
		DeleteFile(lpFilePath);
	return bResult;
}



void CHttpSocket::SetDownloadCallback(IHttpCallback* pCallback, void* pParam)
{
	m_paramsData.callback = pCallback;
	m_paramsData.lpparam = pParam;
}

bool CHttpSocket::DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)
{
	bool bResult = false;
	BYTE *pBuffer = NULL;
	try
	{
		wstring strHostName, strPage;
		u_short uPort = 80;
		MyParseUrlW(lpUrl, strHostName, strPage, uPort);
		if (uPort == 443) {
			//抱歉，socket方式暂时不支持HTTPS协议
			return false;
		}
		string host = U2A(strHostName);
		if (!InitSocket(host, uPort))
			throw HttpErrorInit;
		CHttpHeader header;
		header.setHost(host);
		//这里可能会重定向回来
	__request:
		header.setRequestPath(U2A(strPage));
		std::string strSend = header.toString(HttpGet);
		int nRet = send(m_socket, strSend.c_str(), strSend.size(), 0);
		if (SOCKET_ERROR == nRet)
			throw HttpErrorSend;
		int	nRecvSize = 0, nWriteSize = 0;
		int	nFileSize = 0, nLoadSize = 0;
		bool	bFilter = false;//HTTP返回头是否已经被过滤掉
		const int nBufferSzie = 1024 * 4;
		char szHeader[nBufferSzie + 1];
		while (true)
		{
			nRecvSize = recv(m_socket, szHeader, nBufferSzie, 0);
			if (SOCKET_ERROR == nRecvSize)
				throw HttpErrorSocket;
			if (nRecvSize == 0)
				break;
			if (!bFilter)
			{
				std::string str(szHeader, nRecvSize);
				int nPos = str.find("\r\n\r\n");
				if (-1 == nPos)
					throw HttpErrorHeader;
				std::string strHeader(szHeader, nPos);
				CHttpHeader header(strHeader);
				int nHttpValue = header.GetReturnValue();
				if (404 == nHttpValue)//文件不存在
				{
					throw HttpError404;
				}
				if (nHttpValue>300 && nHttpValue<400)//重定向
				{
					wstring reloadUrl = A2U(header.GetValue(HEADER_LOCATION));
					if (reloadUrl.find(L"http://") != 0 && reloadUrl.find(L"https://") != 0)
					{
						strPage = reloadUrl;
						goto __request;
					}
					if (INVALID_SOCKET != m_socket)
					{
						closesocket(m_socket);
						m_socket = INVALID_SOCKET;
					}
					return DownloadToMem(reloadUrl.c_str(), ppBuffer, nSize);
				}
				nFileSize = atoi(header.GetValue(HEADER_CONTENT_LENGTH).c_str());
				*nSize = nFileSize;
				if (nFileSize>DOWNLOAD_BUFFER_SIZE || nFileSize <= 0)
					throw HttpErrorBuffer;
				pBuffer = (BYTE*)malloc(nFileSize);
				nWriteSize = nRecvSize - nPos - 4;
				if (nWriteSize>0)
				{
					memcpy(pBuffer + nLoadSize, szHeader + nPos + 4, nWriteSize);
					nLoadSize += nWriteSize;
				}
				if (nFileSize == nLoadSize)
				{
					bResult = true;
					break;
				}
				bFilter = true;
				continue;
			}
			memcpy(pBuffer + nLoadSize, szHeader, nRecvSize);
			nLoadSize += nRecvSize;
			if (nLoadSize >= nFileSize)
			{
				bResult = true;
				break;
			}
		}
	}
	catch (HttpInterfaceError error)
	{
		m_paramsData.errcode = error;
		if (pBuffer)
		{
			free(pBuffer);
			pBuffer = NULL;
		}
		*ppBuffer = NULL;
		*nSize = 0;
	}
	if (bResult)
	{
		*ppBuffer = pBuffer;
	}
	return bResult;
}
