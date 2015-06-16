#pragma once

#include "YTFile.h"
#include "YTLock.h"
#include "YTThread.h"

#define MAX_LOG_BUFFER	1024*20

// ��־���Ͷ���
enum ENum_LogType
{
	LT_SYS		= 0x00,		// ϵͳ��־����¼ϵͳ�����ر��Լ��ؼ�λ�õ��쳣����Ϣ
	LT_RUN		= 0x01,		// ������־����ʾ�û���¼�رյ����
	LT_DEBUG	= 0x02,		// ������־��Ϊ�˽�ʾ�����쳣��ӡһЩ�Ĳ�����־
};

class CYTLogFile : public CYTFile
{
public:
	CYTLogFile(int logwritetime = 20);
	~CYTLogFile(void);

public:
	// ������������
	void SetFileName(const char * filename);								// ������־�ļ���ǰ׺
	void SetDirectory(const char * directory);								// ������־�ļ�Ŀ¼
	void SetLogType(int maxtype);											// �����־�������ú�ֻ��ӡ<=���õļ������־

	void WriteLog(int logtype, const char * log, int loglen = 0);			// д��һ���ַ���
	void WriteLog_Time(int logtype, const char * log, int loglen = 0);		// д��һ���ַ�������ʱ���¼��
	void WriteLogFormat(int logtype, const char * format, ...);				// д��һ�����и�ʽ������
	void WriteLogFormat_Time(int logtype, const char * format, ...);		// д��һ�����и�ʽ�����ݣ���ʱ���¼��

private:
	// ��������
	bool CheckFile(void);													// ����ļ�����û���򴴽�
	void FlushLogBuffer(void);												// ����ʱ�����е���Ϣд���ļ�
	void WriteToFile(const char * log, int loglen);							// д����־

	static unsigned int __stdcall LogWriteThread(void * pParam);			// �ļ�����д���߳�
	
private:
	// ˽�г�Ա
	int			m_maxtype;							// �����־�������ú�ֻ��ӡ<=���õļ������־
	char		m_Name[128];						// ��־�ļ���ǰ׺
	char		m_Directory[256];					// ��־�ļ�Ŀ¼
	
	int			m_logdate;							// ��ǰ�ļ�����
	char		m_logbuffer[MAX_LOG_BUFFER+1];		// ��־��ʱ����
	int			m_loglen;							// ��־��ʱ���泤��
	CYTLock		m_lock;								// �ļ�����������

	int			m_logwritetime;						// д���浽�ļ���ʱ�����
	int			m_stopflag;							// �߳��˳���־
	CYTThread	m_thread;							// ��ʱд���浽�ļ��Ŀ����߳�
};

