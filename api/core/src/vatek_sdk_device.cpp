#include <vatek_sdk_device.h>
#include <core/hal/halservice_rescure.h>
#include <core/hal/halreg_calibration.h>
#include <core/base/output_rfmixer.h>
#include <service/ui/ui_service_transform.h>
#include "device/internal/cross_device_tool.h"

extern vatek_result vatek_device_malloc(cross_device *pcross, vatek_device **pvatek);
extern void vatek_device_free(vatek_device *pvatek);

struct vatek_device_list
{
	int32_t nums;
	cross_device *cross;
	cross_device **listdevices;
};

vatek_result vatek_device_list_enum(uint32_t bus, hal_service_mode service, hvatek_devices *hdevices)
{
	cross_device *newdevs = NULL;
	vatek_result nres = cross_devices_create(&newdevs);
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
				if ((newdevs->bus & bus))
				{
					if (service == service_unknown ||
						service == newdevs->service)
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
	}

	return nres;
}

vatek_result vatek_device_list_enum_by_usbid(uint16_t vid, uint16_t pid, hvatek_devices *hdevices)
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

uint32_t vatek_device_list_get_bus(hvatek_devices hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->bus;
	return DEVICE_BUS_UNKNOWN;
}

const char *vatek_device_list_get_name(hvatek_devices hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->get_device_name();
	return "_unknown";
}

hal_service_mode vatek_device_list_get_service(hvatek_devices hdevices, int32_t idx)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	if (idx < pdevices->nums)
		return pdevices->listdevices[idx]->service;
	return service_unknown;
}

vatek_result vatek_device_open(hvatek_devices hdevices, int32_t idx, hvatek_chip *hchip)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	vatek_result nres = vatek_badparam;
	if (idx < pdevices->nums)
	{
		vatek_device *pvatek = NULL;
		cross_device *pcross = pdevices->listdevices[idx];
		nres = vatek_device_malloc(pcross, &pvatek);
		if (is_vatek_success(nres))
		{
			nres = chip_info_get((hvatek_chip)pvatek, &pvatek->info);
			if (is_vatek_success(nres))
				*hchip = pvatek;
			else
				vatek_device_free(pvatek);
		}
	}

	return nres;
}

void vatek_device_list_free(hvatek_devices hdevices)
{
	vatek_device_list *pdevices = (vatek_device_list *)hdevices;
	cross_devices_free(pdevices->cross);
	free(pdevices);
}

uint32_t vatek_device_get_bus(hvatek_chip hchip)
{
	cross_device *pcross = ((vatek_device *)hchip)->cross;
	return pcross->bus;
}

Pchip_info vatek_device_get_info(hvatek_chip hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	pvatek->info.status = chip_status_get(hchip, &pvatek->info.errcode);
	return &pvatek->info;
}

const char *vatek_device_get_name(hvatek_chip hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	return pvatek->cross->get_device_name();
}

vatek_result vatek_device_start_sine(hvatek_chip hchip, uint32_t freqkhz)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	Pchip_info pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_waitcmd)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_start(hchip, HALREG_SERVICE_BASE_CNTL, freqkhz);
		}
		else nres = vatek_success;

		if (is_vatek_success(nres))
			nres = chip_send_command(hchip, BASE_CMD_TEST_START_SINE, HALREG_SERVICE_BASE_CNTL, HALREG_SYS_ERRCODE);
	}
	return nres;
}

vatek_result vatek_device_start_test(hvatek_chip hchip, Pmodulator_param pmod, uint32_t freqkhz)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	Pchip_info pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_waitcmd)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_start(hchip, HALREG_SERVICE_BASE_CNTL, freqkhz);
		}

		else nres = vatek_success;

		if (is_vatek_success(nres))
		{
			nres = modulator_param_set(hchip, pmod);
			if (is_vatek_success(nres))
				nres = chip_send_command(hchip, BASE_CMD_TEST_START_SINE, HALREG_SERVICE_BASE_CNTL, HALREG_SYS_ERRCODE);
		}
	}
	return nres;
}

vatek_result vatek_device_polling(hvatek_chip hchip)
{
	vatek_result nres = vatek_badstatus;
	Pchip_info pinfo = vatek_device_get_info(hchip);
	if (pinfo->status == chip_status_running)
		nres = vatek_success;
	return nres;
}

void vatek_device_stop(hvatek_chip hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	Pchip_info pinfo = vatek_device_get_info(hchip);
	vatek_result nres = vatek_badstatus;
	if (pinfo->status == chip_status_running)
	{
		if (pinfo->peripheral_en & PERIPHERAL_FINTEKR2)
		{
			nres = rfmixer_stop(hchip, HALREG_SERVICE_BASE_CNTL);
			if (!is_vatek_success(nres))VWAR("rfmixer_stop fail : %d", nres);
		}
		nres = chip_send_command(hchip, BASE_CMD_STOP, HALREG_SERVICE_BASE_CNTL, HALREG_SYS_ERRCODE);
		if (!is_vatek_success(nres))VWAR("stop fail : %d", nres);
	}
}

vatek_result vatek_device_close_reboot(hvatek_chip hchip)
{
	vatek_result nres = vatek_success;
	vatek_chip_write_memory(hchip, HALREG_SERVICE_BASE_CNTL, BASE_CMD_REBOOT);
	vatek_chip_write_memory(hchip, HALREG_RESCUE_CNTL, RESCUE_CNTL_REBOOT);
	nres = vatek_device_close(hchip);
	cross_os_sleep(500);
	return nres;
}

vatek_result vatek_device_close(hvatek_chip hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_device_free(pvatek);
	return vatek_success;
}

vatek_result vatek_device_calibration_load(hvatek_chip hchip, Pcalibration_param pcalibration)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
		nres = calibration_get(hchip, pcalibration);
	return nres;
}

vatek_result vatek_device_calibration_apply(hvatek_chip hchip, Pcalibration_param pcalibration)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_badstatus;
	Pchip_info pinfo = vatek_device_get_info(hchip);
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

vatek_result vatek_device_r2_apply(hvatek_chip hchip, int r2_power)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_badstatus;
	Pchip_info pinfo = vatek_device_get_info(hchip);
	if (pinfo->status == chip_status_running)
	{
		nres = vatek_unsupport;
		if (pvatek->info.peripheral_en & PERIPHERAL_CALIBRATION)
			nres = rfmixer_r2_adjust_pagain(hchip, r2_power);
	}
	return nres;
}

vatek_result vatek_device_calibration_save(hvatek_chip hchip, Pcalibration_param pcalibration)
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

vatek_result vatek_device_stream_start(hvatek_chip hchip, Pmodulator_param pmod, uint32_t stream_mode)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	cross_stream *pstream = pvatek->cross->stream;
	vatek_result nres = vatek_unsupport;

	if (pstream)
	{
		Pchip_info pchipinfo = vatek_device_get_info(hchip);
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
			if (pmod->type == modulator_isdb_t || pmod->type == modulator_dtmb || (stream_mode == stream_remux && pmod->type == modulator_dvb_t2))
				mode = stream_mode_output_nodma;
			nres = pstream->start_stream(pvatek->cross->hcross, mode);
			if (is_vatek_success(nres))
				pvatek->streammode = stream_mode_output;
		}
	}
	return nres;
}

vatek_result vatek_device_stream_write(hvatek_chip hchip, uint8_t *pbuf, int32_t size)
{
	vatek_result nres = vatek_badstatus;
	vatek_device *pvatek = (vatek_device *)hchip;
	hcross_device hcross = (hcross_device)pvatek->cross->hcross;
	cross_stream *pstream = pvatek->cross->stream;
	if (pvatek->streammode == stream_mode_output)
		nres = pstream->write_stream(hcross, pbuf, size);
	return nres;
}

vatek_result vatek_device_stream_stop(hvatek_chip hchip)
{
	vatek_result nres = vatek_badstatus;
	vatek_device *pvatek = (vatek_device *)hchip;
	hcross_device hcross = (hcross_device)pvatek->cross->hcross;
	cross_stream *pstream = pvatek->cross->stream;

	if (pvatek->streammode != stream_mode_idle)
	{
		nres = pstream->stop_stream(hcross);
		pvatek->streammode = stream_mode_idle;
	}
	return nres;
}

vatek_result vatek_device_usbbulk_send(hvatek_chip hchip, Pusbbulk_command pcmd, Pusbbulk_result presult, uint8_t *pbuf, int32_t len)
{
	#define USBBUF_DIR_NULL	0
	#define USBBUF_DIR_IN	1
	#define USBBUF_DIR_OUT	2
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		husb_device husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;

		nres = pbulk->send_command(husb, pcmd);
		if (is_vatek_success(nres))
			nres = pbulk->get_result(husb, presult);

		if (is_vatek_success(nres) && pbuf && len)
		{
			int32_t bufdir = USBBUF_DIR_NULL;
			if (pcmd->mode == usbbulk_flash)
			{
				if (pcmd->_h.flash.mode == usbflash_write)bufdir = USBBUF_DIR_OUT;
				else if (pcmd->_h.flash.mode == usbflash_read)bufdir = USBBUF_DIR_IN;
			}
			else if (pcmd->mode == usbbulk_aux)
			{
				if (pcmd->_h.aux.mode == usbaux_async)
					bufdir = USBBUF_DIR_OUT;
			}

			if (bufdir != USBBUF_DIR_NULL && pbuf && len)
			{
				if (bufdir == USBBUF_DIR_OUT)nres = pbulk->write(husb, pbuf, len);
				else if (bufdir == USBBUF_DIR_IN)nres = pbulk->read(husb, pbuf, len);
				if (is_vatek_success(nres))
					nres = pbulk->get_result(husb, presult);
			}
		}
	}
	return nres;
}

vatek_result vatek_device_usbbulk_get_result(hvatek_chip hchip, Pusbbulk_result presult)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		husb_device husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->get_result(husb, presult);
	}
	return nres;
}

vatek_result vatek_device_usbbulk_write(hvatek_chip hchip, uint8_t *pbuf, int32_t len)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		husb_device husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->write(husb, pbuf, len);
	}
	return nres;
}

vatek_result vatek_device_usbbulk_read(hvatek_chip hchip, uint8_t *pbuf, int32_t len)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		husb_device husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->read(husb, pbuf, len);
	}
	return nres;
}

vatek_result vatek_device_usbbulk_get_size(hvatek_chip hchip)
{
	vatek_device *pvatek = (vatek_device *)hchip;
	vatek_result nres = vatek_unsupport;
	if (pvatek->cross->bulk)
	{
		husb_device husb = pvatek->cross->hcross;
		cross_usbbulk *pbulk = pvatek->cross->bulk;
		nres = pbulk->get_size(husb);
	}
	return nres;
}

vatek_result vatek_device_malloc(cross_device *pcross, vatek_device **pvatek)
{
	vatek_device *newdev = (vatek_device *)malloc(sizeof(vatek_device));
	vatek_result nres = vatek_memfail;
	if (newdev)
	{
		memset(newdev, 0, sizeof(vatek_device));
		newdev->cross = pcross;
		newdev->streammode = stream_mode_idle;
		*pvatek = newdev;
		nres = vatek_success;
	}
	return nres;
}

void vatek_device_free(vatek_device *pvatek)
{
	free(pvatek);
}
