// UseHttp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..//IHttp/IHttpInterface.h"
#ifdef _DEBUG
#pragma comment(lib, "..//bin//Debug//IHttpD")
#else
#pragma comment(lib, "..//bin//Release//IHttp")
#endif
#include <crtdbg.h>
#include <Windows.h>


bool TestWinInet();
bool TestWinHttp();
bool TestSocketHttp();
bool TestDownloadFile();

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
	virtual bool	IsNeedStop()
	{
		//如果需要在外部终止下载，返回true
		return false;//继续下载
	}
};

//extern "C" __declspec(dllimport ) bool CreateInstance(IHttpBase** pBase, HttpFlag flag);


int _tmain(int argc, _TCHAR* argv[])
{
	//TestWinInet();		//测试使用WinInet实现的HTTP接口
	//TestWinHttp();		//测试使用WinHttp实现的HTTP接口
	//TestSocketHttp();		//测试使用Socket实现的HTTP接口
	TestDownloadFile();		//测试下载文件，使用回调接口获取下载进度

	system("pause");
	//打印出内存泄漏信息
 	_CrtDumpMemoryLeaks();
	return 0;
}

bool TestWinInet()
{
	IWininetHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, Hf_WinInet);
	if (!bRet)
	{
		return false;
	}
	char* pMem = NULL;
	int nSize = 0;
	const wchar_t* pUrl = L"http://blog.csdn.net/mfcing";
	string str = pHttp->Request(pUrl, Hr_Get);
	if (str.empty())
	{
		//请求失败
		pHttp->FreeInstance();
		return false;
	}
	if (pHttp->DownloadToMem(pUrl, (void**)&pMem, &nSize))
	{
		//下载到内存中，与下载到本地相比效率更高，不用读写文件
		
		//用完之后一定要释放这块内存空间
		free(pMem);
	}
	else
	{
		//下载失败，获取错误信息
		HttpInterfaceError code = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	pHttp->FreeInstance();
	return true;
}

bool TestWinHttp()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, Hf_WinHttp);
	if (!bRet)
	{
		return false;
	}
	const char* pUrl = "www.haoso.com";
	string strHtml = pHttp->Request(pUrl, Hr_Get);
	if (strHtml.empty())
	{
		//请求失败
		pHttp->FreeInstance();
		return false;
	}
	else
	{
		printf("%s html : %s", pUrl, strHtml.c_str());
	}
	pHttp->FreeInstance();
	return true;
}

bool TestSocketHttp()
{
	ISocketHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, Hf_Socket);
	if (!bRet)
	{
		return false;
	}
	const wchar_t* pUrl = L"www.sogou.com";
	char* pHtml = NULL;
	int nSize = 0;
	if (!pHttp->DownloadToMem(pUrl, (void**)&pHtml, &nSize))
	{
		//下载失败
		pHttp->FreeInstance();
		return false;
	}
	printf("html: %s\n", pHtml);
	//释放内存空间
	free(pHtml);
	pHttp->FreeInstance();
	return true;
}

bool TestDownloadFile()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, Hf_WinHttp);
	if (!bRet)
	{
		return false;
	}
	CMyCallback cb;
	pHttp->SetDownloadCallback(&cb, NULL);
	const wchar_t* pUrl = L"http://sw.bos.baidu.com/sw-search-sp/software/97e90d6bfca7b/WeChat_2.2.0.37_Setup.exe";
	const wchar_t* pSavePath = L"c:\\down.exe";
	if (!pHttp->DownloadFile(pUrl, pSavePath))
	{
		//下载失败
		pHttp->FreeInstance();
		return false;
	}
	pHttp->FreeInstance();
	return true;
}