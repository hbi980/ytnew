/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYMutex.h
* 文件摘要：等待控制锁
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYMUTEX_H__
#define __BYMUTEX_H__

#include <windows.h>

class CBYMutex
{
public:
	CBYMutex();
	virtual ~CBYMutex();
	
public:
	// 公共导出函数
	bool Lock(long time = 5000);	// 加锁
	void UnLock();					// 解锁
	
private:
	// 私有成员
	HANDLE	m_handle;		// 控制句柄
};

#endif