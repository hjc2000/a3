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

vatek_result cross_os_get_time(struct timespec *tp)
{
	LARGE_INTEGER curtime;
	init_queryperformance();
	QueryPerformanceCounter(&curtime);
	convert_tick_to_timespec(curtime, tp);
	return (vatek_result)0;
}

uint64_t cross_os_get_time_us()
{
	LARGE_INTEGER curtime;
	init_queryperformance();
	QueryPerformanceCounter(&curtime);
	return ((uint64_t)(curtime.QuadPart * TIME_SECOND_TO_US) / qf_freq.QuadPart);
}

uint64_t cross_os_time_to_us(struct timespec *tp)
{
	return (tp->tv_sec * 1000000) + (tp->tv_nsec / 1000);
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

void cross_os_time_plus_ms(struct timespec *tp, int32_t ms)
{
	if (ms >= 1000)
	{
		tp->tv_sec += ms / 1000;
		ms %= 1000;
	}

	tp->tv_nsec += ms * 1000;
	if (tp->tv_nsec >= TIME_SECOND_TO_NS)
	{
		tp->tv_sec++;
		tp->tv_nsec -= TIME_SECOND_TO_NS;
	}
}

void cross_os_time_plus_time(struct timespec *tp, struct timespec *tappend)
{
	tp->tv_nsec += tappend->tv_nsec;
	tp->tv_sec += tappend->tv_sec;

	if (tp->tv_nsec > TIME_SECOND_TO_NS)
	{
		tp->tv_sec++;
		tp->tv_nsec -= TIME_SECOND_TO_NS;
	}
}
