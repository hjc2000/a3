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
	if (path[len - 1] != PATH_BACKSLASH)path[len++] = PATH_BACKSLASH;
	strcpy(&path[len], append);
}
