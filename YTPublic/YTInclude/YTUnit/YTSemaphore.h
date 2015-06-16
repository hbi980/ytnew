#pragma once

#include <windows.h>

class CYTSemaphore
{
public:
	CYTSemaphore(void);
	~CYTSemaphore(void);

public:
	// ������������
	int  Init(int inicount, int maxcount);		// ��ʼ��
	int  Lock(int timeout = 5000);				// ����
	void UnLock(int count = 1);				// ����
	
private:
	// ˽�г�Ա
	HANDLE	m_handle;		// ���ƾ��
};

