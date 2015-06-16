#include "../../YTPublic/YTInclude/YTUnit/YTMutex.h"


CYTMutex::CYTMutex(void)
{
	m_handle = ::CreateMutex(NULL, FALSE, NULL);
}

CYTMutex::~CYTMutex(void)
{
	if(m_handle != NULL)
	{
		::ReleaseMutex(m_handle);
		::CloseHandle(m_handle);
	}
}

// ¼ÓËø
bool CYTMutex::Lock(int time)
{
	if(::WaitForSingleObject(m_handle, time) == WAIT_OBJECT_0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ½âËø
void CYTMutex::UnLock(void)
{
	::ReleaseMutex(m_handle);
}
