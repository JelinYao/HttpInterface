// IHttp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "WininetHttp.h"
#include "SocketHttp.h"
#include "WinHttp/HttpClient.h"




LIB_FUN	bool CreateInstance( IHttpBase** pBase, HttpFlag flag )
{
	IHttpBase* pInst = NULL;
	switch( flag )
	{
	case Hf_Socket:
		pInst = new CHttpSocket();
		break;
	case Hf_WinInet:
		pInst = new CWininetHttp();
		break;
	case Hf_WinHttp:
		pInst = new CWinHttp();
		break;
	}
	*pBase = pInst;
	return pInst != NULL;
}

