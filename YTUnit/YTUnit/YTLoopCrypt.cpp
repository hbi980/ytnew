#include "../../YTPublic/YTInclude/YTUnit/YTLoopCrypt.h"
#include <stdlib.h>

CYTLoopCrypt::CYTLoopCrypt(void)
{
}


CYTLoopCrypt::~CYTLoopCrypt(void)
{
}

// 生成KEY
void CYTLoopCrypt::GenerateKey(unsigned char * key, int len)
{
	for(int i=0; i<len; i++)
	{
		key[i] = rand()%255+1;
	}
}

// 加解密函数（同一个）
void CYTLoopCrypt::Crypt(unsigned char * data, int len, unsigned char * key, int keylen)
{
	int inlen = len-1;
	int i;
	while(inlen > 0)
	{
		for(i=0; i<keylen; i++)
		{
			data[inlen] = key[i]^data[inlen];
			inlen--;
			if(inlen<0)
			{
				break;
			}
		}
	}
}