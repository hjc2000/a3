#include "win_os_common.h"

vatek_result cross_os_create_mutex(HANDLE *hmutex)
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

void cross_os_lock_mutex(HANDLE hmutex)
{
	HANDLE hmux = (HANDLE)hmutex;
	uint32_t nres = WaitForSingleObject(hmux, INFINITE);

	if (nres != WAIT_OBJECT_0)
		cross_os_printf("lock mutex fail : 0x%08x", hmutex);
}

vatek_result cross_os_lock_mutex_timeout(HANDLE hmutex, uint32_t ms)
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

vatek_result cross_os_trylock_mutex(HANDLE hmutex)
{
	HANDLE hmux = (HANDLE)hmutex;
	uint32_t nres = WaitForSingleObject(hmux, 0);

	if (nres == WAIT_OBJECT_0)
		return vatek_success;
	else if (nres == WAIT_TIMEOUT)
		return vatek_timeout;
	return vatek_unknown;
}

void cross_os_release_mutex(HANDLE hmutex)
{
	if (!ReleaseMutex((HANDLE)hmutex))win_get_last_error();
}

vatek_result cross_os_free_mutex(HANDLE hmutex)
{
	CloseHandle((HANDLE)hmutex);
	return vatek_success;
}
