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
	Hr_Post,
	Hr_Get,
};
//枚举下载状态
enum DownloadState
{
	DS_Loading = 0,
	DS_Fialed,
	DS_Finished,
};

/******************************************************
*定义错误信息
*
******************************************************/
enum HttpInterfaceError
{
	Hir_Success = 0,		//成功
	Hir_InitErr,			//初始化失败
	Hir_ConnectErr,			//连接HTTP服务器失败
	Hir_SendErr,			//发送请求失败
	Hir_QueryErr,			//查询HTTP请求头失败
	Hir_404,				//页面不存在
	Hir_IllegalUrl,			//无效的URL
	Hir_CreateFileErr,		//创建文件失败
	Hir_DownloadErr,		//下载失败
	Hir_QueryIPErr,			//获取域名对应的地址失败
	Hir_SocketErr,			//套接字错误
	Hir_UserCancel,			//用户取消下载
	Hir_BufferErr,			//文件太大，缓冲区不足
	Hir_HeaderErr,			//HTTP请求头错误
	Hir_ParamErr,			//参数错误，空指针，空字符……
	Hir_UnknowErr,

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
	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam)				= 0;
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)							= 0;
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)		= 0;
	virtual void	FreeInstance()															= 0;
	virtual HttpInterfaceError GetErrorCode()												= 0;
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
#define EXPORT_LIB
#define LIB_DLL
#ifdef EXPORT_LIB//导出库
	#ifdef LIB_DLL
		#define LIB_FUN extern "C" __declspec(dllexport)
	#else
		#define LIB_FUN
	#endif
#else//引用库
	#ifdef LID_DLL
		#define LIB_FUN extern "C" __declspec(dllimport)
	#else
		#define LIB_FUN
	#endif
#endif

/***********************************************************
*声明导出函数部分
*
************************************************************
*/

enum HttpFlag
{
	Hf_Socket = 0,
	Hf_WinInet,
	Hf_WinHttp,
};

LIB_FUN	bool CreateInstance(IHttpBase** pBase, HttpFlag flag);



