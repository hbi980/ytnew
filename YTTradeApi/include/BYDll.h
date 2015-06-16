/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYDll.h
* 文件摘要：Dll加载控制
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYDLL_H__
#define __BYDLL_H__

#include <windows.h>

class CBYDll
{
public:
	CBYDll();
	virtual ~CBYDll();
	
public:
	// 公共导出函数
	long	LoadDll(const char * dllname);			// 加载dll
	void	CloseDll();								// 关闭dll
	void *	GetFuncAddr(const char * funcname);		// 获取函数地址

private:
	// 私有成员
	HINSTANCE	m_handle;		// dll句柄
};

#endif