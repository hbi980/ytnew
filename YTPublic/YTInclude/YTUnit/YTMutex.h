#pragma once

#include <windows.h>

class CYTMutex
{
public:
	CYTMutex(void);
	~CYTMutex(void);

public:
	// ������������
	bool Lock(int time = 5000);	// ����
	void UnLock(void);			// ����
	
private:
	// ˽�г�Ա
	HANDLE	m_handle;		// ���ƾ��
};

