#pragma once

#include <windows.h>

class CYTDll
{
public:
	CYTDll(void);
	~CYTDll(void);

public:
	// 公共导出函数
	int		LoadDll(const char * dllname);			// 加载dll
	void	CloseDll(void);								// 关闭dll
	void *	GetFuncAddr(const char * funcname);		// 获取函数地址

private:
	// 私有成员
	HINSTANCE	m_handle;		// dll句柄
};

