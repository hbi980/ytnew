/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYLoopCrypt.h
* 文件摘要：循环异或加密（加密不改变缓存大小）
*
* 特别注意：由于密钥的产生是通过rand随机方式，因此调用GenerateKey的线程或进程请先调用srand(time(NULL))初始化种子
*
* 修改记录：2014-05-22 saimen创建
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
	// 静态导出函数
	static void GenerateKey(unsigned char * key, long len);									// 生成KEY
	static void Crypt(unsigned char * data, long len, unsigned char * key, long keylen);	// 加解密函数（同一个）
};

#endif