#include "../inc/tool_stream.h"
#include "../inc/tool_printf.h"
#include <mux_define.h>


#define UDP_FRAME_LEN			1316
#define UDP_RTP_HDR_LEN			12
#define UDP_MAX_FRAME_LEN		(UDP_FRAME_LEN + UDP_RTP_HDR_LEN)

#define UDP_SLICE_BUF_MASK		0x1F

#define UDP_BUFFER_LEN			(((4*1024*1024) / UDP_FRAME_LEN) * UDP_FRAME_LEN)

extern vatek_result tool_udp_stream_start(void_stream_source hsource);
extern vatek_result tool_udp_stream_check(void_stream_source hsource);
extern uint8_t *tool_udp_stream_get(void_stream_source hsource);
extern vatek_result tool_udp_stream_stop(void_stream_source hsource);
extern void tool_udp_stream_free(void_stream_source hsource);

/// <summary>
///		接收 UDP 流的线程函数
/// </summary>
/// <param name="param"></param>
extern void tool_recv_handle(cross_thread_param *param);

extern uint8_t *tool_get_write_buffer(UdpTsStreamSource *pudp);
extern void commit_write_buffer(UdpTsStreamSource *pudp);
extern uint8_t *tool_get_valid_buffer(UdpTsStreamSource *pudp);
extern int32_t tool_check_valid_buffer(UdpTsStreamSource *pudp);

vatek_result stream_source_udp_get(const char *ipaddr, TsStreamSource *psource)
{
	socket_param sparam = socket_param{};
	vatek_result nres = vatek_success;
	void_cross_socket hsocket = NULL;
	HANDLE hlock = NULL;

	sparam.buffer_len = UDP_BUFFER_LEN;
	sparam.url = ipaddr;
	sparam.mode = socket_client;

	nres = cross_os_create_socket(&sparam, &hsocket);
	if (is_vatek_success(nres))
		nres = cross_os_create_mutex(&hlock);

	if (is_vatek_success(nres))
	{
		UdpTsStreamSource *pudp = new UdpTsStreamSource;
		nres = vatek_memfail;
		if (pudp)
		{
			memset(pudp, 0, sizeof(UdpTsStreamSource));
			pudp->hsocket = hsocket;
			pudp->hlock = hlock;

			psource->hsource = pudp;
			psource->start = tool_udp_stream_start;
			psource->stop = tool_udp_stream_stop;
			psource->check = tool_udp_stream_check;
			psource->get = tool_udp_stream_get;
			psource->free = tool_udp_stream_free;
			_disp_l("open UDP/RTP address - [%s]", sparam.url);
			printf("\r\n");
			nres = vatek_success;
		}
	}

	if (!is_vatek_success(nres))
	{
		if (hsocket)
			cross_os_free_socket(hsocket);
		if (hlock)
			cross_os_free_mutex(hlock);
	}

	return  nres;
}

vatek_result tool_udp_stream_start(void_stream_source hsource)
{
	vatek_result nres = vatek_badstatus;
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)hsource;
	if (!pudp->hrecv)
	{
		nres = cross_os_connect_socket(pudp->hsocket);
		if (is_vatek_success(nres))
		{
			pudp->isrunning = 1;
			pudp->hrecv = cross_os_create_thread(tool_recv_handle, pudp);
			if (!pudp->hrecv)nres = vatek_hwfail;

			if (!is_vatek_success(nres))
				nres = cross_os_disconnect_socket(pudp->hsocket);
		}
		else
		{
			_disp_err("cross_os_connect_socket fail : %d", nres);
		}
	}

	return nres;
}

vatek_result tool_udp_stream_check(void_stream_source hsource)
{
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)hsource;
	if (!pudp->isrunning)
	{
		_disp_err("recv thread not running");
		return vatek_badstatus;
	}
	else
	{
		int32_t valid = tool_check_valid_buffer((UdpTsStreamSource *)hsource);
		if (valid)return (vatek_result)1;
		return (vatek_result)0;
	}
}

uint8_t *tool_udp_stream_get(void_stream_source hsource)
{
	if (tool_udp_stream_check(hsource))
		return tool_get_valid_buffer((UdpTsStreamSource *)hsource);
	return NULL;
}

vatek_result tool_udp_stream_stop(void_stream_source hsource)
{
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)hsource;
	if (pudp->isrunning)
	{
		pudp->isrunning = 2;
		while (pudp->isrunning)
			cross_os_sleep(100);
		cross_os_free_thread(pudp->hrecv);
		cross_os_disconnect_socket(pudp->hsocket);
	}

	pudp->hrecv = NULL;
	return vatek_success;
}

void tool_udp_stream_free(void_stream_source hsource)
{
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)hsource;
	tool_udp_stream_stop(hsource);
	delete hsource;
}

void tool_recv_handle(cross_thread_param *param)
{
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)param->void_userparam;
	vatek_result nres = vatek_success;
	socket_protocol nprotocol = cross_os_get_protocol_socket(pudp->hsocket);
	int32_t framelen = UDP_FRAME_LEN;
	int32_t frameoffset = 0;
	uint8_t *pbuf = (uint8_t *)malloc(UDP_MAX_FRAME_LEN);
	if (!pbuf)
	{
		throw - 1;
	}

	int32_t pktnums = 0;
	int32_t retry = 0;
	uint8_t *pktbuf = NULL;

	memset(pbuf, 0, UDP_MAX_FRAME_LEN);

	if (nprotocol == protocol_rtp)
	{
		framelen = UDP_MAX_FRAME_LEN;
		frameoffset = UDP_RTP_HDR_LEN;
	}


	while (pudp->isrunning == 1)
	{
		nres = cross_os_recv_socket(pudp->hsocket, pbuf, framelen, 1000);
		if (is_vatek_success(nres))
		{
			uint8_t *precv = &pbuf[frameoffset];
			int32_t pos = frameoffset;

			while (pos < framelen && pudp->isrunning)
			{
				if (pktbuf == NULL)
				{
					pktbuf = tool_get_write_buffer(pudp);
					pktnums = 0;
				}

				if (!pktbuf)cross_os_sleep(1);
				else
				{
					if (precv[0] == TS_PACKET_SYNC_TAG)
						memcpy(&pktbuf[pktnums * TS_PACKET_LEN], precv, TS_PACKET_LEN);
					else nres = vatek_format;

					if (is_vatek_success(nres))
					{
						precv += TS_PACKET_LEN;
						pos += TS_PACKET_LEN;
						pktnums++;

						if (pktnums == CHIP_STREAM_SLICE_PACKET_NUMS)
						{
							commit_write_buffer(pudp);
							pktbuf = NULL;
						}
					}
					else break;
				}
			}
		}
		if (!is_vatek_success(nres))
		{
			if (nres == vatek_timeout)retry++;
			else break;
			if (retry > 10)break;;
		}
		else retry = 0;
	}
	pudp->isrunning = 0;
	free(pbuf);
}

uint8_t *tool_get_write_buffer(UdpTsStreamSource *pudp)
{
	uint8_t *ptr = NULL;
	int32_t nptr;
	cross_os_lock_mutex(pudp->hlock);
	nptr = (pudp->buf_wptr + 1) & UDP_SLICE_BUF_MASK;
	if (nptr != pudp->buf_rptr)
		ptr = &pudp->buf_pool[pudp->buf_wptr][0];
	cross_os_release_mutex(pudp->hlock);
	return ptr;
}

void commit_write_buffer(UdpTsStreamSource *pudp)
{
	cross_os_lock_mutex(pudp->hlock);
	pudp->buf_wptr++;
	pudp->buf_wptr &= UDP_SLICE_BUF_MASK;
	cross_os_release_mutex(pudp->hlock);
}

uint8_t *tool_get_valid_buffer(UdpTsStreamSource *pudp)
{
	uint8_t *ptr = NULL;
	cross_os_lock_mutex(pudp->hlock);
	ptr = &pudp->buf_pool[pudp->buf_rptr][0];
	pudp->buf_rptr++;
	pudp->buf_rptr &= UDP_SLICE_BUF_MASK;
	cross_os_release_mutex(pudp->hlock);
	return ptr;
}

int32_t tool_check_valid_buffer(UdpTsStreamSource *pudp)
{
	int32_t len;
	cross_os_lock_mutex(pudp->hlock);
	len = pudp->buf_wptr - pudp->buf_rptr;
	if (len < 0)
		len += UDP_SLICE_BUF_NUMS;
	cross_os_release_mutex(pudp->hlock);
	return len;
}

vatek_result UdpTsStreamSource::Start()
{
	vatek_result nres = vatek_badstatus;
	UdpTsStreamSource *pudp = (UdpTsStreamSource *)hsource;
	if (!pudp->hrecv)
	{
		nres = cross_os_connect_socket(pudp->hsocket);
		if (is_vatek_success(nres))
		{
			pudp->isrunning = 1;
			pudp->hrecv = cross_os_create_thread(tool_recv_handle, pudp);
			if (!pudp->hrecv)nres = vatek_hwfail;

			if (!is_vatek_success(nres))
				nres = cross_os_disconnect_socket(pudp->hsocket);
		}
		else
		{
			_disp_err("cross_os_connect_socket fail : %d", nres);
		}
	}

	return nres;
}