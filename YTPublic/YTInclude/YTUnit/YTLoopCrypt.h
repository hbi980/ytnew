#pragma once

#include <windows.h>

class CYTLoopCrypt
{
public:
	CYTLoopCrypt(void);
	~CYTLoopCrypt(void);

public:
	// ��̬��������
	static void GenerateKey(unsigned char * key, int len);								// ����KEY
	static void Crypt(unsigned char * data, int len, unsigned char * key, int keylen);	// �ӽ��ܺ�����ͬһ����
};

