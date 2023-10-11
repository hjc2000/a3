
#include <cross/cross_os_api.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>


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
