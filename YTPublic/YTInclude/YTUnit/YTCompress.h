#pragma once

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

class CYTCompress
{
public:
	CYTCompress(void);
	~CYTCompress(void);

public:
	// ������������
	int Compress(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// ѹ��
	int Expand(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// ��ѹ��

private:
	// ��������
	void Init(void);
	unsigned int FindChildNode(int parentcode, int childcode);
	unsigned int DecodeString(unsigned int count, unsigned int code);
	int PutBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int value, unsigned int size);
	int GetBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int * value, unsigned int size);

private:
	// ˽�г�Ա
	tagCompressInfo	m_dictory[9973];
	char			m_stack[9973];
};

