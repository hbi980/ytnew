/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYFile.h
* 文件摘要：文件控制
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYFILE_H__
#define __BYFILE_H__

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

// 文件操作类
class CBYFile
{
public:
	CBYFile();
	virtual ~CBYFile();
	
public:
	// 公共导出函数
	long Open(const char * filename, unsigned long mode = FILEMODE_ALL | FILEMODE_SHAREALL);	// 文件打开
	long Create(const char * filename);											// 文件创建
	void Close();																// 文件关闭
	long SetLength(unsigned long len);											// 设置文件为指定大小
	unsigned long Length();														// 文件大小
	bool FlushFileBuffer();														// 更新缓存立刻到文件
	unsigned long Seek(unsigned long mode, long offset);						// 文件光标位置移动
	unsigned long Read(char * outbuf, unsigned long len);						// 文件读取
	unsigned long Write(char * inbuf, unsigned long len);						// 文件写入

public:
	// 静态导出函数
	static long CreateDirectory(const char * pathname);							// 创建目录
	static long DeleteDirectory(const char * pathname);							// 删除目录
	static long DeleteFile(const char * filename);								// 删除一个文件
	static bool IsExist(const char * filename);									// 判断文件是否存在
	
protected:
	// 受保护成员
	HANDLE		m_handle;		// 文件句柄
};

#endif