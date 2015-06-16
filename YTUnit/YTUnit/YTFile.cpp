#include "../../YTPublic/YTInclude/YTUnit/YTFile.h"
#include "io.h"

CYTFile::CYTFile(void)
{
	m_handle = INVALID_HANDLE_VALUE;
}

CYTFile::~CYTFile(void)
{
	Close();
}

// �ļ���
int CYTFile::Open(const char * filename, unsigned int mode)
{
	Close();
	
	unsigned int openmode	= 0;
	unsigned int sharemode = 0;

	if((mode & FILEMODE_READ) == FILEMODE_READ)
	{
		openmode |= GENERIC_READ;
	}

	if((mode & FILEMODE_WRITE) == FILEMODE_WRITE)
	{
		openmode |= GENERIC_WRITE;
	}

	if((mode & FILEMODE_SHAREREAD) == FILEMODE_SHAREREAD)
	{
		sharemode |= FILE_SHARE_READ;
	}
	
	if((mode & FILEMODE_SHAREWRITE) == FILEMODE_SHAREWRITE)
	{
		sharemode |= FILE_SHARE_WRITE;
	}

	m_handle = ::CreateFile(filename, 
							openmode, 
							sharemode, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);
	if(m_handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// �ļ�����
int CYTFile::Create(const char * filename)
{
	Close();

	m_handle = ::CreateFile(filename, 
							GENERIC_READ|GENERIC_WRITE, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							NULL, 
							CREATE_NEW, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);
	if(m_handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// �ļ��ر�
void CYTFile::Close(void)
{
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

// �����ļ�Ϊָ����С
int CYTFile::SetLength(unsigned int len)
{
	if(::SetFilePointer(m_handle, len, 0, FILESEEK_BEGIN) == 0xFFFFFFFF)
	{
		return -1;
	}

	if(::SetEndOfFile(m_handle) != TRUE)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

// �ļ���С
unsigned int CYTFile::Length(void)
{
	return Seek(FILESEEK_END, 0);
}

// ���»������̵��ļ�
bool CYTFile::FlushFileBuffer(void)
{
	if(::FlushFileBuffers(m_handle) == TRUE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// �ļ����λ���ƶ�
unsigned int CYTFile::Seek(unsigned int mode, int offset)
{
	return ::SetFilePointer(m_handle, offset, 0, mode);
}

// �ļ���ȡ
unsigned int CYTFile::Read(char * outbuf, unsigned int len)
{
	unsigned long ret = 0;
	if(::ReadFile(m_handle, outbuf, len, &ret, NULL) == FALSE)
	{
		return 0xFFFFFFFF;
	}
	else
	{
		return ret;
	}
}

// �ļ�д��
unsigned int CYTFile::Write(char * inbuf, unsigned int len)
{
	unsigned long ret = 0;
	if(::WriteFile(m_handle, inbuf, len, &ret, NULL) == FALSE)
	{
		return 0xFFFFFFFF;
	}
	else
	{
		return ret;
	}
}

// ����Ŀ¼
int CYTFile::CreateDirectory(const char * pathname)
{
	if(::CreateDirectory(pathname, NULL) == FALSE)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// ɾ��Ŀ¼
int CYTFile::DeleteDirectory(const char * pathname)
{
	if(::RemoveDirectory(pathname) == FALSE)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// ɾ��һ���ļ�
int CYTFile::DeleteFile(const char * filename)
{
	if(::DeleteFile(filename) == FALSE)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// �ж��ļ��Ƿ����
bool CYTFile::IsExist(const char * filename)
{
	if(_access(filename, 0) == -1)
	{
		return false;
	}
	
	return true;
}
