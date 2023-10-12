#include <vatek_sdk_device.h>
#include <halservice_rescure.h>
#include <halreg_calibration.h>
#include <output_rfmixer.h>
#include <ui_service_transform.h>
#include "device/internal/cross_device_tool.h"
#include<Exception.h>

vatek_result vatek_device_list_enum(uint32_t bus, hal_service_mode service, vatek_device_list **hdevices)
{
	cross_device *newdevs = NULL;
	vatek_result nres = cross_devices_create(&newdevs);
	if (nres > vatek_success)
	{
		int32_t len = sizeof(vatek_device_list) + (sizeof(cross_device *) * (nres + 1));
		vatek_device_list *newlist = (vatek_device_list *)malloc(len);
		if (!newlist)
		{
			throw Exception();
		}

		int32_t pos = 0;
		nres = vatek_success;
		memset((uint8_t *)newlist, 0, len);
		newlist->listdevices = (cross_device **)&((uint8_t *)newlist)[sizeof(vatek_device_list)];
		newlist->cross = newdevs;
		while (newdevs)
		{
			if ((newdevs->bus & bus))
			{
				if (service == service_unknown || service == newdevs->service)
				{
					newlist->listdevices[pos] = newdevs;
					pos++;
				}
			}
			newdevs = newdevs->next;
		}

		if (!pos)
		{
			free(newlist);
		}
		else
		{
			newlist->nums = pos;
			*hdevices = newlist;
			nres = (vatek_result)pos;
		}
	}

	return nres;
}

vatek_result vatek_device_list_enum_by_usbid(uint16_t vid, uint16_t pid, vatek_device_list **hdevices)
{
	cross_device *newdevs = NULL;
	vatek_result nres = cross_devices_create_by_usbid(vid, pid, &newdevs);
	if (nres > vatek_success)
	{
		int32_t len = sizeof(vatek_device_list) + (sizeof(cross_device *) * (nres + 1));
		vatek_device_list *newlist = (vatek_device_list *)malloc(len);
		nres = vatek_memfail;
		if (newlist)
		{
			int32_t pos = 0;
			nres = vatek_success;
			memset((uint8_t *)newlist, 0, len);
			newlist->listdevices = (cross_device **)&((uint8_t *)newlist)[sizeof(vatek_device_list)];
			newlist->cross = newdevs;
			while (newdevs)
			{
				newlist->listdevices[pos] = newdevs;
				pos++;
				newdevs = newdevs->next;
			}

			if (!pos)free(newlist);
			else
			{
				newlist->nums = pos;
				*hdevices = newlist;
				nres = (vatek_result)pos;
			}
		}
	}
	return nres;
}

uint32_t vatek_device_list_get_bus(vatek_device_list *hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->bus;
	return DEVICE_BUS_UNKNOWN;
}

const char *vatek_device_list_get_name(vatek_device_list *hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->get_device_name();
	return "_unknown";
}

hal_service_mode vatek_device_list_get_service(vatek_device_list *hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->service;
	return service_unknown;
}

vatek_result vatek_device_open(vatek_device_list *hdevices, int32_t idx, vatek_device * *hchip)
{
	vatek_result nres = vatek_badparam;
	if (idx < hdevices->nums)
	{
		cross_device *pcross = hdevices->listdevices[idx];
		vatek_device *pvatek = new vatek_device{ pcross };
		nres = chip_info_get((vatek_device *)pvatek, &pvatek->info);
		if (is_vatek_success(nres))
			*hchip = pvatek;
		else
			delete pvatek;
	}

	return vatek_result::vatek_success;
}

void vatek_device_list_free(vatek_device_list *hdevices)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	cross_devices_free(pdevices->cross);
	free(pdevices);
}

uint32_t vatek_device_get_bus(vatek_device * hchip)
{
	cross_device *pcross = ((vatek_device *)hchip)->cross;
	return pcross->bus;
}

chip_info *vatek_device_get_info(vatek_device * hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	pvatek->info.status = chip_status_get(hchip, &pvatek->info.errcode);
	return &pvatek->info;
}

const char *vatek_device_get_name(vatek_device * hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	return pvatek->cross->get_device_name();
}

vatek_result vatek_device_start_sine(vatek_device * hchip, uint32_t freqkhz)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	chip_info *pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_waitcmd)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_start(hchip, HALREG_SERVICE_BASE_CNTL, freqkhz);
		}
		else
		{
			nres = vatek_success;
		}

		if (is_vatek_success(nres))
		{
			nres = chip_send_command(
				hchip,
				BASE_CMD_TEST_START_SINE,
				HALREG_SERVICE_BASE_CNTL,
				HALREG_SYS_ERRCODE
			);
		}
	}

	return nres;
}

vatek_result vatek_device_start_test(vatek_device * hchip, Pmodulator_param pmod, uint32_t freqkhz)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	chip_info *pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_waitcmd)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_start(hchip, HALREG_SERVICE_BASE_CNTL, freqkhz);
		}
		else
			nres = vatek_success;

		if (is_vatek_success(nres))
		{
			nres = modulator_param_set(hchip, pmod);
			if (is_vatek_success(nres))
			{
				nres = chip_send_command(
					hchip,
					BASE_CMD_TEST_START_SINE,
					HALREG_SERVICE_BASE_CNTL,
					HALREG_SYS_ERRCODE
				);
			}
		}
	}

	return nres;
}

vatek_result vatek_device_polling(vatek_device * hchip)
{
	vatek_result nres = vatek_badstatus;
	chip_info *pinfo = vatek_device_get_info(hchip);
	if (pinfo->status == chip_status_running)
		nres = vatek_success;

	return nres;
}

void vatek_device_stop(vatek_device * hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	chip_info *pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_running)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_stop(hchip, HALREG_SERVICE_BASE_CNTL);
			if (!is_vatek_success(nres))
				VWAR("rfmixer_stop fail : %d", nres);
		}

		nres = chip_send_command(
			hchip,
			BASE_CMD_STOP,
			HALREG_SERVICE_BASE_CNTL,
			HALREG_SYS_ERRCODE
		);

		if (!is_vatek_success(nres))
			VWAR("stop fail : %d", nres);
	}
}

vatek_result vatek_device_close_reboot(vatek_device * hchip)
{
	vatek_result nres = vatek_success;
	vatek_chip_write_memory(hchip, HALREG_SERVICE_BASE_CNTL, BASE_CMD_REBOOT);
	vatek_chip_write_memory(hchip, HALREG_RESCUE_CNTL, RESCUE_CNTL_REBOOT);
	nres = vatek_device_close(hchip);
	cross_os_sleep(500);
	return nres;
}

vatek_result vatek_device_close(vatek_device * hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	delete pvatek;
	return vatek_success;
}

vatek_result vatek_device_calibration_load(vatek_device * hchip, Pcalibration_param pcalibration)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
		nres = calibration_get(hchip, pcalibration);

	return nres;
}

vatek_result vatek_device_calibration_apply(vatek_device * hchip, Pcalibration_param pcalibration)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_badstatus;
	chip_info *pinfo = vatek_device_get_info(hchip);
	if (pinfo->status == chip_status_running)
	{
		nres = vatek_unsupport;
		if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
			nres = calibration_set(hchip, pcalibration, 1);
		nres = calibration_adjust_gain(hchip, pcalibration->dac_power, pcalibration);
		nres = rfmixer_r2_adjust_pagain(hchip, pcalibration->r2_power);

	}

	return nres;
}

vatek_result vatek_device_r2_apply(vatek_device * hchip, int r2_power)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_badstatus;
	chip_info *pinfo = vatek_device_get_info(hchip);
	if (pinfo->status == chip_status_running)
	{
		nres = vatek_unsupport;
		if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
			nres = rfmixer_r2_adjust_pagain(hchip, r2_power);
	}

	return nres;
}

vatek_result vatek_device_calibration_save(vatek_device * hchip, Pcalibration_param pcalibration)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
	{
		nres = calibration_set(hchip, pcalibration, 0);
		cross_os_sleep(200);
		if (is_vatek_success(nres))
			nres = chip_send_command(hchip, BASE_CMD_CALIBRATION_SAVE, HALREG_SERVICE_BASE_CNTL, HALREG_SYS_ERRCODE);
		cross_os_sleep(200);
	}

	return nres;
}

vatek_result vatek_device_stream_start(vatek_device * hchip, Pmodulator_param pmod, uint32_t stream_mode)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	cross_stream *pstream = pvatek->cross->stream;
	vatek_result nres = vatek_unsupport;

	if (pstream)
	{
		chip_info *pchipinfo = vatek_device_get_info(hchip);
		if (pchipinfo->hal_service == service_transform)
		{
			nres = vatek_badstatus;
			if (pchipinfo->status == chip_status_running)
				nres = vatek_success;
		}

		if (is_vatek_success(nres))
		{
			cross_stream_mode mode = stream_mode_output;
			/* _isdb-t must disable usb_dma because chip memory bandwidth limited.
				DVB-T2 only remux set output_nodma*/
			if (pmod->type == modulator_isdb_t ||
				pmod->type == modulator_dtmb ||
				(stream_mode == stream_remux && pmod->type == modulator_dvb_t2))
			{
				mode = stream_mode_output_nodma;
			}

			nres = pstream->start_stream(pvatek->cross->hcross, mode);
			if (is_vatek_success(nres))
			{
				pvatek->streammode = stream_mode_output;
			}
		}
	}

	return nres;
}

vatek_result vatek_device_stream_write(vatek_device * hchip, uint8_t *pbuf, int32_t size)
{
	vatek_result nres = vatek_badstatus;
	vatek_device *pvatek = (vatek_device *)hchip;
	usb_handle_list_node * hcross = (usb_handle_list_node *)pvatek->cross->hcross;
	cross_stream *pstream = pvatek->cross->stream;
	if (pvatek->streammode == stream_mode_output)
	{
		nres = pstream->write_stream(hcross, pbuf, size);
	}

	return nres;
}

vatek_result vatek_device_stream_stop(vatek_device * hchip)
{
	vatek_result nres = vatek_badstatus;
	vatek_device *pvatek = (vatek_device *)hchip;
	usb_handle_list_node * hcross = (usb_handle_list_node *)pvatek->cross->hcross;
	cross_stream *pstream = pvatek->cross->stream;

	if (pvatek->streammode != stream_mode_idle)
	{
		nres = pstream->stop_stream(hcross);
		pvatek->streammode = stream_mode_idle;
	}

	return nres;
}

vatek_result vatek_device_usbbulk_send(vatek_device * hchip, usbbulk_command * pcmd, usbbulk_result * presult, uint8_t *pbuf, int32_t len)
{
	#define USBBUF_DIR_NULL	0
	#define USBBUF_DIR_IN	1
	#define USBBUF_DIR_OUT	2
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		usb_handle_list_node * husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;

		nres = pbulk->send_command(husb, pcmd);
		if (is_vatek_success(nres))
			nres = pbulk->get_result(husb, presult);

		if (is_vatek_success(nres) && pbuf && len)
		{
			int32_t bufdir = USBBUF_DIR_NULL;
			if (pcmd->mode == usbbulk_flash)
			{
				if (pcmd->_h.flash.mode == usbflash_write)
					bufdir = USBBUF_DIR_OUT;
				else if (pcmd->_h.flash.mode == usbflash_read)
					bufdir = USBBUF_DIR_IN;
			}
			else if (pcmd->mode == usbbulk_aux)
			{
				if (pcmd->_h.aux.mode == usbaux_async)
					bufdir = USBBUF_DIR_OUT;
			}

			if (bufdir != USBBUF_DIR_NULL && pbuf && len)
			{
				if (bufdir == USBBUF_DIR_OUT)
					nres = pbulk->write(husb, pbuf, len);
				else if (bufdir == USBBUF_DIR_IN)
					nres = pbulk->read(husb, pbuf, len);

				if (is_vatek_success(nres))
					nres = pbulk->get_result(husb, presult);
			}
		}
	}

	return nres;
}

vatek_result vatek_device_usbbulk_get_result(vatek_device * hchip, usbbulk_result * presult)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		usb_handle_list_node * husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->get_result(husb, presult);
	}

	return nres;
}

vatek_result vatek_device_usbbulk_write(vatek_device * hchip, uint8_t *pbuf, int32_t len)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		usb_handle_list_node * husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->write(husb, pbuf, len);
	}

	return nres;
}

vatek_result vatek_device_usbbulk_read(vatek_device * hchip, uint8_t *pbuf, int32_t len)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		usb_handle_list_node * husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->read(husb, pbuf, len);
	}

	return nres;
}

vatek_result vatek_device_usbbulk_get_size(vatek_device * hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		usb_handle_list_node * husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->get_size(husb);
	}

	return nres;
}
