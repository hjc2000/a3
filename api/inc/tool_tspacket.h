#ifndef _TOOL_TSPACKET_
#define _TOOL_TSPACKET_

#include <stdint.h>
#include <mux_define.h>

#define TSPACKET_PCR_PID		0x100

#ifdef __cplusplus
extern "C" {
#endif

	/// <summary>
	///		传入时间，将构造一个伪造的 ts 包，里面的 PCR 信息的时间就是传进去的时间。
	/// </summary>
	/// <param name="ptime"></param>
	/// <returns>伪造的 ts 包的头指针</returns>
	uint8_t* tspacket_get_pcr(Pmux_time_tick ptime);

	/// <summary>
	///		内部准备了 2 个 ts 包，每次调用交替切换返回的 ts 包。
	/// </summary>
	/// <param name=""></param>
	/// <returns>返回内部预先准备好的 ts 包的头指针</returns>
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
