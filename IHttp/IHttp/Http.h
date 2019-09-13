#pragma once
#include "IHttpInterface.h"


struct HttpParamsData
{
	void *lpparam;
	IHttpCallback *callback;
	HttpInterfaceError errcode;
};