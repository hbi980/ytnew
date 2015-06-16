#pragma once

#include <windows.h>

#include <string>
#include <fstream>
using namespace std;

typedef unsigned char byte;
typedef unsigned int uint32;

class CYTMd5
{
public:
	CYTMd5(void);
	~CYTMd5(void);
	CYTMd5(const void* input, size_t length);
	CYTMd5(const string& str);
	CYTMd5(ifstream& in);
	void update(const void* input, size_t length);
	void update(const string& str);
	void update(ifstream& in);
	const byte* digest(void);
	string toString(void);
	void reset(void);

private:
	void update(const byte* input, size_t length);
	void final(void);
	void transform(const byte block[64]);
	void encode(const uint32* input, byte* output, size_t length);
	void decode(const byte* input, uint32* output, size_t length);
	string bytesToHexString(const byte* input, size_t length);
	
	/* class uncopyable */
	CYTMd5(const CYTMd5&);
	CYTMd5& operator=(const CYTMd5&);

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

