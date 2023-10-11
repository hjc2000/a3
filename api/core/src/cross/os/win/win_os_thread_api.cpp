#include "internal/win_os_common.h"
#include<Exception.h>

struct win_thread
{
	cross_thread_param param{};
	HANDLE hhandle{};
	DWORD threadid{};
};

void_cross_thread cross_os_create_thread(fpcross_thread_function fpfun, void *userparam)
{
	win_thread *newthread = new win_thread;
	if (!newthread)
	{
		throw Exception{};
	}

	memset(newthread, 0, sizeof(win_thread));
	newthread->param.void_userparam = userparam;

	newthread->hhandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fpfun, &newthread->param, 0, &newthread->threadid);
	if (newthread->hhandle != INVALID_HANDLE_VALUE)
		return newthread;

	delete newthread;
	return NULL;
}

vatek_result cross_os_free_thread(void_cross_thread hthread)
{
	win_thread *winthread = (win_thread *)hthread;
	vatek_result nres = cross_os_join_thread(hthread);
	CloseHandle(winthread->hhandle);
	delete winthread;
	return nres;
}

vatek_result cross_os_join_thread(void_cross_thread hthread)
{
	win_thread *winthread = (win_thread *)hthread;
	uint32_t nres = WaitForSingleObject(winthread->hhandle, 1000);
	if (nres == WAIT_OBJECT_0)
		return winthread->param.result;
	else if (nres == WAIT_TIMEOUT)
	{
		cross_os_printf("WaitForSingleObject timeout");
		return vatek_timeout;
	}
	cross_os_printf("WaitForSingleObject unknown : %d", nres);
	return vatek_unknown;
}

vatek_result cross_os_get_thread_result(void_cross_thread hthread)
{
	win_thread *winthread = (win_thread *)hthread;
	return winthread->param.result;
}
