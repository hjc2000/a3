#include<cross/cross_os_api.h>
#include<chrono>
#include<iostream>

using namespace std;
using namespace std::chrono;

HAL_API void cross_os_sleep(int32_t ms)
{
	this_thread::sleep_for(milliseconds(ms));
}

HAL_API void cross_os_usleep(uint32_t us)
{
	this_thread::sleep_for(microseconds(us));
}

void cross_os_yield()
{
	this_thread::yield();
}

#ifdef _MSC_VER
#define PATH_BACKSLASH	'\\'
#else
#define PATH_BACKSLASH	'/'
#endif

void cross_os_append_path(char *path, const char *append)
{
	int32_t len = (int32_t)strlen(path);
	if (path[len - 1] != PATH_BACKSLASH)
		path[len++] = PATH_BACKSLASH;
	strcpy(&path[len], append);
}

uint32_t cross_os_get_tick_ms(void)
{
	duration since_epoch = system_clock::now().time_since_epoch();
	int64_t millis = duration_cast<milliseconds>(since_epoch).count();
	return millis;
}

void cross_os_wait_unit(timespec *target)
{
	uint64_t curtick = cross_os_get_time_us();
	uint64_t timetick = cross_os_time_to_us(target);

	if (curtick > timetick)
		return;
	curtick = timetick - curtick;
	cross_os_usleep((uint32_t)curtick);
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
