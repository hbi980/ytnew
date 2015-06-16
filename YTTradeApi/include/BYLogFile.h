/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYLogFile.h
* �ļ�ժҪ����־�ļ�����
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYLOGFILE_H__
#define __BYLOGFILE_H__

#include "BYFile.h"
#include "BYLock.h"
#include "BYThread.h"

#define MAX_LOG_BUFFER	1024*20

// ��־���Ͷ���
enum ENum_LogType
{
	LT_SYS		= 0x00,		// ϵͳ��־����¼ϵͳ�����ر��Լ��ؼ�λ�õ��쳣����Ϣ
	LT_RUN		= 0x01,		// ������־����ʾ�û���¼�رյ����
	LT_DEBUG	= 0x02,		// ������־��Ϊ�˽�ʾ�����쳣��ӡһЩ�Ĳ�����־
};

// �ļ�������
class CBYLogFile : public CBYFile
{
public:
	CBYLogFile(long logwritetime = 20);
	virtual ~CBYLogFile();
	
public:
	// ������������
	void SetFileName(const char * filename);								// ������־�ļ���ǰ׺
	void SetDirectory(const char * directory);								// ������־�ļ�Ŀ¼
	void SetLogType(long maxtype);											// �����־�������ú�ֻ��ӡ<=���õļ������־

	void WriteLog(long logtype, const char * log, long loglen = 0);			// д��һ���ַ���
	void WriteLog_Time(long logtype, const char * log, long loglen = 0);	// д��һ���ַ�������ʱ���¼��
	void WriteLogFormat(long logtype, const char * format, ...);			// д��һ�����и�ʽ������
	void WriteLogFormat_Time(long logtype, const char * format, ...);		// д��һ�����и�ʽ�����ݣ���ʱ���¼��

private:
	// ��������
	bool CheckFile();														// ����ļ�����û���򴴽�
	void FlushLogBuffer();													// ����ʱ�����е���Ϣд���ļ�
	void WriteToFile(const char * log, long loglen);						// д����־

	static unsigned int __stdcall LogWriteThread(void * pParam);			// �ļ�����д���߳�
	
private:
	// ˽�г�Ա
	long		m_maxtype;							// �����־�������ú�ֻ��ӡ<=���õļ������־
	char		m_Name[128];						// ��־�ļ���ǰ׺
	char		m_Directory[256];					// ��־�ļ�Ŀ¼
	
	long		m_logdate;							// ��ǰ�ļ�����
	char		m_logbuffer[MAX_LOG_BUFFER+1];		// ��־��ʱ����
	long		m_loglen;							// ��־��ʱ���泤��
	CBYLock		m_lock;								// �ļ�����������

	long		m_logwritetime;						// д���浽�ļ���ʱ�����
	long		m_stopflag;							// �߳��˳���־
	CBYThread	m_thread;							// ��ʱд���浽�ļ��Ŀ����߳�
};

#endif