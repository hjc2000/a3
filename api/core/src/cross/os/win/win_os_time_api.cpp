#include "internal/win_os_common.h"
#include<chrono>
#include<ctime>

using namespace std;
using namespace std::chrono;

#pragma comment(lib,"Winmm.lib") 

static int32_t qf_init = 0;
static LARGE_INTEGER qf_freq;
static long qf_nano_tick;

void convert_tick_to_timespec(LARGE_INTEGER tick, struct timespec *tv)
{
	tv->tv_sec = tick.QuadPart / qf_freq.QuadPart;
	tv->tv_nsec = (long)(tick.QuadPart % qf_freq.QuadPart) * qf_nano_tick;
}

void init_queryperformance()
{
	if (!qf_init)
	{
		QueryPerformanceFrequency(&qf_freq);
		qf_nano_tick = (long)(TIME_SECOND_TO_NS / qf_freq.QuadPart);
		qf_init = 1;
	}
}

vatek_result cross_os_time_eclipse(struct timespec *st, struct timespec *eclipse)
{
	struct timespec curtime;
	vatek_result nres = cross_os_get_time(&curtime);
	if (is_vatek_success(nres))
	{
		if (curtime.tv_nsec > st->tv_nsec)
		{
			eclipse->tv_nsec = st->tv_nsec + (1000000000UL - curtime.tv_nsec);
			eclipse->tv_sec = (curtime.tv_sec - st->tv_sec) - 1;
		}
		else
		{
			eclipse->tv_nsec = curtime.tv_nsec - st->tv_nsec;
			eclipse->tv_sec = curtime.tv_sec - st->tv_sec;
		}
	}
	return nres;
}
