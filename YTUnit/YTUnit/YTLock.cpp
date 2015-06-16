#include "../../YTPublic/YTInclude/YTUnit/YTLock.h"


CYTLock::CYTLock(void)
{
	::InitializeCriticalSection(&m_section);
}

CYTLock::~CYTLock(void)
{
	::DeleteCriticalSection(&m_section);
}

// ¼ÓËø
void CYTLock::Lock()
{
	::EnterCriticalSection(&m_section);
}

// ½âËø
void CYTLock::UnLock()
{
	::LeaveCriticalSection(&m_section);
}
