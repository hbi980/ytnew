/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYMd5.h
* 文件摘要：MD5加密控制
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYMD5_H__
#define __BYMD5_H__

#include <windows.h>

#include <string>
#include <fstream>
using namespace std;

typedef unsigned char byte;
typedef unsigned int uint32;

class CBYMd5
{
public:
	CBYMd5();
	CBYMd5(const void* input, size_t length);
	CBYMd5(const string& str);
	CBYMd5(ifstream& in);
	void update(const void* input, size_t length);
	void update(const string& str);
	void update(ifstream& in);
	const byte* digest();
	string toString();
	void reset();

private:
	void update(const byte* input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const uint32* input, byte* output, size_t length);
	void decode(const byte* input, uint32* output, size_t length);
	string bytesToHexString(const byte* input, size_t length);
	
	/* class uncopyable */
	CBYMd5(const CBYMd5&);
	CBYMd5& operator=(const CBYMd5&);

private:
	uint32 _state[4];	/* state (ABCD) */
	uint32 _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64];	/* input buffer */
	byte _digest[16];	/* message digest */
	bool _finished;		/* calculate finished ? */
	
	static const byte PADDING[64];	/* padding for calculate */
	static const char HEX[16];
	enum { BUFFER_SIZE = 1024 };
};

#endif