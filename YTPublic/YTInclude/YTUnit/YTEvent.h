#pragma once

#include <windows.h>

class CYTEvent
{
public:
	CYTEvent(void);
	~CYTEvent(void);

public:
	// ������������
	bool Wait(int timeout = 5000);			// �ȴ�
	void notify(void);						// ����
	
private:
	// ˽�г�Ա
	HANDLE	m_handle;		// ���ƾ��
};

