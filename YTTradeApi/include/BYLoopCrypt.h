/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYLoopCrypt.h
* �ļ�ժҪ��ѭ�������ܣ����ܲ��ı仺���С��
*
* �ر�ע�⣺������Կ�Ĳ�����ͨ��rand�����ʽ����˵���GenerateKey���̻߳�������ȵ���srand(time(NULL))��ʼ������
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYLOOPCRYPT_H__
#define __BYLOOPCRYPT_H__

#include <windows.h>

class CBYLoopCrypt
{
public:
	CBYLoopCrypt();
	virtual ~CBYLoopCrypt();
	
public:
	// ��̬��������
	static void GenerateKey(unsigned char * key, long len);									// ����KEY
	static void Crypt(unsigned char * data, long len, unsigned char * key, long keylen);	// �ӽ��ܺ�����ͬһ����
};

#endif