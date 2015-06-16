#include "../../YTPublic/YTInclude/YTUnit/YTThread.h"
#include <process.h>

CYTThread::CYTThread(void)
{
	m_handle		= NULL;
	m_stopflag		= true;
	m_id			= 0;
	memset(m_name, 0, sizeof(m_name));
	m_bForceClose	= true;
}

CYTThread::~CYTThread(void)
{
	if(m_bForceClose)
	{
		StopThread();
	}
	else
	{
		::CloseHandle(m_handle);
		m_handle = NULL;
		m_id = 0;
	}
}

// 启动线程
int CYTThread::BeginThread(ThreadFunc func, void * param, const char * name)
{
	StopThread();

	if(name != NULL)
	{
		memcpy(m_name, name, min(sizeof(m_name)-1, strlen(name)));
	}
	m_stopflag	= false;

	m_handle = (HANDLE)_beginthreadex(NULL, 0, func, param, 0, (unsigned *)&m_id);
	if(m_handle == NULL)
	{
		m_stopflag	= true;
		m_id		= 0;
		memset(m_name, 0, sizeof(m_name));
		return -1;
	}

	return 0;
}

// 关闭线程
void CYTThread::StopThread(unsigned int timeout)
{
	if(m_handle != NULL)
	{
		m_stopflag = true;
		memset(m_name, 0, sizeof(m_name));

		unsigned int ret = ::WaitForSingleObject(m_handle, timeout);
		if(ret==WAIT_TIMEOUT || ret==WAIT_FAILED)
		{
			::TerminateThread(m_handle, 0);
		}

		::CloseHandle(m_handle);
		m_handle = NULL;

		m_id = 0;
	}
}

// 设置强制关闭线程标志
void CYTThread::SetForceCloseThreadFlag(bool flag)
{
	m_bForceClose = flag;
}

// 获取线程关闭状态标志
bool CYTThread::GetStopFlag(void)
{
	return m_stopflag;
}

// 获取线程ID
unsigned int CYTThread::GetID(void)
{
	return m_id;
}

// 获取线程名称
const char * CYTThread::GetName(void)
{
	return (const char *)m_name;
}