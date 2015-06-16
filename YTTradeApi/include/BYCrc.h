/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYCrc.h
* 文件摘要：crc16位校验
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYCRC_H__
#define __BYCRC_H__

#include <windows.h>

class CBYCrc
{
public:
	CBYCrc();
	virtual ~CBYCrc();
	
public:
	// 公共导出函数
	unsigned short CrcCheck(char * inbuf, unsigned short inlen);	// crc校验
};

#endif