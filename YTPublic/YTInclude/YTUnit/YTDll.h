#pragma once

#include <windows.h>

class CYTDll
{
public:
	CYTDll(void);
	~CYTDll(void);

public:
	// ������������
	int		LoadDll(const char * dllname);			// ����dll
	void	CloseDll(void);								// �ر�dll
	void *	GetFuncAddr(const char * funcname);		// ��ȡ������ַ

private:
	// ˽�г�Ա
	HINSTANCE	m_handle;		// dll���
};

