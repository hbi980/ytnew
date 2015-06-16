/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYSemaphore.h
* �ļ�ժҪ���ź���
*
* �޸ļ�¼��2014-05-22 saimen����
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
	// ������������
	long Init(long inicount, long maxcount);	// ��ʼ��
	long Lock(long timeout = 5000);				// ����
	void UnLock(long count = 1);				// ����
	
private:
	// ˽�г�Ա
	HANDLE	m_handle;		// ���ƾ��
};

#endif