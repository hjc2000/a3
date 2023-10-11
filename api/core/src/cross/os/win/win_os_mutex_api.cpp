#include "internal/win_os_common.h"

vatek_result cross_os_create_mutex_name(const char *tag, void_cross_mutex *hmutex)
{
	HANDLE newmux = INVALID_HANDLE_VALUE;
	vatek_result nres = vatek_memfail;

	newmux = CreateMutexA(NULL, FALSE, win_get_global_name(tag));
	if (newmux != INVALID_HANDLE_VALUE)
	{
		*hmutex = newmux;
		nres = vatek_success;
	}
	else
		win_get_last_error();
	return nres;
}

vatek_result cross_os_open_mutex_name(const char *tag, void_cross_mutex *hmuxtex)
{
	HANDLE newmux = INVALID_HANDLE_VALUE;
	newmux = OpenMutexA(0, FALSE, win_get_global_name(tag));
	if (newmux != INVALID_HANDLE_VALUE)
	{
		*hmuxtex = newmux;
		return vatek_success;
	}
	else
		win_get_last_error();
	return vatek_memfail;
}

vatek_result cross_os_create_mutex(void_cross_mutex *hmutex)
{
	HANDLE newmutex = CreateMutexA(NULL, FALSE, NULL);
	vatek_result nres = vatek_memfail;
	if (newmutex != INVALID_HANDLE_VALUE)
	{
		*hmutex = newmutex;
		nres = vatek_success;
	}
	else
		win_get_last_error();
	return nres;
}

void cross_os_lock_mutex(void_cross_mutex hmutex)
{
	HANDLE hmux = (HANDLE)hmutex;
	uint32_t nres = WaitForSingleObject(hmux, INFINITE);

	if (nres != WAIT_OBJECT_0)
		cross_os_printf("lock mutex fail : 0x%08x", hmutex);
}

vatek_result cross_os_lock_mutex_timeout(void_cross_mutex hmutex, uint32_t ms)
{
	HANDLE hmux = (HANDLE)hmutex;
	uint32_t nres = WaitForSingleObject(hmux, ms);

	if (nres == WAIT_OBJECT_0)
		return vatek_success;
	else if (nres == WAIT_TIMEOUT)
		return vatek_timeout;
	else
	{
		cross_os_printf("lock mutex fail : 0x%08x", nres);
		return vatek_unknown;
	}
}

vatek_result cross_os_trylock_mutex(void_cross_mutex hmutex)
{
	HANDLE hmux = (HANDLE)hmutex;
	uint32_t nres = WaitForSingleObject(hmux, 0);

	if (nres == WAIT_OBJECT_0)
		return vatek_success;
	else if (nres == WAIT_TIMEOUT)
		return vatek_timeout;
	return vatek_unknown;
}

void cross_os_release_mutex(void_cross_mutex hmutex)
{
	if (!ReleaseMutex((HANDLE)hmutex))win_get_last_error();
}

vatek_result cross_os_free_mutex(void_cross_mutex hmutex)
{
	CloseHandle((HANDLE)hmutex);
	return vatek_success;
}
