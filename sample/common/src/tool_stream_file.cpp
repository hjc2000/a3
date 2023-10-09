#include "../inc/tool_stream.h"
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"


/// <summary>
///		直接返回 vatek_success，其实就是 0
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern vatek_result file_stream_start(hstream_source hsource);
extern vatek_result file_stream_check(hstream_source hsource);

/// <summary>
///		将 hsource 强制转换为 handle_file * 后返回 buffer 字段。这是一个一维数组的头指针。
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern uint8_t *file_stream_get(hstream_source hsource);

/// <summary>
///		
/// </summary>
/// <param name="hsource"></param>
/// <returns>直接返回 vatek_success</returns>
extern vatek_result file_stream_stop(hstream_source hsource);

/// <summary>
///		关闭 hsource 内的 fhandle 字段指向的文件，然后释放 hsource 对象
/// </summary>
/// <param name="hsource"></param>
extern void file_stream_free(hstream_source hsource);

/// <summary>
///		文件句柄。其实是对 C 的 FILE 类型的文件句柄的包装
/// </summary>
struct handle_file
{
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
};

/// <summary>
///		锁定 ts 流。
///		* 总共会读取 2 个包，检查同步字节。如果成功同步到 2 个包，就认为锁定成功。
///		* 锁定成功后会将文件指针恢复到原来的位置。
/// </summary>
/// <param name="pfile"></param>
/// <returns>成功返回 0，失败返回错误代码</returns>
extern vatek_result file_lock(handle_file *pfile);

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
extern vatek_result file_check_sync(FILE *hfile, int32_t pos, int32_t offset);

vatek_result stream_source_file_get(const char *file, tsstream_source *psource)
{
	handle_file *pfile = new handle_file;
	if (!pfile)
	{
		// 内存分配失败
		return vatek_memfail;
	}

	/* 打开文件，将文件句柄放到刚才分配的 handle_file 里面。
	* 打开方式：读写，二进制
	*/
	pfile->fhandle = fopen(file, "rb+");
	vatek_result nres = vatek_format;
	if (pfile->fhandle)
	{
		// 通过 seek 到文件末尾来获取文件大小
		fseek(pfile->fhandle, 0, SEEK_END);
		pfile->file_size = (int32_t)ftell(pfile->fhandle);
		// 刚才 seek 到文件末尾了，现在要 seek 回文件开始
		fseek(pfile->fhandle, 0, SEEK_SET);

		// 锁定 ts 流
		nres = file_lock(pfile);
		if (!is_vatek_success(nres))
		{
			fclose(pfile->fhandle);
			delete pfile;
		}
		else
		{
			psource->hsource = pfile;
			psource->start = file_stream_start;
			psource->stop = file_stream_stop;
			psource->get = file_stream_get;
			psource->check = file_stream_check;
			psource->free = file_stream_free;
			_disp_l("open file - [%s] - packet length:%d - packet size:%d", file, pfile->packet_len, pfile->file_size);
			printf("\r\n");
		}
	}

	if (!is_vatek_success(nres))
		_disp_err("file not current ts format - [%s]", file);

	return nres;
}

vatek_result file_stream_start(hstream_source hsource)
{
	return vatek_success;
}

vatek_result file_stream_check(hstream_source hsource)
{
	handle_file *pfile = (handle_file *)hsource;
	int32_t pos = 0;
	uint8_t *ptr = &pfile->buffer[0];
	vatek_result nres = vatek_success;

	/* CHIP_STREAM_SLICE_PACKET_NUMS 是用缓冲区大小除以 188，即计算缓冲区能容纳多少个
	* ts 包。
	*/
	while (pos < CHIP_STREAM_SLICE_PACKET_NUMS)
	{
		// 读取一个 ts 包
		nres = (vatek_result)fread(ptr, pfile->packet_len, 1, pfile->fhandle);
		if (nres != 1)
		{
			// 读取失败，就 seek 回文件头，然后锁定 ts 流，然后 continue
			// 为什么要这样？可能是为了排除因为即将到达文件尾所以读取包失败。
			fseek(pfile->fhandle, 0, SEEK_SET);
			nres = file_lock(pfile);
			if (is_vatek_success(nres))
				continue;
		}

		// 读取包成功
		if (ptr[0] == TS_PACKET_SYNC_TAG)
		{
			pos++;
			ptr += TS_PACKET_LEN;
		}
		else
		{
			nres = file_lock(pfile);
			if (!is_vatek_success(nres))
				break;
		}
	}

	if (is_vatek_success(nres))
		nres = (vatek_result)pos;

	return nres;
}

uint8_t *file_stream_get(hstream_source hsource)
{
	handle_file *pfile = (handle_file *)hsource;
	return &pfile->buffer[0];
}

vatek_result file_stream_stop(hstream_source hsource)
{
	return vatek_success;
}

void file_stream_free(hstream_source hsource)
{
	handle_file *pfile = (handle_file *)hsource;
	fclose(pfile->fhandle);
	delete pfile;
}

vatek_result file_lock(handle_file *pfile)
{
	vatek_result nres = vatek_badstatus;
	uint8_t sync;
	uint32_t count = 0;

	for (;;)
	{
		// 获取当前文件指针
		size_t pos = ftell(pfile->fhandle);

		/* 读取 1 个字节，返回值为成功读取的元素个数，而不是字节数。在这里，一个元素就是 1 个字节，所以元素数
		* 等于字节数。
		*/
		nres = (vatek_result)fread(&sync, 1, 1, pfile->fhandle);
		if (nres != 1)
		{
			// 不等于 1，说明没有成功读取到 1 个字节，返回错误代码
			return vatek_hwfail;
		}

		// 成功读取到 1 个字节，检查读取到的 1 个字节是否是 ts 的同步字节
		if (sync == TS_PACKET_SYNC_TAG)
		{
			// 如果是同步字节
			pfile->packet_len = 0;

			/* pos 的值是一进入循环就第一时间获取了，此时还没有读取任何一个字节。执行到这里说明读取了 1 个
			* 字节，并且这个字节是同步字节。
			*
			* 调用 file_check_sync 函数后，内部会以文件开头为参考点，seek 一段 pos + TS_PACKET_LEN 的距离，
			* 因为 pos 指向的是同步字节的位置，而 TS_PACKET_LEN 等于一个 ts 包的长度，
			* 所以 seek 后文件指针又是处于同步字节的位置。然后 file_check_sync 会读取 1 个字节并检查是否是
			* 同步字节。如果是同步字节，加上刚才读取的同步字节，总共同步到 2 个包。
			*/
			nres = file_check_sync(pfile->fhandle, (int32_t)pos, TS_PACKET_LEN);
			if (is_vatek_success(nres))
			{
				// 是同步字节
				pfile->packet_len = TS_PACKET_LEN;
			}
			else
			{
				// 不是同步字节，可能是因为此 ts 流的包大小不是 188 字节，而是 204 字节，再试一次。
				nres = file_check_sync(pfile->fhandle, (int32_t)pos, 204);

				if (is_vatek_success(nres))
				{
					// 再试一次后如果是同步字节
					pfile->packet_len = 204;
				}
			}

			if (pfile->packet_len != 0)
			{
				// pfile->packet_len != 0 说明前面的 file_check_sync 成功了
				// 成功后 seek 回原来的位置。将 seek 的结果作为返回值。成功为 0，失败为 -1.
				nres = (vatek_result)fseek(pfile->fhandle, (int32_t)pos, SEEK_SET);
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

vatek_result file_check_sync(FILE *hfile, int32_t pos, int32_t offset)
{
	// seek 成功返回 0，失败返回 -1
	vatek_result nres = (vatek_result)fseek(hfile, pos + offset, SEEK_SET);
	if (nres)
	{
		// seek 失败，返回 -1
		return (vatek_result)-1;
	}

	uint8_t tag = 0;

	// 读取 1 个字节
	nres = (vatek_result)fread(&tag, 1, 1, hfile);
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
