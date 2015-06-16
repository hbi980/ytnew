#include "../../YTPublic/YTInclude/YTUnit/YTSemaphore.h"


CYTSemaphore::CYTSemaphore(void)
{
	m_handle = NULL;
}

CYTSemaphore::~CYTSemaphore(void)
{
	if(m_handle != NULL)
	{
		::CloseHandle(m_handle);
		m_handle = NULL;
	}
}

// 初始化
int CYTSemaphore::Init(int inicount, int maxcount)
{
	if(m_handle != NULL)
	{
		::CloseHandle(m_handle);
		m_handle = NULL;
	}

	m_handle = ::CreateSemaphore(NULL, inicount, maxcount, NULL);
	if(m_handle != NULL)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

// 加锁
int CYTSemaphore::Lock(int timeout)
{
	if(::WaitForSingleObject(m_handle, timeout) == WAIT_OBJECT_0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

// 解锁
void CYTSemaphore::UnLock(int count)
{
	::ReleaseSemaphore(m_handle, count, NULL);
}