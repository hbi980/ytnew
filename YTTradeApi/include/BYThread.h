/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYThread.h
* �ļ�ժҪ���߳̿���
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYTHREAD_H__
#define __BYTHREAD_H__

#include <windows.h>

// �����̶߳���
typedef unsigned int (__stdcall * ThreadFunc)(void *);

class CBYThread
{
public:
	CBYThread();
	virtual ~CBYThread();
	
public:
	// ������������
	long BeginThread(ThreadFunc func, void * param, const char * name);	// �����߳�
	void StopThread(unsigned long timeout = 5000);						// �ر��߳�
	void SetForceCloseThreadFlag(bool flag);							// ����ǿ�ƹر��̱߳�־

	bool GetStopFlag();													// ��ȡ�̹߳ر�״̬��־
	unsigned long GetID();												// ��ȡ�߳�ID
	const char * GetName();												// ��ȡ�߳�����

private:
	// ˽�г�Ա
	HANDLE			m_handle;		// �ļ����
	bool			m_stopflag;		// �߳��Ƿ�ر�״̬
	unsigned long	m_id;			// �߳�ID
	char			m_name[128];	// �߳���
	bool			m_bForceClose;	// �Ƿ�����ǿ�ƹر�
};

#endif