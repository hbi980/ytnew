#pragma once

#include <windows.h>

class CYTLoopCrypt
{
public:
	CYTLoopCrypt(void);
	~CYTLoopCrypt(void);

public:
	// 静态导出函数
	static void GenerateKey(unsigned char * key, int len);								// 生成KEY
	static void Crypt(unsigned char * data, int len, unsigned char * key, int keylen);	// 加解密函数（同一个）
};

