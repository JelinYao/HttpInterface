// UseHttp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..//IHttp/IHttpInterface.h"
#ifdef _DEBUG
#pragma comment(lib, "../lib/IHttpD.lib")
#else
#pragma comment(lib, "../lib/IHttp.lib")
#endif
#include <crtdbg.h>
#include <Windows.h>


bool TestWinInet();
bool TestWinHttp();
bool TestSocketHttp();
bool TestDownloadFile();

//下载文件的回调类，显示下载进度&控制下载
class CMyCallback
	: public IHttpCallback
{
public:
	virtual void OnDownloadCallback(void* pParam, DownloadState state, double nTotalSize, double nLoadSize)
	{
		if (nTotalSize>0)
		{
			int nPercent = (int)( 100*(nLoadSize/nTotalSize) );
			printf("下载进度：%d%%\n", nPercent);
		}
	}
	virtual bool IsNeedStop()
	{
		//如果需要在外部终止下载，返回true
		return false;//继续下载
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//TestWinInet();		//测试使用WinInet实现的HTTP接口
	//TestWinHttp();		//测试使用WinHttp实现的HTTP接口
	TestSocketHttp();		//测试使用Socket实现的HTTP接口
	//TestDownloadFile();	//测试下载文件，使用回调接口获取下载进度

	system("pause");
	//打印出内存泄漏信息
 	_CrtDumpMemoryLeaks();
	return 0;
}

bool TestWinInet()
{
	IWininetHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinInet);
	if (!bRet)
	{
		return false;
	}
	char* pMem = NULL;
	int nSize = 0;
	const wchar_t* pUrl = L"https://blog.csdn.net/mfcing";
	//添加自定义http头信息
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	string str = pHttp->Request(pUrl, HttpGet);
	if (str.empty())
	{
		//请求失败
		pHttp->FreeInstance();
		return false;
	}
	if (pHttp->DownloadToMem(pUrl, (void**)&pMem, &nSize))
	{//下载到内存中，与下载到本地相比效率更高，不用读写文件（仅适用于文件小文件）
		
		//用完之后一定要释放这块内存空间
		free(pMem);
	}
	else
	{
		//下载失败，获取错误信息
		DWORD dwCode = GetLastError();
		HttpInterfaceError code = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	//测试post请求
	std::string ret = pHttp->Request("https://chat.jelinyao.cn/postTest", HttpPost, "{\"name\":\"Jelin\",\"address\":\"Shanghai\"}");
	pHttp->FreeInstance();
	return true;
}

bool TestWinHttp()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinHttp);
	if (!bRet)
	{
		return false;
	}
	const char* pUrl = "https://www.qq.com";
	//添加自定义http头信息
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	string strHtml = pHttp->Request(pUrl, HttpGet);
	if (strHtml.empty())
	{
		//请求失败
		pHttp->FreeInstance();
		return false;
	}
	else
	{
		printf("%s html : %s\n", pUrl, strHtml.c_str());
	}
	//测试post请求
	std::string ret = pHttp->Request("https://chat.jelinyao.cn/postTest", HttpPost, "{\"name\":\"Jelin\",\"address\":\"Shanghai\"}");
	pHttp->FreeInstance();
	return true;
}

bool TestSocketHttp()
{
	//使用winsock之前需要进行初始化
	InitWSASocket();
	ISocketHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeSocket);
	if (!bRet)
	{
		UninitWSASocket();
		return false;
	}
	const wchar_t* pUrl = L"http://www.hbsrsksy.cn/";
	char* pHtml = NULL;
	int nSize = 0;
	//添加自定义http头信息
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	//下载网页内容到内存中，该内存由malloc动态申请，使用后需要手动释放
	if (!pHttp->DownloadToMem(pUrl, (void**)&pHtml, &nSize))
	{
		//下载失败
		pHttp->FreeInstance();
		UninitWSASocket();
		return false;
	}
	printf("html: %s\n", pHtml);
	//释放内存空间
	free(pHtml);
	pHttp->FreeInstance();
	UninitWSASocket();
	return true;
}

bool TestDownloadFile()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinHttp);
	if (!bRet)
	{
		return false;
	}
	CMyCallback cb;
	pHttp->SetDownloadCallback(&cb, NULL);
	const wchar_t* pUrl = L"https://pm.myapp.com/invc/xfspeed/qqsoftmgr/QQSoftDownloader_v1.1_webnew_22127@.exe";
	const wchar_t* pSavePath = L"c:\\down.exe";
	if (!pHttp->DownloadFile(pUrl, pSavePath))
	{
		//下载失败
		DWORD dwCode = GetLastError();
		HttpInterfaceError error = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	pHttp->FreeInstance();
	return true;
}