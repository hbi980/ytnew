#pragma once

#include <windows.h>

class CYTLock
{
public:
	CYTLock(void);
	~CYTLock(void);

public:
	// ������������
	void Lock(void);			// ����
	void UnLock(void);			// ����
	
private:
	// ˽�г�Ա
	CRITICAL_SECTION	m_section;		// ���ݿ�����
};

