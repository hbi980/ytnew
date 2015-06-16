#pragma once

#include <windows.h>

class CYTEvent
{
public:
	CYTEvent(void);
	~CYTEvent(void);

public:
	// 公共导出函数
	bool Wait(int timeout = 5000);			// 等待
	void notify(void);						// 唤醒
	
private:
	// 私有成员
	HANDLE	m_handle;		// 控制句柄
};

