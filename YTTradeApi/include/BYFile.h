/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYFile.h
* �ļ�ժҪ���ļ�����
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYFILE_H__
#define __BYFILE_H__

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

// �ļ�������
class CBYFile
{
public:
	CBYFile();
	virtual ~CBYFile();
	
public:
	// ������������
	long Open(const char * filename, unsigned long mode = FILEMODE_ALL | FILEMODE_SHAREALL);	// �ļ���
	long Create(const char * filename);											// �ļ�����
	void Close();																// �ļ��ر�
	long SetLength(unsigned long len);											// �����ļ�Ϊָ����С
	unsigned long Length();														// �ļ���С
	bool FlushFileBuffer();														// ���»������̵��ļ�
	unsigned long Seek(unsigned long mode, long offset);						// �ļ����λ���ƶ�
	unsigned long Read(char * outbuf, unsigned long len);						// �ļ���ȡ
	unsigned long Write(char * inbuf, unsigned long len);						// �ļ�д��

public:
	// ��̬��������
	static long CreateDirectory(const char * pathname);							// ����Ŀ¼
	static long DeleteDirectory(const char * pathname);							// ɾ��Ŀ¼
	static long DeleteFile(const char * filename);								// ɾ��һ���ļ�
	static bool IsExist(const char * filename);									// �ж��ļ��Ƿ����
	
protected:
	// �ܱ�����Ա
	HANDLE		m_handle;		// �ļ����
};

#endif