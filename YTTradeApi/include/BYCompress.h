/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYCompress.h
* 文件摘要：压缩算法，目前采用LZW压缩算法（8192长度内的压缩）
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYCOMPRESS_H__
#define __BYCOMPRESS_H__

#include <windows.h>

// 相关定义
#pragma pack(1)

typedef struct 
{
	short	codevalue;
	short	parentcode;
	char	character;
} tagCompressInfo;

#pragma pack()

// 压缩算法类
class CBYCompress
{
public:
	CBYCompress();
	virtual ~CBYCompress();
	
public:
	// 公共导出函数
	int Compress(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// 压缩
	int Expand(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// 解压缩

private:
	// 辅助函数
	void Init();
	unsigned long FindChildNode(long parentcode, long childcode);
	unsigned long DecodeString(unsigned long count, unsigned long code);
	int PutBit(char * inbuf, unsigned short inlen, unsigned long offset, unsigned long value, unsigned long size);
	int GetBit(char * inbuf, unsigned short inlen, unsigned long offset, unsigned long * value, unsigned long size);

private:
	// 私有成员
	tagCompressInfo	m_dictory[9973];
	char			m_stack[9973];
};

#endif