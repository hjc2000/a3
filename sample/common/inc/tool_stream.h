#ifndef _TOOL_STREAM_
#define _TOOL_STREAM_

#include <output_modulator.h>
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"
#include<memory>
#include<functional>
#include<cross_os_api.h>
#include<iostream>

using namespace std;

// 一个 ts 的包长度是 188 字节
#define TS_PACKET_LEN				188

// 用来读取 ts 流的缓冲区的大小
#define TSSLICE_BUFFER_LEN			CHIP_STREAM_SLICE_LEN

// 缓冲区大小除以一个 ts 包的尺寸，得出缓冲区可以储存多少个 ts 包
#define TSSLICE_PACKET_NUM			(TSSLICE_BUFFER_LEN / TS_PACKET_LEN)

/// <summary>
///		被强转成：
///		tool_handle_udp *
///		handle_test *
///		FileWrapper *
/// </summary>
typedef void *void_stream_source;

/// <summary>
///		ts 流源。
///		* ts 流源有：文件流源、UDP 流源。具体的流源派生此类。
/// </summary>
class TsStreamSource
{
public:
	TsStreamSource()
	{
		hsource = this;
	}

	virtual ~TsStreamSource() {}

	void_stream_source hsource = nullptr;

	function<vatek_result(void_stream_source hsource)> start;
	function<vatek_result(void_stream_source hsource)> check;
	function<uint8_t *(void_stream_source hsource)> get;
	function<vatek_result(void_stream_source hsource)> stop;

	virtual vatek_result Start() = 0;
	virtual void Free() = 0;
};

/// <summary>
///		对 C 的 FILE 类型的文件句柄的包装
/// </summary>
class FileTsStreamSource :public TsStreamSource
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
	FileTsStreamSource();

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
	vatek_result lock_ts_file_stream()
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

	vatek_result Start()
	{
		return vatek_success;
	}
	void Free();
};

#define UDP_SLICE_BUF_NUMS		32
class UdpTsStreamSource :public TsStreamSource
{
public:
	UdpTsStreamSource();

	~UdpTsStreamSource()
	{
		cout << "UdpTsStreamSource 析构" << endl;
	}

	void_cross_socket hsocket = nullptr;
	void_cross_thread hrecv = nullptr;
	HANDLE hlock = nullptr;
	int32_t buf_rptr = 0;
	int32_t buf_wptr = 0;

	/// <summary>
	///		定义一个二维数组。因为二维数组内存上是连续的，所以可以当成长度为
	///		UDP_SLICE_BUF_NUMS * CHIP_STREAM_SLICE_LEN 的一维数组使用。这是用来接收 UDP 流
	///		的缓冲区。
	/// </summary>
	uint8_t buf_pool[UDP_SLICE_BUF_NUMS][CHIP_STREAM_SLICE_LEN]{};

	/// <summary>
	///		线程函数当前是否在执行
	/// </summary>
	int32_t isrunning = 0;

	vatek_result Start();
	vatek_result Stop();
	vatek_result Check();
	uint8_t *Get();
	void Free();
};

/// <summary>
///		从文件中获取流
/// </summary>
/// <param name="file">要打开的文件的路径</param>
/// <param name="psource"></param>
/// <returns></returns>
vatek_result stream_source_file_get(const char *file, FileTsStreamSource *psource);
vatek_result stream_source_udp_get(const char *ipaddr, UdpTsStreamSource *psource);

#endif
