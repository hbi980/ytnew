#pragma once

#include <windows.h>

// 定义线程对象
typedef unsigned int (__stdcall * ThreadFunc)(void *);

class CYTThread
{
public:
	CYTThread(void);
	~CYTThread(void);

public:
	// 公共导出函数
	int  BeginThread(ThreadFunc func, void * param, const char * name);	// 启动线程
	void StopThread(unsigned int timeout = 5000);						// 关闭线程
	void SetForceCloseThreadFlag(bool flag);							// 设置强制关闭线程标志

	bool GetStopFlag(void);												// 获取线程关闭状态标志
	unsigned int GetID(void);											// 获取线程ID
	const char * GetName(void);											// 获取线程名称

private:
	// 私有成员
	HANDLE			m_handle;		// 文件句柄
	bool			m_stopflag;		// 线程是否关闭状态
	unsigned int	m_id;			// 线程ID
	char			m_name[128];	// 线程名
	bool			m_bForceClose;	// 是否允许强制关闭
};

