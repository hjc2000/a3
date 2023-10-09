#include "../inc/tool_stream.h"

/// <summary>
///		流开始。其实就是将 hsource 内表示时间的两个字段赋值为 0
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern vatek_result test_stream_start(hstream_source hsource);

/// <summary>
/// 
/// </summary>
/// <param name="hsource"></param>
/// <returns>将 1 强制转换为 vatek_result 类型</returns>
extern vatek_result test_stream_check(hstream_source hsource);


extern uint8_t *test_stream_get(hstream_source hsource);

/// <summary>
/// 
/// </summary>
/// <param name="hsource"></param>
/// <returns>直接返回 vatek_success</returns>
extern vatek_result test_stream_stop(hstream_source hsource);

/// <summary>
///		在堆中释放 hsource
/// </summary>
/// <param name="hsource"></param>
extern void test_stream_free(hstream_source hsource);

vatek_result stream_source_test_get(Pmodulator_param pmod, tsstream_source *stream_source)
{
	uint32_t bitrate = modulator_param_get_bitrate(pmod);
	handle_test *ptest = (handle_test *)malloc(sizeof(handle_test));
	if (!ptest)
	{
		throw - 1;
	}

	memset(ptest, 0, sizeof(handle_test));
	ptest->slice_ns = (1000000000 / (bitrate / (TS_PACKET_LEN * 8))) * CHIP_STREAM_SLICE_PACKET_NUMS;
	_disp_l("open test_stream : %d bps - %d ns", bitrate, ptest->slice_ns);

	stream_source->hsource = ptest;
	stream_source->start = test_stream_start;
	stream_source->stop = test_stream_stop;
	stream_source->check = test_stream_check;
	stream_source->get = test_stream_get;
	stream_source->free = test_stream_free;
	return vatek_success;
}

vatek_result test_stream_start(hstream_source hsource)
{
	handle_test *ptest = (handle_test *)hsource;
	ptest->time.ms = 0;
	ptest->time.ns = 0;
	return vatek_success;
}

vatek_result test_stream_check(hstream_source hsource)
{
	return (vatek_result)1;
}

uint8_t *test_stream_get(hstream_source hsource)
{
	handle_test *ptest = (handle_test *)hsource;
	int32_t nums = 0;
	uint8_t *ptr = &ptest->buffer[0];

	while (nums < CHIP_STREAM_SLICE_PACKET_NUMS)
	{
		if (nums == 0)
			memcpy(ptr, tspacket_get_pcr(&ptest->time), TS_PACKET_LEN);
		else
			memcpy(ptr, tspacket_get_suffing(), TS_PACKET_LEN);

		ptr += TS_PACKET_LEN;
		nums++;
	}

	ptest->time.ns += ptest->slice_ns;

	// 纳秒进位到毫秒
	if (ptest->time.ns >= 1000000)
	{
		ptest->time.ms += ptest->time.ns / 1000000;
		ptest->time.ns = ptest->time.ns % 1000000;
	}

	return &ptest->buffer[0];
}

vatek_result test_stream_stop(hstream_source hsource)
{
	return vatek_success;
}

void test_stream_free(hstream_source hsource)
{
	free(hsource);
}
