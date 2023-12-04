#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include<sys/time.h>

// 29ns
static long long GetTime()
{
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec * 1000000000 + tp.tv_nsec;
}

// 30ns
static long long GetVdSoTime()
{
	struct timeval timeVal;
	::gettimeofday(&timeVal, NULL);
	return timeVal.tv_sec * 1000000 + timeVal.tv_usec;
}

// 10ns
static long long GetNativeRdTsc()
{
	unsigned int low, high;
	__asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
	return (long long)(low) | ((long long)(high) << 32);
}

static long long GetAtomicRdTsc()
{
	unsigned long long var;
	__asm__ volatile ("lfence\n\t"
	                 "rdtsc\n\t"  
	                 "shl $32,%%rdx;"
	                 "or %%rdx,%%rax;"
	                 "mov %%rax, %0;"
	                 "lfence\n\t":"=r"(var)
	                 ::"%rax", "%rbx", "%rcx", "%rdx");

	return var;
}

static long long _beginTime = 0;
static long long _beginRdtsc = 0;
static long long _freqPerNano = 0;

// 20ns
static long long GetTimeByRdTsc()
{
	return _beginTime + (GetNativeRdTsc() - _beginRdtsc) / _freqPerNano;
}

int main()
{
    _beginTime = GetTime();
    auto rdtscStart = GetAtomicRdTsc();
    ::sleep(1);
    auto rdtscEnd = GetAtomicRdTsc();
    _freqPerNano = (rdtscEnd - rdtscStart)/1000000000;
    _freqPerNano = (_freqPerNano == 0) ? 1 : _freqPerNano;
    _beginTime = GetTime();
    _beginRdtsc = GetAtomicRdTsc();

    auto startTime = GetTime();
    const int execTimes = 5;
    const int maxLoop = 100000000;
    for(int count = 0; count < execTimes; ++count)
    {
        auto startTime = GetTime();

        for(int idx = 0; idx < maxLoop; ++idx)
	          GetTime();
        
	auto endTime = GetTime();
	printf("GetTime use time:%lld, loop:%d\n", (endTime - startTime), maxLoop);
    }

    for(int count = 0; count < execTimes;++count)
    {
	auto startTime = GetTime();

	for(int idx=0; idx<maxLoop; ++idx)
            GetVdSoTime();

	 auto endTime = GetTime();
	printf("GetVdSoTime use time:%lld, loop:%d\n", (endTime - startTime), maxLoop);

    }
    for(int count = 0; count < execTimes;++count)
    {
       auto startTime = GetTime();

        for(int idx=0; idx<maxLoop; ++idx)
	            GetNativeRdTsc();

        auto endTime = GetTime();
        printf("GetNativeRdTsc use time:%lld, loop:%d\n", (endTime - startTime), maxLoop);

    }

    for(int count = 0; count < execTimes;++count)
    {
	 auto startTime = GetTime();
         for(int idx=0; idx<maxLoop; ++idx)
		 GetTimeByRdTsc();

         auto endTime = GetTime();
         printf("GetTimeByRdTsc use time:%lld, loop:%d\n", (endTime - startTime), maxLoop);

    }

    // rdtsc对时不准会多跑10几秒不适合做高精度定时,而适合做精度不高的代码测试
    for(int count = 0; count < 100; ++count)
    {
	    sleep(1);
	    auto clockTime = GetTime();
	    auto rdtscTime = GetTimeByRdTsc();
	    printf("clock time:%lld, rdtscTime:%lld\n", clockTime, rdtscTime);
    }
    return 0;    
}
