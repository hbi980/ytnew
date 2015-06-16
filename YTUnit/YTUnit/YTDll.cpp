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

// ����dll
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

// �ر�dll
void CYTDll::CloseDll(void)
{
	if(m_handle != NULL)
	{
		::FreeLibrary(m_handle);
		m_handle = NULL;
	}
}

// ��ȡ������ַ
void * CYTDll::GetFuncAddr(const char * funcname)
{
	if(m_handle==NULL || funcname==NULL)
	{
		return NULL;
	}

	return ::GetProcAddress(m_handle, funcname);
}