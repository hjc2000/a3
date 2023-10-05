
#include "../inc/tool_stream.h"
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"


/// <summary>
///		永远返回 vatek_success，其实就是 0
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern vatek_result file_stream_start(hstream_source hsource);
extern vatek_result file_stream_check(hstream_source hsource);
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
typedef struct _handle_file
{
	int32_t packet_len;
	int32_t file_size;

	/// <summary>
	///		C 的文件句柄
	/// </summary>
	FILE *fhandle;
	uint8_t buffer[CHIP_STREAM_SLICE_LEN];
}handle_file, *Phandle_file;

/// <summary>
///		锁定 ts 流。
///		总共会读取 2 个包，检查同步字节。如果成功同步到 2 个包，就认为锁定成功。
/// </summary>
/// <param name="pfile"></param>
/// <returns></returns>
extern vatek_result file_lock(Phandle_file pfile);

/// <summary>
///		将文件指针移动到以文件开始为参考点的 pos + offset 处，然后读取 1 个字节，检查是否是 ts
///		的同步字节。
/// </summary>
/// <param name="hfile"></param>
/// <param name="pos"></param>
/// <param name="offset"></param>
/// <returns></returns>
extern vatek_result file_check_sync(FILE *hfile, int32_t pos, int32_t offset);

vatek_result stream_source_file_get(const char *file, Ptsstream_source psource)
{
	/* 动态分配一块 handle_file 这么大的内存，用来存放 handle_file 对象。返回这块内存的指针。
	*/
	Phandle_file pfile = (Phandle_file)malloc(sizeof(handle_file));
	vatek_result nres = vatek_memfail;
	if (pfile)
	{
		memset(pfile, 0, sizeof(handle_file));

		// 打开文件，将文件句柄放到刚才分配的 handle_file 里面
		pfile->fhandle = fopen(file, "rb+");
		nres = vatek_format;
		if (pfile->fhandle)
		{
			fseek(pfile->fhandle, 0, SEEK_END);
			pfile->file_size = (int32_t)ftell(pfile->fhandle);
			fseek(pfile->fhandle, 0, SEEK_SET);

			nres = file_lock(pfile);
			if (!is_vatek_success(nres))
			{
				fclose(pfile->fhandle);
				free(pfile);
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
	}

	return nres;
}

vatek_result file_stream_start(hstream_source hsource)
{
	return vatek_success;
}

vatek_result file_stream_check(hstream_source hsource)
{
	Phandle_file pfile = (Phandle_file)hsource;
	int32_t pos = 0;
	uint8_t *ptr = &pfile->buffer[0];
	vatek_result nres = vatek_success;

	while (pos < CHIP_STREAM_SLICE_PACKET_NUMS)
	{
		nres = (vatek_result)fread(ptr, pfile->packet_len, 1, pfile->fhandle);
		if (nres == 0)
		{
			fseek(pfile->fhandle, 0, SEEK_SET);
			nres = file_lock(pfile);
			if (is_vatek_success(nres))
				continue;
		}
		else if (nres == 1)
		{
			if (ptr[0] == TS_PACKET_SYNC_TAG)
			{
				pos++;
				ptr += TS_PACKET_LEN;
			}
			else
			{
				nres = file_lock(pfile);
			}
		}

		if (!is_vatek_success(nres))
			break;
	}

	if (is_vatek_success(nres))
		nres = pos;
	return nres;
}

uint8_t *file_stream_get(hstream_source hsource)
{
	Phandle_file pfile = (Phandle_file)hsource;
	return &pfile->buffer[0];
}

vatek_result file_stream_stop(hstream_source hsource)
{
	return vatek_success;
}

void file_stream_free(hstream_source hsource)
{
	Phandle_file pfile = (Phandle_file)hsource;
	fclose(pfile->fhandle);
	free(pfile);
}

vatek_result file_lock(Phandle_file pfile)
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
			// 不等于 1，说明没有成功读取到 1 个字节
			nres = vatek_hwfail;
		}
		else if (nres == 0)
		{
			nres = vatek_badstatus;
		}
		else
		{
			// 检查读取到的 1 个字节是否是 ts 的同步字节
			if (sync == TS_PACKET_SYNC_TAG)
			{
				// 如果是同步字节
				pfile->packet_len = 0;

				/* pos 的值是一进入循环就第一时间获取了，此时还没有读取任何一个字节。然后到这里是读取了 1 个
				* 字节，并且这个字节是同步字节。调用 file_check_sync 函数后，内部会以文件开头为参考点，seek
				* 一段 pos + TS_PACKET_LEN 的距离，因为 pos 指向的是同步字节的位置，而 pos + TS_PACKET_LEN
				* 等于一个 ts 包的长度，所以 seek 后文件指针又是处于同步字节的位置。然后 file_check_sync 会读取
				* 1 个字节并检查是否是同步字节。
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
						pfile->packet_len = 204;
				}

				if (nres == vatek_format)
				{
					nres = vatek_success;
				}
				else if (pfile->packet_len != 0)
				{
					nres = (vatek_result)fseek(pfile->fhandle, (int32_t)pos, SEEK_SET);
					break;
				}
			}
		}

		// 锁定 ts 流成功，退出循环
		if (!is_vatek_success(nres))
			break;

		count++;

		// 计数溢出后还没锁定到 ts 流，就超时
		if (count > 1000)
			return vatek_timeout;
	}

	return nres;
}

vatek_result file_check_sync(FILE *hfile, int32_t pos, int32_t offset)
{
	vatek_result nres = (vatek_result)fseek(hfile, pos + offset, SEEK_SET);
	if (is_vatek_success(nres))
	{
		uint8_t tag = 0;

		// 读取 1 个字节
		nres = (vatek_result)fread(&tag, 1, 1, hfile);
		if (nres == 1)
		{
			// 检查读取到的 1 个字节是否是 ts 的同步字节
			if (tag == TS_PACKET_SYNC_TAG)
			{
				nres = (vatek_result)1;
			}
			else
			{
				nres = vatek_format;
			}
		}
		else
		{
			nres = vatek_hwfail;
		}
	}

	return nres;
}