// IHttp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "WininetHttp.h"
#include "SocketHttp.h"
#include "WinHttp/HttpClient.h"




LIB_FUN	bool CreateInstance( IHttpBase** pBase, InterfaceType flag )
{
	IHttpBase* pInst = NULL;
	switch (flag)
	{
	case TypeSocket:
		pInst = new CHttpSocket();
		break;
	case TypeWinInet:
		pInst = new CWininetHttp();
		break;
	case TypeWinHttp:
		pInst = new CWinHttp();
		break;
	}
	*pBase = pInst;
	return pInst != NULL;
}

