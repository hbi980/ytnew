#pragma once

#include <windows.h>

class CYTCrc
{
public:
	CYTCrc(void);
	~CYTCrc(void);

public:
	// ������������
	unsigned short CrcCheck(char * inbuf, unsigned short inlen);	// crcУ��
};

