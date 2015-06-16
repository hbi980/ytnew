#pragma once

#include <windows.h>

class CYTSemaphore
{
public:
	CYTSemaphore(void);
	~CYTSemaphore(void);

public:
	// 公共导出函数
	int  Init(int inicount, int maxcount);		// 初始化
	int  Lock(int timeout = 5000);				// 加锁
	void UnLock(int count = 1);				// 解锁
	
private:
	// 私有成员
	HANDLE	m_handle;		// 控制句柄
};

