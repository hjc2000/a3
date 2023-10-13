#include "../inc/tool_stream.h"
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"

/// <summary>
///		直接返回 vatek_success，其实就是 0
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern vatek_result file_stream_start(void_stream_source hsource);
extern vatek_result file_stream_check(void_stream_source hsource);

/// <summary>
///		将 hsource 强制转换为 FileTsStreamSource * 后返回 buffer 字段。这是一个一维数组的头指针。
/// </summary>
/// <param name="hsource"></param>
/// <returns></returns>
extern uint8_t *file_stream_get(void_stream_source hsource);

/// <summary>
///		
/// </summary>
/// <param name="hsource"></param>
/// <returns>直接返回 vatek_success</returns>
extern vatek_result file_stream_stop(void_stream_source hsource);

/// <summary>
///		关闭 hsource 内的 fhandle 字段指向的文件，然后释放 hsource 对象
/// </summary>
/// <param name="hsource"></param>
extern void file_stream_free(void_stream_source hsource);

vatek_result stream_source_file_get(const char *file, TsStreamSource *psource)
{
	FileTsStreamSource *pfile = new FileTsStreamSource;
	if (!pfile)
	{
		// 内存分配失败
		return vatek_memfail;
	}

	/* 打开文件，将文件句柄放到刚才分配的 FileTsStreamSource 里面。
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
		nres = pfile->lock_ts_file_stream();
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

vatek_result file_stream_start(void_stream_source hsource)
{
	return vatek_success;
}

vatek_result file_stream_check(void_stream_source hsource)
{
	FileTsStreamSource *pfile = (FileTsStreamSource *)hsource;
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
			nres = pfile->lock_ts_file_stream();
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
			nres = pfile->lock_ts_file_stream();
			if (!is_vatek_success(nres))
				break;
		}
	}

	if (is_vatek_success(nres))
		nres = (vatek_result)pos;

	return nres;
}

uint8_t *file_stream_get(void_stream_source hsource)
{
	FileTsStreamSource *pfile = (FileTsStreamSource *)hsource;
	return &pfile->buffer[0];
}

vatek_result file_stream_stop(void_stream_source hsource)
{
	return vatek_success;
}

void file_stream_free(void_stream_source hsource)
{
	FileTsStreamSource *pfile = (FileTsStreamSource *)hsource;
	fclose(pfile->fhandle);
	delete pfile;
}
