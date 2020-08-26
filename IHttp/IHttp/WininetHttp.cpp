#include "StdAfx.h"
#include "WininetHttp.h"
#include <Wininet.h>
#pragma comment(lib, "Wininet")
#include "Common.h"


CWininetHttp::CWininetHttp(void)
: m_hSession(NULL)
, m_hConnect(NULL)
, m_hRequest(NULL)
, m_bHttps(false)
{
	memset(&m_paramsData, 0, sizeof(HttpParamsData));
}

CWininetHttp::~CWininetHttp(void)
{
	Release();
}

string CWininetHttp::Request( LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData/* = NULL*/, LPCSTR lpHeader/*=NULL*/)
{
	string strRet;
	try
	{
		if ( lpUrl == NULL || strlen(lpUrl) == 0 )
			throw HttpErrorParam;
		Release();
		m_hSession	= InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession )
			throw HttpErrorInit;
		INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
		string strHostName, strPageName;
		MyParseUrlA(lpUrl, strHostName, strPageName, port);
		if (port == INTERNET_DEFAULT_HTTPS_PORT)
			m_bHttps = true;
		m_hConnect = InternetConnectA(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect )
			throw HttpErrorConnect;
		DWORD dwFlags = INTERNET_FLAG_RELOAD;
		if (m_bHttps)
			dwFlags |= (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		m_hRequest = HttpOpenRequestA(m_hConnect, (type == HttpGet) ? "GET" : "POST", strPageName.c_str(), "HTTP/1.1", NULL, NULL, dwFlags, NULL);
		if ( NULL == m_hRequest )
			throw HttpErrorInit;
		BOOL bRet = FALSE;
		DWORD dwHeaderSize = (NULL == lpHeader) ? 0 : strlen(lpHeader);
		DWORD dwSize = (lpPostData == NULL) ? 0 : strlen(lpPostData); 
		std::string httpHeaders = m_header.toHttpHeaders();
		HttpAddRequestHeadersA(m_hRequest, httpHeaders.c_str(), httpHeaders.size(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
		bRet = HttpSendRequestA(m_hRequest, lpHeader, dwHeaderSize, (LPVOID)lpPostData, dwSize);
		if ( !bRet )
			throw HttpErrorSend;
		char szBuffer[READ_BUFFER_SIZE + 1] = { 0 };
		DWORD dwReadSize = READ_BUFFER_SIZE;
		if ( !HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL) )
			throw HttpErrorQuery;
		if ( NULL != strstr(szBuffer, "404") )
			throw HttpError404;
		while( true )
		{
			bRet = InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
			if (!bRet || (0 == dwReadSize))
				break;
			szBuffer[dwReadSize] = '\0';
			strRet.append(szBuffer, dwReadSize);
		}
	}
	catch(HttpInterfaceError error)
	{
		DWORD code = GetLastError();
		m_paramsData.errcode = error;
	}
	return strRet;
}

string CWininetHttp::Request( LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData/* = NULL*/, LPCWSTR lpHeader/*=NULL*/ )
{
	string strRet;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw HttpErrorParam;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession )
			throw HttpErrorInit;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		if (port == INTERNET_DEFAULT_HTTPS_PORT)
			m_bHttps = true;
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect )
			throw HttpErrorConnect;
		DWORD dwFlags = INTERNET_FLAG_RELOAD;
		if (m_bHttps)
			dwFlags |= (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		m_hRequest = HttpOpenRequestW(m_hConnect, (type == HttpGet) ? L"GET" : L"POST", strPageName.c_str(), L"HTTP/1.1", NULL, NULL, dwFlags, NULL);
		if ( NULL == m_hRequest )
			throw HttpErrorInit;
		BOOL bRet = FALSE;
		DWORD dwHeaderSize = (NULL == lpHeader) ? 0 : wcslen(lpHeader);
		DWORD dwSize = (lpPostData == NULL) ? 0 : strlen(lpPostData);
		std::string httpHeaders = m_header.toHttpHeaders();
		HttpAddRequestHeadersA(m_hRequest, httpHeaders.c_str(), httpHeaders.size(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
		bRet = HttpSendRequestW(m_hRequest, lpHeader, dwHeaderSize, (LPVOID)lpPostData, dwSize);
		if (!bRet)
			throw HttpErrorSend;
		char szBuffer[READ_BUFFER_SIZE+1]={0};
		DWORD dwReadSize = READ_BUFFER_SIZE;
		if (!HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL))
			throw HttpErrorQuery;
		if (NULL != strstr(szBuffer, "404"))
			throw HttpError404;
		while (true)
		{
			bRet = InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
			if (!bRet || (0 == dwReadSize))
				break;
			szBuffer[dwReadSize] = '\0';
			strRet.append(szBuffer, dwReadSize);
		}
	}
	catch(HttpInterfaceError error)
	{
		m_paramsData.errcode = error;
	}
	return strRet;
}

bool CWininetHttp::DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)
{
	bool bResult = false;
	BYTE* pBuffer = NULL;
	FILE* fp = NULL;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw HttpErrorIllegalUrl;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession ) throw HttpErrorInit;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		if(port == INTERNET_DEFAULT_HTTPS_PORT)
			m_bHttps = true;
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect ) throw HttpErrorConnect;
		//INTERNET_FLAG_IGNORE_CERT_CN_INVALID：忽略因服务器的证书主机名与请求的主机名不匹配所导致的错误。
		//INTERNET_FLAG_IGNORE_CERT_DATE_INVALID：忽略由已失效的服务器证书导致的错误。
		DWORD dwFlags = INTERNET_FLAG_RELOAD;
		if(m_bHttps)
			dwFlags |= (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		m_hRequest = HttpOpenRequestW(m_hConnect, L"GET", strPageName.c_str(), L"HTTP/1.1", NULL, NULL, dwFlags, NULL);
		if ( NULL == m_hRequest ) throw HttpErrorInit;
		std::string httpHeaders = m_header.toHttpHeaders();
		HttpAddRequestHeadersA(m_hRequest, httpHeaders.c_str(), httpHeaders.size(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
		BOOL bRet = HttpSendRequestW(m_hRequest, NULL, 0, NULL, 0);
		if ( !bRet ) throw HttpErrorSend;
		char szBuffer[1024+1] = {0};
		DWORD dwReadSize = 1024;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw HttpErrorQuery;
		string strRetHeader = szBuffer;
		if ( string::npos != strRetHeader.find("404") ) throw HttpError404;
		dwReadSize = 1024;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw HttpErrorQuery;
		szBuffer[dwReadSize] = '\0';
		const double uFileSize = atof(szBuffer);
		int nMallocSize = uFileSize< READ_BUFFER_SIZE ? (int)uFileSize: READ_BUFFER_SIZE;
		pBuffer = (BYTE*)malloc(nMallocSize);
		int nFindPos = 0;
		wstring strSavePath(lpFilePath);
		while( wstring::npos != (nFindPos=strSavePath.find(L"\\", nFindPos)) )
		{
			wstring strChildPath=strSavePath.substr(0, nFindPos);
			if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(strChildPath.c_str())) 
				CreateDirectory(strChildPath.c_str(), NULL);
			nFindPos++;
		}
		_wfopen_s(&fp, strSavePath.c_str(), L"wb+");
		if ( NULL == fp ) throw HttpErrorCreateFile;
		double uWriteSize = 0;
		while( true )
		{
			bRet = InternetReadFile(m_hRequest, pBuffer, nMallocSize, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			size_t nWrite = fwrite(pBuffer, dwReadSize, 1, fp);
			if (nWrite == 0)
				throw HttpErrorWriteFile;
			uWriteSize += dwReadSize;
			if (m_paramsData.callback)
				m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpLoading, uFileSize, uWriteSize);
		}
		fclose(fp);
		if ( uFileSize!=uWriteSize ) throw HttpErrorDownload;
		if (m_paramsData.callback)
			m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFinished, uFileSize, uWriteSize);
		bResult = true;
	}
	catch( HttpInterfaceError error )
	{
		m_paramsData.errcode = error;
		if (m_paramsData.callback)
			m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFialed, 0, 0);
	}
	if (pBuffer)
		free(pBuffer);
	if (fp)
		fclose(fp);
	return bResult;
}

bool CWininetHttp::DownloadToMem( LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize )
{
	bool bResult = false;
	BYTE *pDesBuffer = NULL;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw HttpErrorIllegalUrl;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession ) throw HttpErrorInit;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		if (port == INTERNET_DEFAULT_HTTPS_PORT)
			m_bHttps = true;
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect ) throw HttpErrorConnect;
		DWORD dwFlags = INTERNET_FLAG_RELOAD;
		if (m_bHttps)
			dwFlags |= (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		m_hRequest = HttpOpenRequestW(m_hConnect, L"GET", strPageName.c_str(), L"HTTP/1.1", NULL, NULL, dwFlags, NULL);
		if ( NULL == m_hRequest ) throw HttpErrorInit;
		std::string httpHeaders = m_header.toHttpHeaders();
		HttpAddRequestHeadersA(m_hRequest, httpHeaders.c_str(), httpHeaders.size(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
		BOOL bRet = HttpSendRequestW(m_hRequest, NULL, 0, NULL, 0);
		if ( !bRet ) throw HttpErrorSend;
		wchar_t szBuffer[1024+1] = {0};
		DWORD dwReadSize = 1024;
		bRet = HttpQueryInfoW(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw HttpErrorQuery;
		wstring strRetHeader(szBuffer);
		if (string::npos != strRetHeader.find(L"404")) throw HttpError404;
		dwReadSize = 1024;
		bRet = HttpQueryInfoW(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		bool bHasLength = GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND;
		if (!bRet && bHasLength)
			throw HttpErrorQuery;
		int uFileSize = 0;
		if(bHasLength)
		{
			szBuffer[dwReadSize] = '\0';
			uFileSize = _wtoi(szBuffer);
			if (uFileSize > DOWNLOAD_BUFFER_SIZE || uFileSize < 0)
				throw HttpErrorBuffer;//文件大小超过预设最大值，不建议下载到内存
		}
		else
		{
			uFileSize = DOWNLOAD_BUFFER_SIZE;
		}
		pDesBuffer = (BYTE*)malloc(uFileSize);
		int uWriteSize = 0;
		while( true )
		{
			bRet = InternetReadFile(m_hRequest, pDesBuffer + uWriteSize, uFileSize, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			uWriteSize += dwReadSize;
		}
		if (bHasLength && uFileSize != uWriteSize)
			throw HttpErrorDownload;
		*ppBuffer = pDesBuffer;
		*nSize = uWriteSize;
		bResult = true;
	}
	catch( HttpInterfaceError error )
	{
		m_paramsData.errcode = error;
		if ( pDesBuffer )
		{
			free(pDesBuffer);
			pDesBuffer = NULL;
		}
	}
	return bResult;
}

void CWininetHttp::SetDownloadCallback(IHttpCallback* pCallback, void* pParam)
{
	m_paramsData.callback = pCallback;
	m_paramsData.lpparam = pParam;
}

void CWininetHttp::AddHeader(LPCSTR key, LPCSTR value)
{
	if (isEmptyString(key) || isEmptyString(value)) {
		return;
	}
	m_header.addHeader(std::string(key), std::string(value));
}

void CWininetHttp::ReleaseHandle( HINTERNET& hInternet )
{
	if (hInternet) 
	{ 
		InternetCloseHandle(hInternet); 
		hInternet = NULL; 
	}
}

void CWininetHttp::Release()
{
	ReleaseHandle(m_hRequest); 
	ReleaseHandle(m_hConnect); 
	ReleaseHandle(m_hSession);
}
