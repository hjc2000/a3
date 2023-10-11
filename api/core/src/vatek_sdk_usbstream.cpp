#include <vatek_sdk_usbstream.h>
#include <vatek_sdk_transform.h>

#include <core/base/output_rfmixer.h>
#include <core/tools/tool_helpful.h>
#include <cross/cross_os_api.h>

#define US_DEF_PREPARE_TIME				10
#define US_CIRCLEBUF_SIZE				(2*1024*1024)
#define US_CIRCLEBUF_NUMS				(US_CIRCLEBUF_SIZE/CHIP_STREAM_SLICE_LEN)
#define USBSTREAM_SLICE_PACKET_NUMS		(CHIP_STREAM_SLICE_LEN/CHIP_TS_PACKET_LEN)

struct handle_async
{
	uasync_status async_status;
	Pth_circlebuf async_buffer;
	void_cross_mutex async_lock;
	usbstream_slice slicebuf;
};

struct handle_usbstream
{
	usbstream_mode mode;
	usbstream_status status;
	void_vatek_chip hchip;
	hvatek_transform htransform;
	transform_broadcast broadcast;
	int32_t is_support_r2;
	void_cross_thread hthread;
	Ptransform_info info;
	uint32_t stream_tick;
	uint32_t stream_packets;
	usbstream_sync sync;
	handle_async *async;
};

extern void usbstream_sync_handler(cross_thread_param * param);
extern void usbstream_async_handler(cross_thread_param * param);

extern vatek_result usbstream_update_stream(handle_usbstream *pustream);
extern vatek_result usbstream_commit_stream(handle_usbstream *pustream);

extern vatek_result usbstream_async_create(handle_async **pasync, usbstream_param *puparam);
extern vatek_result usbstream_async_get_slice(handle_async *pasync);
extern void usbstream_async_commit_slice(handle_async *pasync);

extern uint8_t *usbstream_async_get_buffer(handle_async *pasync);
extern void usbstream_async_finish_buffer(handle_async *pasync);

extern void usbstream_async_free(handle_async *pasync);

vatek_result vatek_usbstream_open(void_vatek_chip hchip, hvatek_usbstream *husstream)
{
	chip_info *pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_unsupport;
	if (pinfo->hal_service == service_transform &&
		vatek_device_get_bus(hchip) == DEVICE_BUS_USB)
	{
		hvatek_transform htr = NULL;
		nres = vatek_transform_open(hchip, &htr);
		if (is_vatek_success(nres))
		{
			if (pinfo->status != chip_status_waitcmd)
				vatek_transform_stop(htr);
		}

		if (is_vatek_success(nres))
		{
			handle_usbstream *pustream = (handle_usbstream *)malloc(sizeof(handle_usbstream));
			nres = vatek_memfail;
			if (pustream)
			{
				memset(pustream, 0, sizeof(handle_usbstream));
				transform_broadcast_reset(pinfo->chip_module, stream_source_usb, &pustream->broadcast);
				pustream->hchip = hchip;
				pustream->htransform = htr;
				if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
					pustream->is_support_r2 = 1;
				pustream->status = usbstream_status_idle;
				*husstream = pustream;
				nres = vatek_success;
			}
			if (!is_vatek_success(nres))vatek_transform_close(htr);
		}
	}
	return nres;
}

vatek_result vatek_usbstream_start(hvatek_usbstream husstream, usbstream_param *puparam)
{
	vatek_result nres = vatek_badstatus;
	usbstream_param *pustream_new = puparam;

	handle_usbstream *pustream = (handle_usbstream *)husstream;
	if (pustream->status == usbstream_status_idle)
	{
		Pmodulator_param pmod = &pustream->broadcast.modulator;
		uint32_t freqmhz = 0;

		nres = vatek_success;
		memcpy(pmod, &puparam->modulator, sizeof(modulator_param));
		pustream->mode = puparam->mode;
		if (pustream->is_support_r2)
		{
			uint32_t khz = puparam->r2param.freqkhz % 1000;
			freqmhz = (puparam->r2param.freqkhz / 1000) * 1000;
			if (khz != 0 && pmod->ifmode == ifmode_disable)
			{
				pmod->ifmode = ifmode_iqoffset;
				pmod->iffreq_offset = khz;
			}
		}

		if (is_vatek_success(nres))
		{
			fpcross_thread_function fphandler = usbstream_sync_handler;
			if (pustream->mode == ustream_mode_sync)
				memcpy(&pustream->sync, &puparam->sync, sizeof(usbstream_sync));
			else if (pustream->mode == ustream_mode_async)
			{
				nres = usbstream_async_create(&pustream->async, puparam);
				if (is_vatek_success(nres))
					fphandler = usbstream_async_handler;
			}
			else nres = vatek_badparam;

			if (is_vatek_success(nres))
			{
				pustream->broadcast.stream.usb.usb_flags = USB_EN_ASYNCBUFFER;
				if (puparam->remux == ustream_remux_passthrough)
				{
					pustream->broadcast.stream.usb.mode = stream_passthrogh;
					pustream->broadcast.stream.usb.pcrmode = pcr_disable;
				}
				else
				{
					pustream->broadcast.stream.usb.mode = stream_remux;
					pustream->broadcast.stream.usb.pcrmode = pustream_new->pcradjust;
				}
			}

			if (is_vatek_success(nres))
			{
				nres = vatek_transform_start_broadcast(pustream->htransform, &pustream->broadcast, puparam->r2param);
				if (is_vatek_success(nres))
				{
					uint32_t stream_mode = pustream->broadcast.stream.usb.mode;
					nres = vatek_device_stream_start(pustream->hchip, pmod, stream_mode);
					if (!is_vatek_success(nres))vatek_transform_stop(pustream->htransform);
				}

				if (is_vatek_success(nres))
				{
					nres = vatek_transform_polling(pustream->htransform, &pustream->info);
					if (is_vatek_success(nres))
					{
						nres = vatek_hwfail;
						pustream->status = usbstream_status_running;
						pustream->hthread = cross_os_create_thread(fphandler, pustream);
						if (pustream->hthread)nres = vatek_success;
					}
				}
			}

			if (!is_vatek_success(nres))vatek_usbstream_stop(husstream);
		}
	}
	return nres;
}

vatek_result vatek_usbstream_check(hvatek_usbstream husstream)
{
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	vatek_result nres = vatek_badstatus;
	if (pustream->status != usbstream_status_running)
	{
		uint32_t nvalue = 0;
		nres = vatek_chip_read_register(pustream->hchip, HALREG_SYS_ERRCODE, &nvalue);
	}
	else nres = vatek_success;
	return nres;
}

Pbroadcast_info vatek_usbstream_get_info(hvatek_usbstream husstream)
{
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	return &pustream->info->info;
}

usbstream_status vatek_usbstream_get_status(hvatek_usbstream husstream, Ptransform_info *ptrinfo)
{
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	if (ptrinfo)*ptrinfo = pustream->info;
	return pustream->status;
}

vatek_result vatek_ustream_async_get_buffer(hvatek_usbstream husstream, Pusbstream_slice *pslicebuf)
{
	vatek_result nres = vatek_unsupport;
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	if (pustream->async)
	{
		nres = usbstream_async_get_slice(pustream->async);
		if (nres > vatek_success)
			*pslicebuf = &pustream->async->slicebuf;
	}

	return nres;
}

uasync_status vatek_ustream_async_get_status(hvatek_usbstream hustream)
{
	handle_usbstream *pustream = (handle_usbstream *)hustream;
	if (pustream->async)
		return pustream->async->async_status;
	return uasync_status_unsupport;
}

vatek_result vatek_ustream_async_commit_buffer(hvatek_usbstream husstream, Pusbstream_slice pslicebuf)
{
	vatek_result nres = vatek_unsupport;
	handle_usbstream *pustream = (handle_usbstream *)husstream;

	if (pustream->async)
	{
		nres = vatek_badparam;
		if (pslicebuf == &pustream->async->slicebuf)
		{
			usbstream_async_commit_slice(pustream->async);
			nres = vatek_success;
		}
	}
	return nres;
}

vatek_result vatek_usbstream_stop(hvatek_usbstream husstream)
{
	vatek_result nres = vatek_badstatus;
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	uint32_t err = 0;
	if (pustream->status != usbstream_status_idle)
	{
		if (pustream->hthread)
		{
			if (pustream->status == usbstream_status_running)
			{
				pustream->status = usbstream_status_stopping;
				while (pustream->status == usbstream_status_stopping)
					cross_os_sleep(1);
			}
			nres = cross_os_free_thread(pustream->hthread);
			if (!is_vatek_success(nres))VWAR("usbstream thread end error : %d", nres);
			pustream->hthread = NULL;
		}

		nres = vatek_device_stream_stop(pustream->hchip);
		if (!is_vatek_success(nres))VWAR("stop device stream fail : %d", nres);

		nres = vatek_transform_stop(pustream->htransform);
		if (pustream->async)
		{
			usbstream_async_free(pustream->async);
			pustream->async = NULL;
		}
	}
	else nres = vatek_success;
	pustream->status = usbstream_status_idle;

	return nres;
}

void vatek_usbstream_close(hvatek_usbstream husstream)
{
	handle_usbstream *pustream = (handle_usbstream *)husstream;
	vatek_usbstream_stop(husstream);
	vatek_transform_close(pustream->htransform);
	free(pustream);
}

void usbstream_sync_handler(cross_thread_param * param)
{
	handle_usbstream *pustream = (handle_usbstream *)param->userparam;
	vatek_result nres = vatek_success;
	fpsync_get_buffer fpgetbuf = pustream->sync.getbuffer;
	void *fpparam = pustream->sync.void_param;
	pustream->stream_packets = 0;
	pustream->stream_tick = cross_os_get_tick_ms();

	while (pustream->status == usbstream_status_running)
	{
		nres = usbstream_update_stream(pustream);
		if (nres == vatek_success)continue;
		if (is_vatek_success(nres))
		{
			while (pustream->stream_packets >= USBSTREAM_SLICE_PACKET_NUMS)
			{
				VERR("pustream->stream_packets : %d\n", pustream->stream_packets);
				uint8_t *pbuf = NULL;
				nres = fpgetbuf(fpparam, &pbuf);
				if (nres > vatek_success)
					nres = vatek_device_stream_write(pustream->hchip, pbuf, CHIP_STREAM_SLICE_LEN);
				else if (nres == vatek_success)break;

				if (is_vatek_success(nres))pustream->stream_packets -= USBSTREAM_SLICE_PACKET_NUMS;
				else VWAR("write device stream fail : %d", nres);
				if (!is_vatek_success(nres))break;
			}

			if (is_vatek_success(nres))nres = usbstream_commit_stream(pustream);
		}
		if (!is_vatek_success(nres))break;
	}

	param->result = nres;
	if (!is_vatek_success(nres))
		pustream->status = usbstream_err_unknown;
	else pustream->status = usbstream_status_stop;
}

void usbstream_async_handler(cross_thread_param * param)
{
	handle_usbstream *pustream = (handle_usbstream *)param->userparam;
	vatek_result nres = vatek_success;
	handle_async *pasync = pustream->async;
	pustream->stream_packets = 0;
	pustream->stream_tick = cross_os_get_tick_ms();

	while (pustream->status == usbstream_status_running)
	{
		nres = usbstream_update_stream(pustream);
		if (nres == vatek_success)continue;

		if (is_vatek_success(nres))
		{
			int32_t numslice = 0;
			while (pustream->stream_packets >= USBSTREAM_SLICE_PACKET_NUMS)
			{
				VERR("slice pustream->stream_packets : %d\n", pustream->stream_packets);

				uint8_t *pbuf = usbstream_async_get_buffer(pasync);
				if (pbuf)
				{
					nres = vatek_device_stream_write(pustream->hchip, pbuf, CHIP_STREAM_SLICE_LEN);
					if (is_vatek_success(nres))
					{
						usbstream_async_finish_buffer(pasync);
						pustream->stream_packets -= USBSTREAM_SLICE_PACKET_NUMS;
						numslice++;
					}
					else VWAR("write device stream fail : %d", nres);
				}
				else break;

				if (!is_vatek_success(nres))break;
			}

			if (is_vatek_success(nres))
				nres = usbstream_commit_stream(pustream);

			#if 0
			if (pasync->async_status == uasync_status_idle)
			{
				if (pasync->async_packets > pustream->stream_packets)
					pasync->async_recvpackets = 0;
				else
				{
					pasync->async_recvpackets = pustream->stream_packets - pasync->async_packets;
					pustream->stream_packets = pasync->async_packets;
				}
				pasync->async_status = uasync_status_streaming;
			}
			else
			{
				//if (pustream->stream_packets > pasync->async_recvpackets)
				//	pustream->stream_packets -= pasync->async_recvpackets;

				while (pustream->stream_packets >= USBSTREAM_SLICE_PACKET_NUMS)
				{
					uint8_t *pbuf = usbstream_async_get_buffer(pasync);
					int32_t numslice = 0;
					if (pbuf)
					{
						nres = vatek_device_stream_write(pustream->hchip, pbuf, CHIP_STREAM_SLICE_LEN);
						if (is_vatek_success(nres))
						{
							usbstream_async_finish_buffer(pasync);
							pustream->stream_packets -= USBSTREAM_SLICE_PACKET_NUMS;
							numslice++;
						}
						else VWAR("write device stream fail : %d", nres);
					}
					else
					{
						if (!numslice)pustream->stream_packets = 0;
						break;
					}
					if (!is_vatek_success(nres))break;
				}
				if (is_vatek_success(nres))nres = usbstream_commit_stream(pustream);
			}
			#endif
		}
		if (!is_vatek_success(nres))break;
	}
	param->result = nres;
	if (!is_vatek_success(nres))
		pustream->status = usbstream_err_unknown;
	else pustream->status = usbstream_status_stop;
}

vatek_result usbstream_update_stream(handle_usbstream *pustream)
{
	vatek_result nres = (vatek_result)1;
	if (pustream->stream_packets == 0)
	{
		nres = vatek_transform_get_packets(pustream->htransform, &pustream->stream_packets);
		if (is_vatek_success(nres))
		{
			if (pustream->stream_packets == 0)cross_os_sleep(1);
			else nres = (vatek_result)1;
		}
	}
	return nres;
}

vatek_result usbstream_commit_stream(handle_usbstream *pustream)
{
	vatek_result nres = vatek_success;
	if (pustream->stream_packets < USBSTREAM_SLICE_PACKET_NUMS)
	{
		nres = vatek_transform_commit_packets(pustream->htransform);
		if (is_vatek_success(nres))
		{
			if (cross_os_get_tick_ms() - pustream->stream_tick > 1000)
			{
				nres = vatek_transform_polling(pustream->htransform, &pustream->info);
				pustream->stream_tick = cross_os_get_tick_ms();
			}
			cross_os_sleep(1);
		}
		pustream->stream_packets = 0;
	}
	return nres;
}

vatek_result usbstream_async_create(handle_async **pasync, usbstream_param *puparam)
{
	vatek_result nres = vatek_badparam;
	if (puparam->mode == ustream_mode_async)
	{
		void_cross_mutex hlock = NULL;
		nres = cross_os_create_mutex(&hlock);

		if (is_vatek_success(nres))
		{
			handle_async *newasync = (handle_async *)malloc(sizeof(handle_async));
			nres = vatek_memfail;
			if (newasync)
			{
				uint32_t bitrate = modulator_param_get_bitrate(&puparam->modulator);
				int32_t preparems = US_DEF_PREPARE_TIME;
				int32_t slicenums = 0;
				memset(newasync, 0, sizeof(handle_async));

				newasync->async_lock = hlock;

				if (puparam->async.bitrate > 0 && puparam->async.bitrate < bitrate)
					bitrate = puparam->async.bitrate;
				if (puparam->async.prepare_ms > US_DEF_PREPARE_TIME)
					preparems = puparam->async.prepare_ms;

				if (bitrate && preparems)
				{
					bitrate = (bitrate / 8) / CHIP_TS_PACKET_LEN;
					bitrate = (bitrate / 1000) * preparems;
					slicenums = ((bitrate / USBSTREAM_SLICE_PACKET_NUMS) + 1);
					#if 0
					newasync->async_packets = slicenums * USBSTREAM_SLICE_PACKET_NUMS;
					if (newasync->async_packets)nres = vatek_success;
					#endif
					nres = vatek_success;
				}

				if (is_vatek_success(nres))
				{
					newasync->slicebuf.packet_len = USBSTREAM_SLICE_PACKET_NUMS;
					newasync->async_buffer = th_circlebuf_create(CHIP_STREAM_SLICE_LEN, slicenums + (slicenums >> 2));
					if (!newasync->async_buffer)nres = vatek_memfail;
					else
					{
						newasync->async_status = uasync_status_idle;
						nres = vatek_success;
					}
				}

				if (!is_vatek_success(nres))free(newasync);
				else *pasync = newasync;
			}
			if (!is_vatek_success(nres))cross_os_free_mutex(hlock);
		}
	}

	return nres;
}

vatek_result usbstream_async_get_slice(handle_async *pasync)
{
	vatek_result nres = vatek_badstatus;
	if (!pasync->slicebuf.buf)
	{
		uint8_t *ptrbuf = NULL;
		cross_os_lock_mutex(pasync->async_lock);
		ptrbuf = th_circlebuf_get_wptr(pasync->async_buffer);
		cross_os_release_mutex(pasync->async_lock);

		if (ptrbuf)
		{
			pasync->slicebuf.buf = ptrbuf;
			pasync->slicebuf.ptrbuf = ptrbuf;
			nres = (vatek_result)1;
		}
		else nres = vatek_success;
	}
	return nres;
}

void usbstream_async_commit_slice(handle_async *pasync)
{
	if (pasync->slicebuf.buf)
	{
		cross_os_lock_mutex(pasync->async_lock);
		th_circlebuf_commit(pasync->async_buffer);
		cross_os_release_mutex(pasync->async_lock);
		pasync->slicebuf.packet_pos = 0;
		pasync->slicebuf.buf = NULL;
		pasync->slicebuf.ptrbuf = NULL;
	}
}

uint8_t *usbstream_async_get_buffer(handle_async *pasync)
{
	uint8_t *ptr = NULL;
	cross_os_lock_mutex(pasync->async_lock);
	ptr = th_circlebuf_get_rptr(pasync->async_buffer);
	cross_os_release_mutex(pasync->async_lock);
	return ptr;
}

void usbstream_async_finish_buffer(handle_async *pasync)
{
	cross_os_lock_mutex(pasync->async_lock);
	th_circlebuf_finish(pasync->async_buffer);
	cross_os_release_mutex(pasync->async_lock);
}

void usbstream_async_free(handle_async *pasync)
{
	if (pasync->async_lock)
		cross_os_free_mutex(pasync->async_lock);
	th_circlebuf_free(pasync->async_buffer);
	free(pasync);
}
