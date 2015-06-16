/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYCompress.h
* �ļ�ժҪ��ѹ���㷨��Ŀǰ����LZWѹ���㷨��8192�����ڵ�ѹ����
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYCOMPRESS_H__
#define __BYCOMPRESS_H__

#include <windows.h>

// ��ض���
#pragma pack(1)

typedef struct 
{
	short	codevalue;
	short	parentcode;
	char	character;
} tagCompressInfo;

#pragma pack()

// ѹ���㷨��
class CBYCompress
{
public:
	CBYCompress();
	virtual ~CBYCompress();
	
public:
	// ������������
	int Compress(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// ѹ��
	int Expand(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// ��ѹ��

private:
	// ��������
	void Init();
	unsigned long FindChildNode(long parentcode, long childcode);
	unsigned long DecodeString(unsigned long count, unsigned long code);
	int PutBit(char * inbuf, unsigned short inlen, unsigned long offset, unsigned long value, unsigned long size);
	int GetBit(char * inbuf, unsigned short inlen, unsigned long offset, unsigned long * value, unsigned long size);

private:
	// ˽�г�Ա
	tagCompressInfo	m_dictory[9973];
	char			m_stack[9973];
};

#endif