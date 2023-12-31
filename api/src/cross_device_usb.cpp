#include "cross_device_tool.h"

/* usb_device_chip_api */
extern vatek_result usbdevice_read_register(usb_handle_list_node *hdev, int32_t addr, uint32_t *val);
extern vatek_result usbdevice_write_register(usb_handle_list_node *hdev, int32_t addr, uint32_t val);
extern vatek_result usbdevice_read_memory(usb_handle_list_node *hdev, int32_t addr, uint32_t *val);
extern vatek_result usbdevice_write_memory(usb_handle_list_node *hdev, int32_t addr, uint32_t val);
extern vatek_result usbdevice_sendcmd(usb_handle_list_node *hdev, int32_t cmd, int32_t addr, uint8_t *vals, int32_t wlen);
extern vatek_result usbdevice_write_buffer(usb_handle_list_node *hdev, int32_t addr, uint8_t *buf, int32_t wlen);
extern vatek_result usbdevice_read_buffer(usb_handle_list_node *hdev, int32_t addr, uint8_t *buf, int32_t wlen);
extern vatek_result usbdevice_free(usb_handle_list_node *hdev);

extern vatek_result usbdevice_stream_start(usb_handle_list_node *hdev, cross_stream_mode streammode);
extern vatek_result usbdevice_stream_stop(usb_handle_list_node *hdev);
extern vatek_result usbdevice_stream_write(usb_handle_list_node *hdev, uint8_t *pbuf, int32_t len);
extern vatek_result usbdevice_stream_read(usb_handle_list_node *hdev, uint8_t *pbuf, int32_t len);

static cross_core udev_core =
{
	.read_register = usbdevice_read_register,
	.write_register = usbdevice_write_register,
	.read_memory = usbdevice_read_memory,
	.write_memory = usbdevice_write_memory,
	.write_buffer = usbdevice_write_buffer,
	.read_buffer = usbdevice_read_buffer,
	.sendcmd = usbdevice_sendcmd,
};

static cross_stream udev_stream =
{
	.start_stream = usbdevice_stream_start,
	.write_stream = usbdevice_stream_write,
	.read_stream = usbdevice_stream_read,
	.stop_stream = usbdevice_stream_stop,
};

static cross_usbbulk udev_bulk =
{
	.get_size = usb_api_ll_bulk_get_size,
	.send_command = usb_api_ll_bulk_send_command,
	.get_result = usb_api_ll_bulk_get_result,
	.write = usb_api_ll_bulk_write,
	.read = usb_api_ll_bulk_read,
};

vatek_result cross_usb_device_open(usb_handle_list_node *husb, cross_device **pcross)
{
	vatek_result nres = usb_api_ll_open(husb);
	if (is_vatek_success(nres))
	{
		uint32_t val = 0;
		nres = udev_core.read_memory((usb_handle_list_node *)husb, HALREG_SERVICE_MODE, &val);
		if (is_vatek_success(nres))
		{
			hal_service_mode halservice = (hal_service_mode)val;
			cross_device *newdev = NULL;
			nres = cross_device_malloc(&newdev, halservice);
			if (is_vatek_success(nres))
			{
				newdev->driver = cdriver_usb;
				newdev->bus = DEVICE_BUS_USB;
				newdev->service = halservice;
				newdev->hcross = husb;
				newdev->core = &udev_core;
				if (halservice == service_transform)
					newdev->stream = &udev_stream;
				newdev->bulk = &udev_bulk;
				*pcross = newdev;
			}
		}

		if (!is_vatek_success(nres))
			usb_api_ll_close(husb);
	}

	return nres;
}

void cross_usb_device_close(cross_device *pcross)
{
	usb_api_ll_close(pcross->hcross);
	delete pcross;
}

vatek_result usbdevice_stream_start(usb_handle_list_node *hdev, cross_stream_mode streammode)
{
	vatek_result nres = vatek_unsupport;
	if (streammode == stream_mode_output)
		nres = usb_api_ll_set_dma(hdev, 1);
	else if (streammode == stream_mode_output_nodma)
		nres = usb_api_ll_set_dma(hdev, 0);
	return nres;
}

vatek_result usbdevice_stream_stop(usb_handle_list_node *hdev)
{
	return usb_api_ll_set_dma(hdev, 0);
}

vatek_result usbdevice_stream_write(usb_handle_list_node *hdev, uint8_t *pbuf, int32_t len)
{
	vatek_result nres = vatek_success;
	usb_api_ll_lock(hdev);
	nres = usb_api_ll_write(hdev, pbuf, len);
	usb_api_ll_unlock(hdev);
	return nres;
}

vatek_result usbdevice_stream_read(usb_handle_list_node *hdev, uint8_t *pbuf, int32_t len)
{
	return vatek_unsupport;
}

vatek_result usbdevice_read_register(usb_handle_list_node *hdev, int32_t addr, uint32_t *val)
{
	vatek_result nres = vatek_success;
	uint32_t buf[2];

	usb_api_ll_lock(hdev);
	nres = hdev->usb_api_ll_command(VATCMD_CLASSV2_SETADDR, addr, NULL);
	if (is_vatek_success(nres))
	{
		nres = hdev->usb_api_ll_command(VATCMD_CLASSV2_RDREG, 0, (uint8_t *)&buf[0]);
		if (is_vatek_success(nres))
		{
			buf[0] = vatek_buffer_2_uint32((uint8_t *)&buf[0]);
			*val = vatek_buffer_2_uint32((uint8_t *)&buf[1]);
			if (buf[0] != addr)nres = vatek_badparam;
		}
	}

	usb_api_ll_unlock(hdev);
	return nres;
}

vatek_result usbdevice_write_register(usb_handle_list_node *hdev, int32_t addr, uint32_t val)
{
	vatek_result nres = vatek_success;
	usb_handle_list_node *husb = hdev;
	usb_api_ll_lock(husb);
	nres = husb->usb_api_ll_command(VATCMD_CLASSV2_SETADDR, addr, NULL);
	if (is_vatek_success(nres))
	{
		nres = husb->usb_api_ll_command(VATCMD_CLASSV2_WRREG, val, NULL);
	}

	usb_api_ll_unlock(husb);
	return nres;
}

vatek_result usbdevice_read_memory(usb_handle_list_node *hdev, int32_t addr, uint32_t *val)
{
	vatek_result nres = vatek_success;
	usb_handle_list_node *husb = hdev;
	uint32_t buf[2];
	usb_api_ll_lock(husb);

	nres = husb->usb_api_ll_command(VATCMD_CLASSV2_SETADDR, addr, NULL);
	if (is_vatek_success(nres))
	{
		nres = husb->usb_api_ll_command(VATCMD_CLASSV2_RDMEM, 0, (uint8_t *)&buf[0]);
		if (is_vatek_success(nres))
		{
			buf[0] = vatek_buffer_2_uint32((uint8_t *)&buf[0]);
			*val = vatek_buffer_2_uint32((uint8_t *)&buf[1]);
			if (buf[0] != addr)nres = vatek_badparam;
		}
	}
	usb_api_ll_unlock(husb);
	return nres;
}

vatek_result usbdevice_write_memory(usb_handle_list_node *hdev, int32_t addr, uint32_t val)
{
	vatek_result nres = vatek_success;
	usb_handle_list_node *husb = hdev;
	usb_api_ll_lock(husb);
	nres = husb->usb_api_ll_command(VATCMD_CLASSV2_SETADDR, addr, NULL);
	if (is_vatek_success(nres))
	{
		nres = husb->usb_api_ll_command(VATCMD_CLASSV2_WRMEM, val, NULL);
	}
	usb_api_ll_unlock(husb);
	return nres;
}

vatek_result usbdevice_sendcmd(usb_handle_list_node *hdev, int32_t cmd, int32_t addr, uint8_t *vals, int32_t wlen)
{
	vatek_result nres = vatek_success;
	uint32_t buf[2];
	int32_t endaddr = addr + wlen;
	uint32_t usbcmd = 0;
	int32_t iswrite = 0;
	uint32_t *wvalues = (uint32_t *)vals;

	if (cmd == CHIP_CMD_RDREG)usbcmd = VATCMD_CLASSV2_RDREG;
	else if (cmd == CHIP_CMD_WRREG)usbcmd = VATCMD_CLASSV2_WRREG;
	else if (cmd == CHIP_CMD_RDMEM || cmd == CHIP_CMD_RDBUF)usbcmd = VATCMD_CLASSV2_RDMEM;
	else if (cmd == CHIP_CMD_WRMEM || cmd == CHIP_CMD_WRBUF)usbcmd = VATCMD_CLASSV2_WRMEM;
	else return vatek_badparam;

	if (usbcmd == VATCMD_CLASSV2_WRREG ||
		usbcmd == VATCMD_CLASSV2_WRMEM)iswrite = 1;

	usb_api_ll_lock(hdev);
	nres = hdev->usb_api_ll_command(VATCMD_CLASSV2_SETADDR, addr, NULL);
	if (is_vatek_success(nres))
	{
		while (addr < endaddr)
		{
			if (iswrite)
			{
				if (cmd == CHIP_CMD_WRBUF)nres = usb_api_ll_command_buffer(hdev, usbcmd, (uint8_t *)wvalues, NULL);
				else nres = hdev->usb_api_ll_command(usbcmd, *wvalues, NULL);
			}
			else
			{
				nres = hdev->usb_api_ll_command(usbcmd, addr, (uint8_t *)&buf[0]);
				if (is_vatek_success(nres))
				{

					buf[0] = vatek_buffer_2_uint32((uint8_t *)&buf[0]);
					if (cmd != CHIP_CMD_RDBUF)buf[1] = vatek_buffer_2_uint32((uint8_t *)&buf[1]);
					if (buf[0] == addr)*wvalues = buf[1];
				}
			}

			if (!is_vatek_success(nres))break;
			wvalues++;
			addr++;
		}
	}
	usb_api_ll_unlock(hdev);
	return nres;
}

vatek_result usbdevice_write_buffer(usb_handle_list_node *hdev, int32_t addr, uint8_t *buf, int32_t wlen)
{
	return usbdevice_sendcmd(hdev, CHIP_CMD_WRBUF, addr, buf, wlen);
}

vatek_result usbdevice_read_buffer(usb_handle_list_node *hdev, int32_t addr, uint8_t *buf, int32_t wlen)
{
	return usbdevice_sendcmd(hdev, CHIP_CMD_RDBUF, addr, buf, wlen);
}

vatek_result usbdevice_free(usb_handle_list_node *hdev)
{
	return usb_api_ll_close(hdev);
}
