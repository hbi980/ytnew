#pragma once

#include <windows.h>

// �����̶߳���
typedef unsigned int (__stdcall * ThreadFunc)(void *);

class CYTThread
{
public:
	CYTThread(void);
	~CYTThread(void);

public:
	// ������������
	int  BeginThread(ThreadFunc func, void * param, const char * name);	// �����߳�
	void StopThread(unsigned int timeout = 5000);						// �ر��߳�
	void SetForceCloseThreadFlag(bool flag);							// ����ǿ�ƹر��̱߳�־

	bool GetStopFlag(void);												// ��ȡ�̹߳ر�״̬��־
	unsigned int GetID(void);											// ��ȡ�߳�ID
	const char * GetName(void);											// ��ȡ�߳�����

private:
	// ˽�г�Ա
	HANDLE			m_handle;		// �ļ����
	bool			m_stopflag;		// �߳��Ƿ�ر�״̬
	unsigned int	m_id;			// �߳�ID
	char			m_name[128];	// �߳���
	bool			m_bForceClose;	// �Ƿ�����ǿ�ƹر�
};

