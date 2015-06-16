#pragma once

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

class CYTCompress
{
public:
	CYTCompress(void);
	~CYTCompress(void);

public:
	// 公共导出函数
	int Compress(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// 压缩
	int Expand(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen);	// 解压缩

private:
	// 辅助函数
	void Init(void);
	unsigned int FindChildNode(int parentcode, int childcode);
	unsigned int DecodeString(unsigned int count, unsigned int code);
	int PutBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int value, unsigned int size);
	int GetBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int * value, unsigned int size);

private:
	// 私有成员
	tagCompressInfo	m_dictory[9973];
	char			m_stack[9973];
};

