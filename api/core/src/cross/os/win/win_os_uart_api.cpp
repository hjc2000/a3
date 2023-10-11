#include "internal/win_os_common.h"

int32_t try_getchar(void)
{
	int nchar = -1;
	if (_kbhit())
	{
		nchar = _getch();
		if (nchar == 8)nchar = 0x7F;
	}
	return nchar;
}

void cross_os_error(const char *fmt, ...)
{
	char szlog[256];
	va_list va;
	va_start(va, fmt);
	vsprintf(szlog, fmt, va);
	va_end(va);
	OutputDebugStringA(szlog);
	OutputDebugStringA("\r\n");
	while (1);
}