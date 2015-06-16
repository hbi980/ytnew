/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYMutex.h
* �ļ�ժҪ���ȴ�������
*
* �޸ļ�¼��2014-05-22 saimen����
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
	// ������������
	bool Lock(long time = 5000);	// ����
	void UnLock();					// ����
	
private:
	// ˽�г�Ա
	HANDLE	m_handle;		// ���ƾ��
};

#endif