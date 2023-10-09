
#ifndef _TOOL_STREAM_
#define _TOOL_STREAM_

#include <core/base/output_modulator.h>
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"
#include<memory>

using namespace std;

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
	/// <summary>
	///		将文件指针移动到以文件开始为参考点的 pos + offset 处，然后读取 1 个字节，检查是否是 ts
	///		的同步字节。
	/// </summary>
	/// <param name="hfile"></param>
	/// <param name="pos"></param>
	/// <param name="offset"></param>
	/// <returns>
	///		是同步字节返回 1，不是同步字节或发生了错误则返回错误代码。
	///		如果是格式错误，返回的错误代码为 vatek_format。
	/// </returns>
	vatek_result file_check_sync(int32_t pos, int32_t offset)
	{
		// seek 成功返回 0，失败返回 -1
		vatek_result nres = (vatek_result)fseek(fhandle, pos + offset, SEEK_SET);
		if (nres)
		{
			// seek 失败，返回 -1
			return (vatek_result)-1;
		}

		uint8_t tag = 0;

		// 读取 1 个字节
		nres = (vatek_result)fread(&tag, 1, 1, fhandle);
		if (nres != 1)
		{
			// 没有成功读取到 1 个字节
			return vatek_hwfail;
		}

		// 检查读取到的 1 个字节是否是 ts 的同步字节
		if (tag == TS_PACKET_SYNC_TAG)
		{
			// 是同步字节
			return (vatek_result)1;
		}

		// 不是同步字节，返回格式错误的错误代码
		return vatek_format;
	}

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
	vatek_result lock_file_stream()
	{
		vatek_result nres = vatek_badstatus;
		uint8_t sync;
		uint32_t count = 0;

		for (;;)
		{
			// 获取当前文件指针
			size_t pos = ftell(fhandle);

			/* 读取 1 个字节，返回值为成功读取的元素个数，而不是字节数。在这里，一个元素就是 1 个字节，所以元素数
			* 等于字节数。
			*/
			nres = (vatek_result)fread(&sync, 1, 1, fhandle);
			if (nres != 1)
			{
				// 不等于 1，说明没有成功读取到 1 个字节，返回错误代码
				return vatek_hwfail;
			}

			// 成功读取到 1 个字节，检查读取到的 1 个字节是否是 ts 的同步字节
			if (sync == TS_PACKET_SYNC_TAG)
			{
				// 如果是同步字节
				packet_len = 0;

				/* pos 的值是一进入循环就第一时间获取了，此时还没有读取任何一个字节。执行到这里说明读取了 1 个
				* 字节，并且这个字节是同步字节。
				*
				* 调用 file_check_sync 函数后，内部会以文件开头为参考点，seek 一段 pos + TS_PACKET_LEN 的距离，
				* 因为 pos 指向的是同步字节的位置，而 TS_PACKET_LEN 等于一个 ts 包的长度，
				* 所以 seek 后文件指针又是处于同步字节的位置。然后 file_check_sync 会读取 1 个字节并检查是否是
				* 同步字节。如果是同步字节，加上刚才读取的同步字节，总共同步到 2 个包。
				*/
				nres = file_check_sync((int32_t)pos, TS_PACKET_LEN);
				if (is_vatek_success(nres))
				{
					// 是同步字节
					packet_len = TS_PACKET_LEN;
				}
				else
				{
					// 不是同步字节，可能是因为此 ts 流的包大小不是 188 字节，而是 204 字节，再试一次。
					nres = file_check_sync((int32_t)pos, 204);

					if (is_vatek_success(nres))
					{
						// 再试一次后如果是同步字节
						packet_len = 204;
					}
				}

				if (packet_len != 0)
				{
					// pfile->packet_len != 0 说明前面的 file_check_sync 成功了
					// 成功后 seek 回原来的位置。将 seek 的结果作为返回值。成功为 0，失败为 -1.
					nres = (vatek_result)fseek(fhandle, (int32_t)pos, SEEK_SET);
					return nres;
				}
			}

			// 没有读取到第 1 个同步字节就递增计数，等下一次循环再次读取下一个字节
			count++;
			// 计数溢出后还没锁定到 ts 流，就超时
			if (count > 1000)
				return vatek_timeout;
		}
	}
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
