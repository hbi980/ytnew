/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYThread.h
* 文件摘要：线程控制
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYTHREAD_H__
#define __BYTHREAD_H__

#include <windows.h>

// 定义线程对象
typedef unsigned int (__stdcall * ThreadFunc)(void *);

class CBYThread
{
public:
	CBYThread();
	virtual ~CBYThread();
	
public:
	// 公共导出函数
	long BeginThread(ThreadFunc func, void * param, const char * name);	// 启动线程
	void StopThread(unsigned long timeout = 5000);						// 关闭线程
	void SetForceCloseThreadFlag(bool flag);							// 设置强制关闭线程标志

	bool GetStopFlag();													// 获取线程关闭状态标志
	unsigned long GetID();												// 获取线程ID
	const char * GetName();												// 获取线程名称

private:
	// 私有成员
	HANDLE			m_handle;		// 文件句柄
	bool			m_stopflag;		// 线程是否关闭状态
	unsigned long	m_id;			// 线程ID
	char			m_name[128];	// 线程名
	bool			m_bForceClose;	// 是否允许强制关闭
};

#endif