#pragma once

#include <windows.h>

class CYTLock
{
public:
	CYTLock(void);
	~CYTLock(void);

public:
	// 公共导出函数
	void Lock(void);			// 加锁
	void UnLock(void);			// 解锁
	
private:
	// 私有成员
	CRITICAL_SECTION	m_section;		// 数据控制锁
};

