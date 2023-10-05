
#ifndef _TOOL_STREAM_
#define _TOOL_STREAM_

#include <core/base/output_modulator.h>

#define TS_PACKET_LEN										188
#define TSSLICE_BUFFER_LEN			CHIP_STREAM_SLICE_LEN
#define TSSLICE_PACKET_NUM			(TSSLICE_BUFFER_LEN/TS_PACKET_LEN)

typedef void* hstream_source;
typedef vatek_result(*fpstream_source_start)(hstream_source hsource);
typedef vatek_result(*fpstream_source_check)(hstream_source hsource);
typedef uint8_t*(*fpstream_source_get)(hstream_source hsource);
typedef vatek_result(*fpstream_source_stop)(hstream_source hsource);
typedef void(*fpstream_source_free)(hstream_source hsource);

/// <summary>
///		ts 流源。内部定义了一个 hstream_source 类型的字段和一些函数指针。
///		这些函数指针的第一个参数全部是 hstream_source hsource。
///		这个结构体就是在模仿 C++ 的类。
/// </summary>
typedef struct _tsstream_source
{
	hstream_source hsource;
	fpstream_source_start start;
	fpstream_source_check check;
	fpstream_source_get get;
	fpstream_source_stop stop;
	fpstream_source_free free;
}tsstream_source,*Ptsstream_source;

#ifdef __cplusplus
extern "C" {
#endif

	/// <summary>
	///		初始化 psource。会为 psource 的 hsource 字段赋值，并为 psource 的函数指针赋值。
	/// </summary>
	/// <param name="pmod">
	///		会使用 pmod 计算 bitrate，然后用 bitrate 计算一些参数，用来初始化一个 handle_test 类型的对象，
	///		最后将这个 handle_test 对象赋值给 psource 的 hsource 字段。
	/// </param>
	/// <param name="psource">此对象会被初始化</param>
	/// <returns></returns>
	vatek_result stream_source_test_get(Pmodulator_param pmod, Ptsstream_source psource);

	/// <summary>
	///		从文件中获取流
	/// </summary>
	/// <param name="file">要打开的文件的路径</param>
	/// <param name="psource"></param>
	/// <returns></returns>
	vatek_result stream_source_file_get(const char* file, Ptsstream_source psource);
	vatek_result stream_source_udp_get(const char* ipaddr, Ptsstream_source psource);

#ifdef __cplusplus
}
#endif

#endif
