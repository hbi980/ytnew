#pragma once

#include <windows.h>

// �ļ���ģʽ����
enum ENum_FileMode
{
	FILEMODE_READ		= 0x01,											// ��
	FILEMODE_WRITE		= 0x02,											// д
	FILEMODE_ALL		= FILEMODE_READ | FILEMODE_WRITE,				// ��д
	FILEMODE_SHAREREAD	= 0x04,											// �����
	FILEMODE_SHAREWRITE = 0x08,											// ����д
	FILEMODE_SHAREALL	= FILEMODE_SHAREREAD | FILEMODE_SHAREWRITE,		// �����д
};

// �ļ��ƶ�λ�ö���
enum ENum_FileSeek
{
	FILESEEK_BEGIN		= 0x00,											// �ļ���ͷ
	FILESEEK_CURRENT	= 0x01,											// ��ǰ���λ��
	FILESEEK_END		= 0x02,											// �ļ���β
};

class CYTFile
{
public:
	CYTFile(void);
	~CYTFile(void);

public:
	// ������������
	int  Open(const char * filename, unsigned int mode = FILEMODE_ALL | FILEMODE_SHAREALL);	// �ļ���
	int  Create(const char * filename);											// �ļ�����
	void Close(void);															// �ļ��ر�
	int SetLength(unsigned int len);											// �����ļ�Ϊָ����С
	unsigned int Length(void);													// �ļ���С
	bool FlushFileBuffer(void);													// ���»������̵��ļ�
	unsigned int Seek(unsigned int mode, int offset);							// �ļ����λ���ƶ�
	unsigned int Read(char * outbuf, unsigned int len);							// �ļ���ȡ
	unsigned int Write(char * inbuf, unsigned int len);							// �ļ�д��

public:
	// ��̬��������
	static int CreateDirectory(const char * pathname);							// ����Ŀ¼
	static int DeleteDirectory(const char * pathname);							// ɾ��Ŀ¼
	static int DeleteFile(const char * filename);								// ɾ��һ���ļ�
	static bool IsExist(const char * filename);									// �ж��ļ��Ƿ����
	
protected:
	// �ܱ�����Ա
	HANDLE		m_handle;		// �ļ����
};