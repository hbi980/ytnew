#include "../../YTPublic/YTInclude/YTUnit/YTDll.h"
#include <process.h>

CYTDll::CYTDll(void)
{
	m_handle = NULL;
}

CYTDll::~CYTDll(void)
{
	CloseDll();
}

// 加载dll
int CYTDll::LoadDll(const char * dllname)
{
	if(dllname == NULL)
	{
		return -1;
	}
	
	CloseDll();

	m_handle = LoadLibrary(dllname);
	if(m_handle == NULL)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// 关闭dll
void CYTDll::CloseDll(void)
{
	if(m_handle != NULL)
	{
		::FreeLibrary(m_handle);
		m_handle = NULL;
	}
}

// 获取函数地址
void * CYTDll::GetFuncAddr(const char * funcname)
{
	if(m_handle==NULL || funcname==NULL)
	{
		return NULL;
	}

	return ::GetProcAddress(m_handle, funcname);
}