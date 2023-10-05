#ifndef _TOOL_TSPACKET_
#define _TOOL_TSPACKET_

#include <stdint.h>
#include <mux/mux_define.h>

#define TSPACKET_PCR_PID		0x100

#ifdef __cplusplus
extern "C" {
#endif

	uint8_t* tspacket_get_pcr(Pmux_time_tick ptime);
	uint8_t* tspacket_get_suffing(void);

	/// <summary>
	///		获取 ts 的空包。
	/// </summary>
	/// <param name=""></param>
	/// <returns>指向空包的指针，是一个一维数组的头指针。</returns>
	uint8_t* tspacket_get_null(void);

#ifdef __cplusplus
}
#endif


#endif
