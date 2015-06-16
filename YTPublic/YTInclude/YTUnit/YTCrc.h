#pragma once

#include <windows.h>

class CYTCrc
{
public:
	CYTCrc(void);
	~CYTCrc(void);

public:
	// 公共导出函数
	unsigned short CrcCheck(char * inbuf, unsigned short inlen);	// crc校验
};

