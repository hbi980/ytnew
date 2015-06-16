#include "../../YTPublic/YTInclude/YTUnit/YTEvent.h"

CYTEvent::CYTEvent(void)
{
	m_handle = CreateEvent(NULL, TRUE, FALSE, NULL); 
}

CYTEvent::~CYTEvent(void)
{
	if(m_handle != NULL)
	{
		SetEvent(m_handle);
		CloseHandle(m_handle);
		m_handle = NULL;
	}
}

// ¼ÓËø
bool CYTEvent::Wait(int timeout)
{
	ResetEvent(m_handle);
	if(::WaitForSingleObject(m_handle, timeout) == WAIT_TIMEOUT)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// ½âËø
void CYTEvent::notify(void)
{
	SetEvent(m_handle);
}