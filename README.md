# HttpInterface
Windows上C++封装的HTTP库，包含三种实现模式（WinInet、WinHttp、socket）

主要实现了HTTP的get\post方法，下载到内存、下载到本地文件，回调下载进度等接口

测试程序中展现了常用的几个方法。

接口类声明：

class IHttpBase
{
public:
	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam)= 0;
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)= 0;
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)= 0;
	virtual void	FreeInstance()= 0;
	virtual HttpInterfaceError GetErrorCode()= 0;
};

////////////////////////////////////////////////////////////////////////////////////
//HTTP请求接口类
class IWininetHttp
	:public IHttpBase
{
public:
	//HTTP请求功能
	virtual string	Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader = NULL)= 0;
	virtual string	Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCWSTR lpHeader = NULL)= 0;
};


///////////////////////////////////////////////////////////////////////////////////////
//HTTP socket类


class ISocketHttp
	:public IHttpBase
{
public:
	virtual LPCWSTR	GetIpAddr()const= 0;
};

///////////////////////////////////////////////////////////////////////////////////////
//WinHttp类
class IWinHttp
	: public IWininetHttp
{
public:
	//设置超时时间，单位：毫秒
	virtual void	SetTimeOut(int dwConnectTime,  int dwSendTime, int dwRecvTime)= 0;		
};


能力有限，有bug欢迎指针 email:jelinyao@163.com
