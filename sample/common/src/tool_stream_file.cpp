#include "../inc/tool_stream.h"
#include "../inc/tool_printf.h"
#include "../inc/tool_tspacket.h"

FileTsStreamSource::FileTsStreamSource()
{
	cout << "FileTsStreamSource 构造函数" << endl;
}

void FileTsStreamSource::Free()
{
	FileTsStreamSource *pfile = (FileTsStreamSource *)hsource;
	fclose(pfile->fhandle);
}

vatek_result stream_source_file_get(const char *file, FileTsStreamSource *psource)
{
	/* 打开文件，将文件句柄放到刚才分配的 FileTsStreamSource 里面。
	* 打开方式：读写，二进制
	*/
	psource->fhandle = fopen(file, "rb+");
	vatek_result nres = vatek_format;
	if (psource->fhandle)
	{
		// 通过 seek 到文件末尾来获取文件大小
		fseek(psource->fhandle, 0, SEEK_END);
		psource->file_size = (int32_t)ftell(psource->fhandle);
		// 刚才 seek 到文件末尾了，现在要 seek 回文件开始
		fseek(psource->fhandle, 0, SEEK_SET);

		// 锁定 ts 流
		nres = psource->lock_ts_file_stream();
		if (!is_vatek_success(nres))
		{
			fclose(psource->fhandle);
		}
		else
		{
			_disp_l("open file - [%s] - packet length:%d - packet size:%d", file, psource->packet_len, psource->file_size);
			printf("\r\n");
		}
	}

	if (!is_vatek_success(nres))
		_disp_err("file not current ts format - [%s]", file);

	return nres;
}

vatek_result FileTsStreamSource::Check()
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

uint8_t *FileTsStreamSource::Get()
{
	FileTsStreamSource *pfile = (FileTsStreamSource *)hsource;
	return &pfile->buffer[0];
}
