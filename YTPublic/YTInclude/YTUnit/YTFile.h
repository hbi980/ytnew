#pragma once

#include <windows.h>

// 文件打开模式定义
enum ENum_FileMode
{
	FILEMODE_READ		= 0x01,											// 读
	FILEMODE_WRITE		= 0x02,											// 写
	FILEMODE_ALL		= FILEMODE_READ | FILEMODE_WRITE,				// 读写
	FILEMODE_SHAREREAD	= 0x04,											// 共享读
	FILEMODE_SHAREWRITE = 0x08,											// 共享写
	FILEMODE_SHAREALL	= FILEMODE_SHAREREAD | FILEMODE_SHAREWRITE,		// 共享读写
};

// 文件移动位置定义
enum ENum_FileSeek
{
	FILESEEK_BEGIN		= 0x00,											// 文件开头
	FILESEEK_CURRENT	= 0x01,											// 当前光标位置
	FILESEEK_END		= 0x02,											// 文件结尾
};

class CYTFile
{
public:
	CYTFile(void);
	~CYTFile(void);

public:
	// 公共导出函数
	int  Open(const char * filename, unsigned int mode = FILEMODE_ALL | FILEMODE_SHAREALL);	// 文件打开
	int  Create(const char * filename);											// 文件创建
	void Close(void);															// 文件关闭
	int SetLength(unsigned int len);											// 设置文件为指定大小
	unsigned int Length(void);													// 文件大小
	bool FlushFileBuffer(void);													// 更新缓存立刻到文件
	unsigned int Seek(unsigned int mode, int offset);							// 文件光标位置移动
	unsigned int Read(char * outbuf, unsigned int len);							// 文件读取
	unsigned int Write(char * inbuf, unsigned int len);							// 文件写入

public:
	// 静态导出函数
	static int CreateDirectory(const char * pathname);							// 创建目录
	static int DeleteDirectory(const char * pathname);							// 删除目录
	static int DeleteFile(const char * filename);								// 删除一个文件
	static bool IsExist(const char * filename);									// 判断文件是否存在
	
protected:
	// 受保护成员
	HANDLE		m_handle;		// 文件句柄
};