/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYCrc.h
* �ļ�ժҪ��crc16λУ��
*
* �޸ļ�¼��2014-05-22 saimen����
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
	// ������������
	unsigned short CrcCheck(char * inbuf, unsigned short inlen);	// crcУ��
};

#endif