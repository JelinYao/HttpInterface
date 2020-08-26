/*****************************************************************
*HTTP处理类，主要用于HTTP GET/POST、下载（支持重定向）功能
*Author：	JelinYao
*Date：		2015/2/14 12:11
*Email：	mailto://jelinyao@163.com
*/
/*****************************************************************
*/
#pragma once
#include <stdio.h>
#include <tchar.h>
#include <string>
using std::string;
using std::wstring;


enum HttpRequest
{
	HttpGet = 0,
	HttpPost,
};
//枚举下载状态
enum DownloadState
{
	HttpLoading = 0,
	HttpFialed,
	HttpFinished,
};

/******************************************************
*定义错误信息
*
******************************************************/
enum HttpInterfaceError
{
	HttpErrorSuccess = 0,		//成功
	HttpErrorInit,				//初始化失败
	HttpErrorConnect,			//连接HTTP服务器失败
	HttpErrorSend,				//发送请求失败
	HttpErrorQuery,				//查询HTTP请求头失败
	HttpError404,				//页面不存在
	HttpErrorIllegalUrl,		//无效的URL
	HttpErrorCreateFile,		//创建文件失败
	HttpErrorDownload,			//下载失败
	HttpErrorQueryIP,			//获取域名对应的地址失败
	HttpErrorSocket,			//套接字错误
	HttpErrorUserCancel,		//用户取消下载
	HttpErrorBuffer,			//文件太大，缓冲区不足
	HttpErrorHeader,			//HTTP请求头错误
	HttpErrorParam,				//参数错误，空指针，空字符……
	HttpErrorWriteFile,			//写入文件失败
	HttpErrorUnknow,			//未知错误

};




//下载的回调
class IHttpCallback
{
public:
	virtual void	OnDownloadCallback(void* pParam, DownloadState state, double nTotalSize, double nLoadSize)		= 0;
	virtual bool	IsNeedStop()																					= 0;
};

class IHttpBase
{
public:
	virtual void SetDownloadCallback(IHttpCallback* pCallback, void* pParam)= 0;
	virtual bool DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)= 0;
	virtual bool DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)= 0;
	virtual void FreeInstance()= 0;
	virtual HttpInterfaceError GetErrorCode()= 0;
	virtual void AddHeader(LPCSTR key, LPCSTR value) = 0;//添加HTTP请求头
};

////////////////////////////////////////////////////////////////////////////////////
//HTTP请求接口类
class IWininetHttp
	:public IHttpBase
{
public:
	//HTTP请求功能
	virtual string	Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader = NULL)			= 0;
	virtual string	Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCWSTR lpHeader = NULL)			= 0;
};


///////////////////////////////////////////////////////////////////////////////////////
//HTTP socket类

class ISocketHttp
	:public IHttpBase
{
public:
	virtual LPCWSTR	GetIpAddr()const	= 0;
};

///////////////////////////////////////////////////////////////////////////////////////
//WinHttp类
class IWinHttp
	: public IWininetHttp
{
public:
	//设置超时时间，单位：毫秒
	virtual void	SetTimeOut(int dwConnectTime,  int dwSendTime, int dwRecvTime)										= 0;		
};



/////////////////////////////////////////////////////////////////////////////////
//DLL的导出函数声明
#ifdef _USRDLL//导出库
		#define LIB_FUNCTION extern "C" __declspec(dllexport)
	#else
		#define LIB_FUNCTION extern "C" __declspec(dllimport)
#endif
/***********************************************************
*声明导出函数部分
*
************************************************************
*/

enum InterfaceType
{
	TypeSocket = 0,
	TypeWinInet,
	TypeWinHttp,
};

LIB_FUNCTION bool CreateInstance(IHttpBase** pBase, InterfaceType flag);

//初始化Windows系统网络库：Winsocket2
LIB_FUNCTION void InitWSASocket();

//卸载系统网络库
LIB_FUNCTION void UninitWSASocket();



