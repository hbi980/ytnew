/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYSemaphore.h
* 文件摘要：信号量
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYSEMAPHORE_H__
#define __BYSEMAPHORE_H__

#include <windows.h>

class CBYSemaphore
{
public:
	CBYSemaphore();
	virtual ~CBYSemaphore();
	
public:
	// 公共导出函数
	long Init(long inicount, long maxcount);	// 初始化
	long Lock(long timeout = 5000);				// 加锁
	void UnLock(long count = 1);				// 解锁
	
private:
	// 私有成员
	HANDLE	m_handle;		// 控制句柄
};

#endif