#pragma once
#include "IHttpInterface.h"


class IHttp
{
public:
	IHttp()
		: m_lpParam(NULL)
		, m_error(Hir_Success)
		, m_pCallback(NULL)
	{

	}
protected:
	void*	m_lpParam;
	HttpInterfaceError	m_error;
	IHttpCallback*	m_pCallback;
};