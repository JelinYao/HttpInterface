#include "StdAfx.h"
#include "WininetHttp.h"
#include <Wininet.h>
#pragma comment(lib, "Wininet")
#include "Common.h"


CWininetHttp::CWininetHttp(void)
: m_hSession(NULL)
, m_hConnect(NULL)
, m_hRequest(NULL)
{
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
			throw Hir_ParamErr;
		Release();
		m_hSession	= InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession )
			throw Hir_InitErr;
		INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
		string strHostName, strPageName;
		MyParseUrlA(lpUrl, strHostName, strPageName, port);
		m_hConnect=InternetConnectA(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect )
			throw Hir_ConnectErr;
		char* pRequestType = NULL;
		if ( Hr_Get == type )
			pRequestType = "GET";
		else
			pRequestType = "POST";
		m_hRequest = HttpOpenRequestA(m_hConnect, pRequestType, strPageName.c_str(), "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == m_hRequest )
			throw Hir_InitErr;
		BOOL bRet = FALSE;
		DWORD dwHeaderSize = (NULL==lpHeader)?0:strlen(lpHeader);
		if ( Hr_Get == type )
			bRet = HttpSendRequestA(m_hRequest, lpHeader, dwHeaderSize, NULL, 0);
		else
		{
			DWORD dwSize = (lpPostData==NULL)?0:strlen(lpPostData);
			bRet = HttpSendRequestA(m_hRequest, lpHeader, dwHeaderSize, (LPVOID)lpPostData, dwSize);
		}
		if ( !bRet )
			throw Hir_SendErr;
		char szBuffer[READ_BUFFER_SIZE+1]={0};
		DWORD dwReadSize = READ_BUFFER_SIZE;
		if ( !HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL) )
			throw Hir_QueryErr;
		if ( NULL != strstr(szBuffer, "404") )
			throw Hir_404;
		while( true )
		{
			bRet=InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			szBuffer[dwReadSize] = '\0';
			strRet.append(szBuffer);
		}
	}
	catch(HttpInterfaceError error)
	{
		m_error = error;
	}
	return strRet;
}

string CWininetHttp::Request( LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData/* = NULL*/, LPCWSTR lpHeader/*=NULL*/ )
{
	string strRet;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw Hir_ParamErr;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession )
			throw Hir_InitErr;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect )
			throw Hir_ConnectErr;
		wchar_t* pRequestType = NULL;
		if ( Hr_Get == type )
			pRequestType = L"GET";
		else
			pRequestType = L"POST";
		m_hRequest = HttpOpenRequestW(m_hConnect, pRequestType, strPageName.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == m_hRequest )
			throw Hir_InitErr;
		BOOL bRet = FALSE;
		DWORD dwHeaderSize = (NULL==lpHeader)?0:wcslen(lpHeader);
		if ( Hr_Get == type )
			bRet = HttpSendRequestW(m_hRequest, lpHeader, dwHeaderSize, NULL, 0);
		else
		{
			DWORD dwSize = (lpPostData==NULL)?0:strlen(lpPostData);
			bRet = HttpSendRequestW(m_hRequest, lpHeader, dwHeaderSize, (LPVOID)lpPostData, dwSize);
		}
		if ( !bRet )
			throw Hir_SendErr;
		char szBuffer[READ_BUFFER_SIZE+1]={0};
		DWORD dwReadSize = READ_BUFFER_SIZE;
		if ( !HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL) )
			throw Hir_QueryErr;
		if ( NULL != strstr(szBuffer, "404") )
			throw Hir_404;
		while( true )
		{
			bRet=InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			szBuffer[dwReadSize]='\0';
			strRet.append(szBuffer);
		}
	}
	catch(HttpInterfaceError error)
	{
		m_error = error;
	}
	return strRet;
}

bool CWininetHttp::DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)
{
	bool bResult = false;
	BYTE* pBuffer = NULL;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw Hir_IllegalUrl;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession ) throw Hir_InitErr;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect ) throw Hir_ConnectErr;
		m_hRequest = HttpOpenRequestW(m_hConnect, L"GET", strPageName.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == m_hRequest ) throw Hir_InitErr;
		BOOL bRet = HttpSendRequestW(m_hRequest, NULL, 0, NULL, 0);
		if ( !bRet ) throw Hir_SendErr;
		char szBuffer[1024+1] = {0};
		DWORD dwReadSize = 1024;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw Hir_QueryErr;
		string strRetHeader = szBuffer;
		if ( string::npos != strRetHeader.find("404") ) throw Hir_404;
		dwReadSize = 1024;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw Hir_QueryErr;
		szBuffer[dwReadSize] = '\0';
		const double uFileSize = atof(szBuffer);
		int nMallocSize = uFileSize<DOWNLOAD_BUFFER_SIZE? (int)uFileSize:DOWNLOAD_BUFFER_SIZE;
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
		FILE* fp = NULL;
		_wfopen_s(&fp, strSavePath.c_str(), L"wb+");
		if ( NULL == fp ) throw Hir_CreateFileErr;
		double uWriteSize = 0;
		while( true )
		{
			bRet = InternetReadFile(m_hRequest, pBuffer, nMallocSize, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			fwrite(szBuffer, dwReadSize, 1, fp);
			uWriteSize += dwReadSize;
			if ( m_pCallback )
				m_pCallback->OnDownloadCallback(m_lpParam, DS_Loading, uFileSize, uWriteSize);
		}
		fclose(fp);
		if ( uFileSize!=uWriteSize ) throw Hir_DownloadErr;
		if ( m_pCallback )
			m_pCallback->OnDownloadCallback(m_lpParam, DS_Finished, uFileSize, uWriteSize);
		bResult = true;
	}
	catch( HttpInterfaceError error )
	{
		m_error = error;
		if ( m_pCallback )
			m_pCallback->OnDownloadCallback(m_lpParam, DS_Fialed, 0, 0);
	}
	if ( pBuffer )
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	return bResult;
}

bool CWininetHttp::DownloadToMem( LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize )
{
	bool bResult = false;
	BYTE *pDesBuffer = NULL;
	try
	{
		if ( NULL == lpUrl || wcslen(lpUrl) == 0 )
			throw Hir_IllegalUrl;
		Release();
		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession ) throw Hir_InitErr;
		INTERNET_PORT port	= INTERNET_DEFAULT_HTTP_PORT;
		wstring strHostName, strPageName;
		MyParseUrlW(lpUrl, strHostName, strPageName, port);
		m_hConnect = InternetConnectW(m_hSession, strHostName.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect ) throw Hir_ConnectErr;
		m_hRequest = HttpOpenRequestW(m_hConnect, L"GET", strPageName.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == m_hRequest ) throw Hir_InitErr;
		BOOL bRet = HttpSendRequestW(m_hRequest, NULL, 0, NULL, 0);
		if ( !bRet ) throw Hir_SendErr;
		wchar_t szBuffer[1024+1] = {0};
		DWORD dwReadSize = 1024;
		bRet = HttpQueryInfoW(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw Hir_QueryErr;
		wstring strRetHeader(szBuffer);
		if ( string::npos != strRetHeader.find(L"404") ) throw Hir_404;
		dwReadSize = 1024;
		bRet = HttpQueryInfoW(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw Hir_QueryErr;
		szBuffer[dwReadSize] = '\0';
		const int uFileSize = _wtoi(szBuffer);
		if ( uFileSize>DOWNLOAD_BUFFER_SIZE || uFileSize<0 )
			throw Hir_BufferErr;
		pDesBuffer = (BYTE*)malloc(uFileSize);
		int uWriteSize = 0;
		while( true )
		{
			bRet = InternetReadFile(m_hRequest, pDesBuffer, uFileSize, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			uWriteSize += dwReadSize;
		}
		if ( uFileSize != uWriteSize ) 
			throw Hir_DownloadErr;
		*ppBuffer = pDesBuffer;
		*nSize = uWriteSize;
		bResult = true;
	}
	catch( HttpInterfaceError error )
	{
		m_error = error;
		if ( pDesBuffer )
		{
			free(pDesBuffer);
			pDesBuffer = NULL;
		}
	}
	return bResult;
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
