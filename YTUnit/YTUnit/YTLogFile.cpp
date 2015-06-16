#include "../../YTPublic/YTInclude/YTUnit/YTLogFile.h"
#include "stdio.h"
#include <time.h>

CYTLogFile::CYTLogFile(int logwritetime)
{
	m_maxtype		= LT_SYS;
	sprintf_s(m_Name, sizeof(m_Name), "log");
	sprintf_s(m_Directory, sizeof(m_Directory), "log");

	m_logdate		= -1;
	memset(m_logbuffer, 0, sizeof(m_logbuffer));
	m_loglen		= 0;

	m_logwritetime	= logwritetime;
	m_stopflag		= false;
	if(m_thread.BeginThread(LogWriteThread, this, " ") < 0)
	{
		m_stopflag = true;
	}
}

CYTLogFile::~CYTLogFile(void)
{
	m_stopflag = true;
	m_thread.StopThread(200);

	m_lock.Lock();
	FlushLogBuffer();
	m_lock.UnLock();

	CYTFile::Close();
}

// ������־�ļ���ǰ׺
void CYTLogFile::SetFileName(const char * filename)
{
	if(filename != NULL)
	{
		memset(m_Name, 0, sizeof(m_Name));
		memcpy(m_Name, filename, min(sizeof(m_Name)-1, strlen(filename)));
	}
}

// ������־�ļ�Ŀ¼
void CYTLogFile::SetDirectory(const char * directory)
{
	if(directory != NULL)
	{
		memset(m_Directory, 0, sizeof(m_Directory));
		memcpy(m_Directory, directory, min(sizeof(m_Directory)-1, strlen(directory)));
	}
}

// �����־�������ú�ֻ��ӡ<=���õļ������־
void CYTLogFile::SetLogType(int maxtype)
{
	if(maxtype < LT_SYS)
	{
		m_maxtype = LT_SYS;
	}
	else if(maxtype > LT_DEBUG)
	{
		m_maxtype = LT_DEBUG;
	}
	else
	{
		m_maxtype = maxtype;
	}
}

// д��һ���ַ���
void CYTLogFile::WriteLog(int logtype, const char * log, int loglen)
{
	if(logtype > m_maxtype)
	{
		return;
	}

	if(log == NULL)
	{
		return;
	}
	
	int len = loglen;
	if(len <= 0)
	{
		len = strlen(log);
	}
	if(len <= 0)
	{
		return;
	}
	
	WriteToFile(log, len);
}

// д��һ���ַ�������ʱ���¼��
void CYTLogFile::WriteLog_Time(int logtype, const char * log, int loglen)
{
	if(logtype > m_maxtype)
	{
		return;
	}

	if(log == NULL)
	{
		return;
	}
	
	int len = loglen;
	if(len <= 0)
	{
		len = strlen(log);
	}
	if(len <= 0)
	{
		return;
	}
	
	SYSTEMTIME systime;
	::GetSystemTime(&systime);
	char szBuffer[40960] = {0};
	sprintf_s(szBuffer+strlen(szBuffer), sizeof(szBuffer)-strlen(szBuffer), "[%02d:%02d:%02d.%03d] ", (systime.wHour+8)%24, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	sprintf_s(szBuffer+strlen(szBuffer), sizeof(szBuffer)-strlen(szBuffer), log);
	
	WriteLog(logtype, szBuffer, strlen(szBuffer));
}

// д��һ�����и�ʽ������
void CYTLogFile::WriteLogFormat(int logtype, const char * format, ...)
{
	if(logtype > m_maxtype)
	{
		return;
	}
	
	if(format == NULL)
	{
		return;
	}
	
	char szBuffer[40960] = {0};
	va_list arg_ptr;
	va_start(arg_ptr, format);
	_vsnprintf_s(szBuffer+strlen(szBuffer), sizeof(szBuffer)-strlen(szBuffer), _TRUNCATE, format, arg_ptr);
	va_end(arg_ptr);
	
	WriteLog(logtype, szBuffer, strlen(szBuffer));
}

// д��һ�����и�ʽ�����ݣ���ʱ���¼��
void CYTLogFile::WriteLogFormat_Time(int logtype, const char * format, ...)
{
	if(logtype > m_maxtype)
	{
		return;
	}
	
	if(format == NULL)
	{
		return;
	}
	
	SYSTEMTIME systime;
	::GetSystemTime(&systime);
	char szBuffer[40960] = {0};
	sprintf_s(szBuffer, sizeof(szBuffer), "[%02d:%02d:%02d.%03d] ", (systime.wHour+8)%24, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	
	va_list arg_ptr;
	va_start(arg_ptr, format);
	_vsnprintf_s(szBuffer+strlen(szBuffer), sizeof(szBuffer)-strlen(szBuffer), _TRUNCATE, format, arg_ptr);
	va_end(arg_ptr);
	
	WriteLog(logtype, szBuffer, strlen(szBuffer));
}

// ����ļ�����û���򴴽�
bool CYTLogFile::CheckFile()
{
	// �����ж�Ŀ¼�Ƿ����
	if(!CYTFile::IsExist(m_Directory))
	{
		if(CYTFile::CreateDirectory(m_Directory) < 0)
		{
			return false;
		}
	}

	// �ж��Ƿ����һ��
	SYSTEMTIME systime;
	::GetSystemTime(&systime);
	int currdate = systime.wYear*10000 + systime.wMonth*100 + systime.wDay;
	if(m_handle==INVALID_HANDLE_VALUE || currdate!=m_logdate)
	{
		CYTFile::Close();
		char szPath[512];
		sprintf_s(szPath, sizeof(szPath), "%s/%s_%d.log", m_Directory, m_Name, currdate);
		if(CYTFile::Open(szPath) < 0)
		{
			if(CYTFile::Create(szPath) < 0)
			{
				return false;
			}
		}
		m_logdate = currdate;
	}

	return true;
}

// ����ʱ�����е���Ϣд���ļ�
void CYTLogFile::FlushLogBuffer()
{
	if(m_loglen <= 0)
	{
		return;
	}

	if(!CheckFile())
	{
		return;
	}

	CYTFile::Seek(FILESEEK_END, 0);
	CYTFile::Write(m_logbuffer, m_loglen);
	CYTFile::FlushFileBuffer();

	memset(m_logbuffer, 0, m_loglen);
	m_loglen = 0;
}

// д����־
void CYTLogFile::WriteToFile(const char * log, int loglen)
{
	if(log == NULL)
	{
		return;
	}

	int len = loglen;
	if(len <= 0)
	{
		len = strlen(log);
	}
	if(len <= 0)
	{
		return;
	}

	m_lock.Lock();

	if(m_loglen+len > MAX_LOG_BUFFER)
	{
		// ���������С��һ����д�ļ�
		FlushLogBuffer();
		if(!CheckFile())
		{
			m_lock.UnLock();
			return;
		}
		CYTFile::Seek(FILESEEK_END, 0);
		CYTFile::Write((char *)log, len);
		CYTFile::FlushFileBuffer();
	}
	else
	{
		memcpy(m_logbuffer+m_loglen, log, len);
		m_loglen += len;
	}
	
	m_lock.UnLock();
}

// �����߳�
unsigned int __stdcall CYTLogFile::LogWriteThread(void * pParam)
{
	CYTLogFile * pYTLogFile = (CYTLogFile *)pParam;
	
	int sleeptime = pYTLogFile->m_logwritetime * 1000;
	while(!pYTLogFile->m_stopflag)
	{
		::Sleep(sleeptime);

		pYTLogFile->m_lock.Lock();
		pYTLogFile->FlushLogBuffer();
		pYTLogFile->m_lock.UnLock();
	}
	
	return 0;	
}
