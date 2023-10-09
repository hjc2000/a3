
#ifndef _TOOL_STREAM_
#define _TOOL_STREAM_

#include <core/base/output_modulator.h>
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"

// 一个 ts 的包长度是 188 字节
#define TS_PACKET_LEN				188

// 用来读取 ts 流的缓冲区的大小
#define TSSLICE_BUFFER_LEN			CHIP_STREAM_SLICE_LEN

// 缓冲区大小除以一个 ts 包的尺寸，得出缓冲区可以储存多少个 ts 包
#define TSSLICE_PACKET_NUM			(TSSLICE_BUFFER_LEN / TS_PACKET_LEN)

typedef void *hstream_source;
typedef vatek_result(*fpstream_source_start)(hstream_source hsource);
typedef vatek_result(*fpstream_source_check)(hstream_source hsource);
typedef uint8_t *(*fpstream_source_get)(hstream_source hsource);
typedef vatek_result(*fpstream_source_stop)(hstream_source hsource);
typedef void(*fpstream_source_free)(hstream_source hsource);

struct handle_test
{
	mux_time_tick time;
	uint32_t slice_ns;
	int32_t file_size;
	uint8_t buffer[CHIP_STREAM_SLICE_LEN];
};

/// <summary>
///		ts 流源。内部定义了一个 hstream_source 类型的字段和一些函数指针。
///		这些函数指针的第一个参数全部是 hstream_source hsource。
///		这个结构体就是在模仿 C++ 的类。
/// </summary>
class tsstream_source
{
public:
	hstream_source hsource = nullptr;
	fpstream_source_start start;
	fpstream_source_check check;
	fpstream_source_get get;
	fpstream_source_stop stop;
	fpstream_source_free free;
};

/// <summary>
///		对 C 的 FILE 类型的文件句柄的包装
/// </summary>
class FileWrapper
{
public:
	/// <summary>
	///		在 file_lock 中会被赋值为一个 ts 包的长度。有可能是 188 或 204.
	/// </summary>
	int32_t packet_len;
	int32_t file_size;

	/// <summary>
	///		C 的文件句柄
	/// </summary>
	FILE *fhandle;
	uint8_t buffer[CHIP_STREAM_SLICE_LEN];

	/// <summary>
	///		锁定 ts 流。
	///		* 总共会读取 2 个包，检查同步字节。如果成功同步到 2 个包，就认为锁定成功。
	///		* 锁定成功后会将文件指针恢复到原来的位置。
	/// </summary>
	/// <param name="pfile"></param>
	/// <returns>成功返回 0，失败返回错误代码</returns>
	vatek_result file_lock();
};

/// <summary>
///		初始化 psource。会为 psource 的 hsource 字段赋值，并为 psource 的函数指针赋值。
/// </summary>
/// <param name="pmod">
///		会使用 pmod 计算 bitrate，然后用 bitrate 计算一些参数，用来初始化一个 handle_test 类型的对象，
///		最后将这个 handle_test 对象赋值给 psource 的 hsource 字段。
/// </param>
/// <param name="psource">此对象会被初始化</param>
/// <returns></returns>
vatek_result stream_source_test_get(Pmodulator_param pmod, tsstream_source *psource);

/// <summary>
///		从文件中获取流
/// </summary>
/// <param name="file">要打开的文件的路径</param>
/// <param name="psource"></param>
/// <returns></returns>
vatek_result stream_source_file_get(const char *file, tsstream_source *psource);
vatek_result stream_source_udp_get(const char *ipaddr, tsstream_source *psource);

#endif
