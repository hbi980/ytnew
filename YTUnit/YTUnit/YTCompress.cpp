#include "../../YTPublic/YTInclude/YTUnit/YTCompress.h"


CYTCompress::CYTCompress(void)
{
}

CYTCompress::~CYTCompress(void)
{
}

// —πÀı
int CYTCompress::Compress(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen)
{
	if(inlen > 8192)
	{
		return -1;
	}
	
	unsigned char * buf = (unsigned char *)inbuf;
	Init();
	memset(outbuf, 0, outlen);
	unsigned short limitsize = min(outlen, inlen);

	int	errorcode;
	int inputptr = 1;
	int charecter;
	unsigned int index;
	int	nextcode = 258;
	unsigned int outbitptr = 0;
	unsigned int outbitsize = 9;
	int	nextbumpsize = 512;
	int stringcode = (int)buf[0];
	while(inputptr < inlen)
	{
		charecter = (int)buf[inputptr];
		index = FindChildNode(stringcode, charecter);
		if(index == (unsigned int)(-1))
		{
			return -1;
		}

		if(m_dictory[index].codevalue != -1)
		{
			stringcode = (int)m_dictory[index].codevalue;
		}
		else
		{
			m_dictory[index].codevalue = (short)nextcode;
			m_dictory[index].parentcode = (short)stringcode;
			m_dictory[index].character = (char)charecter;

			if((errorcode = PutBit(outbuf, limitsize, outbitptr, stringcode, outbitsize)) < 0)
			{
				return errorcode;
			}

			nextcode++;
			stringcode = charecter;
			outbitptr += outbitsize;

			if(nextcode >= nextbumpsize)
			{
				if((errorcode = PutBit(outbuf, limitsize, outbitptr, (unsigned int)257, outbitsize)) < 0)
				{
					return errorcode;
				}

				outbitptr += outbitsize;
				outbitsize++;
				nextbumpsize = nextbumpsize << 1;
			}
		}

		inputptr++;
	}

	if((errorcode = PutBit(outbuf, limitsize, outbitptr, stringcode, outbitsize)) < 0)
	{
		return errorcode;
	}

	if((errorcode = PutBit(outbuf, limitsize, outbitptr+outbitsize, 256, outbitsize)) < 0)
	{
		return errorcode;
	}

	outbitptr += outbitsize + outbitsize;

	int retsize = 0;
	if((outbitptr % 8) != 0)
	{
		retsize = (outbitptr >> 3) + 1;
	}
	else
	{
		retsize = (outbitptr >> 3);
	}

	if(retsize >= inlen)
	{
		return -1;
	}
	else
	{
		return retsize;
	}
}

// Ω‚—πÀı
int CYTCompress::Expand(char * inbuf, unsigned short inlen, char * outbuf, unsigned short outlen)
{
	int errorcode;
	unsigned int oldcode;
	if((errorcode = GetBit(inbuf, inlen, 0, &oldcode, 9)) < 0)
	{
		return errorcode;
	}
	
	outbuf[0] = (char)oldcode;
	int character = (char)oldcode;
	
	unsigned int inbitptr = 9;
	unsigned int newcode;
	unsigned int inbitsize = 9;
	unsigned short outptr = 1;
	int	nextcode = 258;
	unsigned int count;
	while((errorcode = GetBit(inbuf, inlen, inbitptr, &newcode, inbitsize)) > 0)
	{
		if(newcode == 256)
		{
			return outptr;
		}
		
		if(newcode == 257)
		{
			inbitptr += inbitsize;
			inbitsize++;
			continue;
		}
		
		inbitptr += inbitsize;
		
		if((int)newcode >= nextcode)
		{
			m_stack[0] = (char)character;
			count = DecodeString(1, oldcode);
		}
		else
		{
			count = DecodeString(0, newcode);
		}
		
		if(count < 0)
		{
			return -1;
		}
		else if(count > 9973)
		{
			return -1;
		}

		character = m_stack[count-1];
		
		while(count > 0)
		{
			if(outptr >= outlen)
			{
				return -1;
			}
			
			outbuf[outptr] = m_stack[count-1];
			count--;
			outptr++;
		}

		if(nextcode >= 9973)
		{
			return -1;
		}
		
		m_dictory[nextcode].parentcode = (short)oldcode;
		m_dictory[nextcode].character = (char)character;
		nextcode++;
		
		oldcode = newcode;
	}
	
	return errorcode;
}

void CYTCompress::Init()
{
	for(int i=0; i<9973; i++)
	{
		m_dictory[i].codevalue = -1;
		m_dictory[i].parentcode = 0;
		m_dictory[i].character = 0;
	}
}

unsigned int CYTCompress::FindChildNode(int parentcode, int childcode)
{
	int offset;
	unsigned int index = (childcode << (13 - 8)) ^ parentcode;
	if(index == 0)
	{
		offset = 1;
	}
	else
	{
		offset = 9973-index;
	}

	int	itrytimes = 0;
	while(1)
	{
		if(m_dictory[index].codevalue == -1)
		{
			return index;
		}

		if(m_dictory[index].parentcode==(short)parentcode && m_dictory[index].character==(char)childcode)
		{
			return index;
		}

		if((int)index >= offset)
		{
			index -= offset;
		}
		else
		{
			index += 9973-offset;
		}

		itrytimes++;
		if(itrytimes > (9973+1))
		{
			return (unsigned int)(-1);
		}
	}
}

unsigned int CYTCompress::DecodeString(unsigned int count, unsigned int code)
{
	while(code > 255)
	{
		if(count>=9973 || code>=9973)
		{
			return -1;
		}
		m_stack[count] = m_dictory[code].character;
		code = (short)m_dictory[code].parentcode;
		count++;
	}

	m_stack[count] = (char)code;
	count++;
	return count;
}

int CYTCompress::PutBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int value, unsigned int size)
{
	unsigned int byteoffset = offset >> 3;
	if(byteoffset >= inlen)
	{
		return -1;
	}

	unsigned int bytestart = offset - ((offset >> 3) << 3);
	if((8-bytestart) >= size)
	{
		inbuf[byteoffset] = (unsigned char)(inbuf[byteoffset] | (value << (8 - bytestart - size)));
		return 1;
	}
	else
	{
		inbuf[byteoffset] = (unsigned char)(inbuf[byteoffset] | (value >> (size - 8 + bytestart)));
		offset += 8 - bytestart;
		size -= 8 - bytestart;
		value = value & (0xFFFFFFFF >> (32 - size));
		return PutBit(inbuf, inlen, offset, value, size);
	}
}

int CYTCompress::GetBit(char * inbuf, unsigned short inlen, unsigned int offset, unsigned int * value, unsigned int size)
{
	unsigned int byteoffset = offset >> 3;
	if(byteoffset >= inlen)
	{
		return -1;
	}

	unsigned int bytestart = offset - ((offset >> 3) << 3);
	if((8-bytestart) >= size)
	{
		unsigned char invalue = inbuf[byteoffset];
		*value = (unsigned int)((invalue & (0xFF >> bytestart)) >> (8-bytestart-size));
		return 1;
	}
	else
	{
		unsigned char invalue = inbuf[byteoffset];
		*value = (invalue & (0xFF >> bytestart));

		offset += 8-bytestart;
		size -= 8-bytestart;

		unsigned int tmpvalue;
		int errorcode;
		if((errorcode = GetBit(inbuf, inlen, offset, &tmpvalue, size)) < 0)
		{
			return -1;
		}
		
		*value = ((*value) << size) | tmpvalue;
		return 1;
	}
}