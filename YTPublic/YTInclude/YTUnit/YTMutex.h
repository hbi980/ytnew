#pragma once

#include <windows.h>

class CYTMutex
{
public:
	CYTMutex(void);
	~CYTMutex(void);

public:
	// 公共导出函数
	bool Lock(int time = 5000);	// 加锁
	void UnLock(void);			// 解锁
	
private:
	// 私有成员
	HANDLE	m_handle;		// 控制句柄
};

