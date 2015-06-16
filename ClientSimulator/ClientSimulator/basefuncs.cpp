#include "stdafx.h"
#include "basefuncs.h"

double GetTickCountEX()
{
#ifdef _WIN32 
	LARGE_INTEGER Freq_temp,count;
	QueryPerformanceFrequency(&Freq_temp);
	/*if(!QueryPerformanceFrequency(&Freq_temp))
	{
		printf("doesn't support the high-resolution performance counter");
		return -1;//doesn't support the high-resolution performance counter
	}*/
	//SetThreadAffinityMask(GetCurrentThread(), 1); 
	QueryPerformanceCounter(&count);
	
	return (double)count.QuadPart*1000000/Freq_temp.QuadPart;

#else
	struct timeval tv_time;
    gettimeofday(&tv_time,NULL);
    return tv_time.tv_sec*1000000 + tv_time.tv_usec;

#endif
}