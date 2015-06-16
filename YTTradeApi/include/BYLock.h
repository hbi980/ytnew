/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYLock.h
* �ļ�ժҪ��������
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYLOCK_H__
#define __BYLOCK_H__

#include <windows.h>

class CBYLock
{
public:
	CBYLock();
	virtual ~CBYLock();
	
public:
	// ������������
	void Lock();			// ����
	void UnLock();			// ����
	
private:
	// ˽�г�Ա
	CRITICAL_SECTION	m_section;		// ���ݿ�����
};

#endif