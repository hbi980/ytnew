/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYDll.h
* �ļ�ժҪ��Dll���ؿ���
*
* �޸ļ�¼��2014-05-22 saimen����
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
	// ������������
	long	LoadDll(const char * dllname);			// ����dll
	void	CloseDll();								// �ر�dll
	void *	GetFuncAddr(const char * funcname);		// ��ȡ������ַ

private:
	// ˽�г�Ա
	HINSTANCE	m_handle;		// dll���
};

#endif