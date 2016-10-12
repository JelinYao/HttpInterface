#include "stdafx.h"
#include "SocketHttp.h"
#pragma comment (lib, "ws2_32")
#include <Shlwapi.h>




/////////////////////////////////////////////////////////////////////////

CHttpSocket::CHttpHeader::CHttpHeader( const char* pHeader )
:m_uReturnValue(0)
{
	Revolse(std::string(pHeader));
}

CHttpSocket::CHttpHeader::CHttpHeader( const std::string& strHeader )
:m_uReturnValue(0)
{
	Revolse(strHeader);
}

CHttpSocket::CHttpHeader::~CHttpHeader(void)
{
}

std::string CHttpSocket::CHttpHeader::GetValue( const std::string& strKey )
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor=m_ValueMap.find(strKey);
	if ( itor != m_ValueMap.end() )
		strResult=itor->second;
	return strResult;
}

bool CHttpSocket::CHttpHeader::Revolse( const std::string& strHeader )
{
	int nStartPos=0, nFindPos=0, nLineIndex=0;
	std::string strLine, strKey, strValue;
	do
	{
		nFindPos=strHeader.find("\r\n", nStartPos);
		if ( -1 == nFindPos )
			strLine=strHeader.substr(nStartPos, strHeader.size()-nStartPos);
		else
		{
			strLine=strHeader.substr(nStartPos, nFindPos-nStartPos);
			nStartPos=nFindPos+2;
		}
		if ( 0 == nLineIndex )//第一行
		{
			strncpy_s(m_szHttpVersion, strLine.c_str(), 8);
			m_szHttpVersion[8]='\0';
			if ( strcmp(m_szHttpVersion, "HTTP/1.1") != 0 )
				return false;
			int nSpace1=strLine.find(" ");
			int nSpace2=strLine.find(" ", nSpace1+1);
			m_uReturnValue=atoi(strLine.substr(nSpace1+1, nSpace2-nSpace1-1).c_str());
			m_strContent=strLine.substr(nSpace2+1, strLine.size()-nSpace2-1);
			nLineIndex++;
			continue;
		}
		int nSplit=strLine.find(": ");
		strKey=strLine.substr(0, nSplit);
		strValue=strLine.substr(nSplit+2, strLine.size()-nSplit-2);
		std::pair<std::string ,std::string> data;
		data.first=strKey;
		data.second=strValue;
		m_ValueMap.insert(data);
		nLineIndex++;
	}
	while(nFindPos!=-1);
	return true;
}

/////////////////////////////////////////////////////////////////////////


CHttpSocket::CHttpSocket()
	: m_socket(INVALID_SOCKET)
{
	WSADATA data;
	WSAStartup(0x0202, &data);
}

CHttpSocket::~CHttpSocket()
{
	if ( INVALID_SOCKET != m_socket )
		closesocket(m_socket);
	WSACleanup();
}

bool CHttpSocket::InitSocket( const string& strHostName, const WORD sPort )
{
	bool bResult = false;
	try
	{
		HOSTENT* pHostent=gethostbyname(strHostName.c_str());
		if ( NULL == pHostent )
			throw Hir_QueryIPErr;
		char szIP[16]={0};
		sprintf_s(szIP, "%d.%d.%d.%d",
			pHostent->h_addr_list[0][0]&0x00ff,
			pHostent->h_addr_list[0][1]&0x00ff,
			pHostent->h_addr_list[0][2]&0x00ff,
			pHostent->h_addr_list[0][3]&0x00ff);
		m_strIpAddr = A2U(szIP);
		if ( INVALID_SOCKET != m_socket )
			closesocket(m_socket);
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if ( INVALID_SOCKET == m_socket )
			throw Hir_SocketErr;
		int nSec=1000*10;//10秒内没有数据则说明网络断开
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nSec, sizeof(int));
		sockaddr_in		addrServer;
		addrServer.sin_family=AF_INET;
		addrServer.sin_port=htons(sPort);
		addrServer.sin_addr.S_un.S_addr=inet_addr(szIP);
		if ( SOCKET_ERROR == connect(m_socket, (SOCKADDR*)&addrServer, sizeof(addrServer)) )
			throw Hir_ConnectErr;
		bResult=true;
	}
	catch(HttpInterfaceError error)
	{
		m_error = error;
	}
	catch(...)
	{

	}
	return bResult;
}

bool CHttpSocket::DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)
{
	bool bResult	= false;
	FILE* fp		= NULL;
	BYTE* pBuffer	= NULL;
	try
	{
		wstring strHostName, strPage;
		u_short uPort=80;
		MyParseUrlW(lpUrl, strHostName, strPage, uPort);
		string str = U2A(strHostName);
		if ( ! InitSocket(str, uPort) )
			throw L"";
		HTTP_HERDER header;
		strcpy_s(header.szHostName, str.c_str());
__request:
		InitRequestHeader(header, U2A(strPage).c_str());
		std::string strSend = header.ToString();
		int nRet	= send(m_socket, strSend.c_str(), strSend.size(), 0);
		if ( SOCKET_ERROR == nRet )
			throw Hir_SendErr;
		int		nRecvSize = 0, nWriteSize = 0;
		double	nFileSize=0, nLoadSize=0;
		bool	bFilter=false;//HTTP返回头是否已经被过滤掉
		if ( FileExistW(lpFilePath) )
			DeleteFile(lpFilePath);
		_wfopen_s(&fp, lpFilePath, L"wb+");
		if ( NULL == fp )
			throw Hir_CreateFileErr;
		pBuffer = (BYTE*)malloc(DOWNLOAD_BUFFER_SIZE+1);
		do
		{
			if ( m_pCallback && m_pCallback->IsNeedStop() )
				throw Hir_UserCancel;
			nRecvSize = recv(m_socket, (char*)pBuffer, DOWNLOAD_BUFFER_SIZE, 0);
			if ( SOCKET_ERROR == nRecvSize )
				throw Hir_SocketErr;
			if ( nRecvSize>0 )
			{
				pBuffer[nRecvSize]='\0';
				if ( !bFilter )
				{
					std::string str((char*)pBuffer);
					int nPos=str.find("\r\n\r\n");
					if ( -1 == nPos )
						continue;
					std::string strHeader;
					strHeader.append((char*)pBuffer, nPos);
					CHttpHeader header(strHeader);
					int nHttpValue = header.GetReturnValue();
					if ( 404 == nHttpValue )//文件不存在
					{
						throw Hir_404;
					}
					if ( nHttpValue>300 && nHttpValue<400 )//重定向
					{
						wstring strReLoadUrl = A2U(header.GetValue("location"));
						if ( strReLoadUrl.find(L"http://") != 0 )
						{
							strPage = strReLoadUrl;
							goto __request;
						}
						if ( INVALID_SOCKET != m_socket )
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
					nFileSize	= atof(header.GetValue("Content-Length").c_str());
					nWriteSize  = nRecvSize-nPos-4;
					if ( nWriteSize>0 )
					{
						fwrite(pBuffer+nPos+4, nWriteSize, 1, fp);
						nLoadSize += nRecvSize;
						if ( m_pCallback )
							m_pCallback->OnDownloadCallback(m_lpParam, DS_Loading, nFileSize, nLoadSize);
					}
					if ( nFileSize == nLoadSize )
					{
						if ( m_pCallback )
							m_pCallback->OnDownloadCallback(m_lpParam, DS_Finished, nFileSize, nLoadSize);
						bResult = true;
						break;
					}
					bFilter = true;
					continue;
				}
				fwrite(pBuffer, nRecvSize, 1, fp);
				nLoadSize += nRecvSize;
				if ( m_pCallback )
					m_pCallback->OnDownloadCallback(m_lpParam, DS_Loading, nFileSize, nLoadSize);
				if ( nLoadSize >= nFileSize )
				{
					bResult = true;
					if ( m_pCallback )
						m_pCallback->OnDownloadCallback(m_lpParam, DS_Finished, nFileSize, nLoadSize);
					break;
				}
			}
		}
		while( nRecvSize>0 );
	}
	catch(HttpInterfaceError error)
	{
		m_error = error;
		if ( m_pCallback )
			m_pCallback->OnDownloadCallback(m_lpParam, DS_Fialed, 0, 0);
	}
	catch(...)
	{
		
	}
	if ( pBuffer )
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	if ( fp )
	{
		fclose(fp);
		fp = NULL;
	}
	if ( !bResult )//下载不成功，删除不完整的文件
		DeleteFile(lpFilePath);
	return bResult;
}



bool CHttpSocket::DownloadToMem( LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize )
{
	bool bResult	= false;
	BYTE* pBuffer	= NULL;
	try
	{
		wstring strHostName, strPage;
		u_short uPort = 80;
		MyParseUrlW(lpUrl, strHostName, strPage, uPort);
		string str = U2A(strHostName);
		if ( ! InitSocket(str, uPort) )
			throw Hir_InitErr;
		HTTP_HERDER header;
		strcpy_s(header.szHostName, str.c_str());
__request:
		InitRequestHeader(header, U2A(strPage).c_str());
		std::string strSend = header.ToString();
		int nRet = send(m_socket, strSend.c_str(), strSend.size(), 0);
		if ( SOCKET_ERROR == nRet )
			throw Hir_SendErr;
		int	nRecvSize = 0, nWriteSize = 0;
		int	nFileSize=0, nLoadSize=0;
		bool	bFilter = false;//HTTP返回头是否已经被过滤掉
		const int nBufferSzie = 1024*4;
		char szHeader[nBufferSzie+1];
		while( true )
		{
			nRecvSize = recv(m_socket, szHeader, nBufferSzie, 0);
			if ( SOCKET_ERROR == nRecvSize )
				throw Hir_SocketErr;
			if ( nRecvSize == 0 )
				break;
			if ( !bFilter )
			{
				std::string str(szHeader, nRecvSize);
				int nPos=str.find("\r\n\r\n");
				if ( -1 == nPos )
					throw Hir_HeaderErr;
				std::string strHeader(szHeader, nPos);
				CHttpHeader header(strHeader);
				int nHttpValue = header.GetReturnValue();
				if ( 404 == nHttpValue )//文件不存在
				{
					throw Hir_404;
				}
				if ( nHttpValue>300 && nHttpValue<400 )//重定向
				{
					wstring strReLoadUrl = A2U(header.GetValue("location"));
					if ( strReLoadUrl.find(L"http://") != 0 )
					{
						strPage = strReLoadUrl;
						goto __request;
					}
					if ( INVALID_SOCKET != m_socket )
					{
						closesocket(m_socket);
						m_socket = INVALID_SOCKET;
					}
					return DownloadToMem(strReLoadUrl.c_str(), ppBuffer, nSize);
				}
				nFileSize = atoi(header.GetValue("Content-Length").c_str());
				*nSize = nFileSize;
				if ( nFileSize>DOWNLOAD_BUFFER_SIZE || nFileSize<=0 )
					throw Hir_BufferErr;
				pBuffer = (BYTE*)malloc(nFileSize);
				nWriteSize  = nRecvSize-nPos-4;
				if ( nWriteSize>0 )
				{
					memcpy(pBuffer, szHeader+nPos+4, nWriteSize);
					nLoadSize += nWriteSize;
				}
				if ( nFileSize == nLoadSize )
				{
					bResult = true;
					break;
				}
				bFilter = true;
				continue;
			}
			memcpy(pBuffer, szHeader, nRecvSize);
			nLoadSize += nRecvSize;
			if ( nLoadSize >= nFileSize )
			{
				bResult = true;
				break;
			}
		}
	}
	catch(HttpInterfaceError error)
	{
		m_error = error;
		if ( pBuffer )
		{
			free(pBuffer);
			pBuffer = NULL;
		}
		*ppBuffer = NULL;
		*nSize = 0;
	}
	if ( bResult )
	{
		*ppBuffer = pBuffer;
	}
	return bResult;
}

void CHttpSocket::InitRequestHeader( HTTP_HERDER& header, const char* pRequest, HttpRequest type/*=get*/, LPCSTR pRange/*=NULL*/, LPCSTR pAccept/*="* / *"*/ )
{
	memset(header.szType, 0, 5);
	if ( Hr_Get == type )
		strcpy_s(header.szType, "GET");
	else
		strcpy_s(header.szType, "POST");
	memset(header.szRequest, 0, MAX_PATH);
	strcpy_s(header.szRequest, pRequest);
	memset(header.szAccept, 0, 100);
	strcpy_s(header.szAccept, pAccept);
	memset(header.szRange, 0, 11);
	if ( pRange )
		strcpy_s(header.szRange, pRange);
}

