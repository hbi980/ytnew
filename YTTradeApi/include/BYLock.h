/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYLock.h
* 文件摘要：控制锁
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYLOCK_H__
#define __BYLOCK_H__

#include <windows.h>

class CBYLock
{
public:
	CBYLock();
	virtual ~CBYLock();
	
public:
	// 公共导出函数
	void Lock();			// 加锁
	void UnLock();			// 解锁
	
private:
	// 私有成员
	CRITICAL_SECTION	m_section;		// 数据控制锁
};

#endif