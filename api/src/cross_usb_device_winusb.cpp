#include <cross_os_api.h>
#include <stdio.h>
#include <cross_usb_device_winusb.h>
#include <cfgmgr32.h>

#pragma comment(lib,"winusb.lib")
#pragma comment(lib,"setupapi.lib")

#define VATEK_USB_DEVICE_TAG        "vatek-usb"
#define VATEK_USB_RESCUE_TAG        "vatek-rescue"

const GUID GUID_DEVINTERFACE_USBApplication1 = { 0xDEE824EF, 0x729B, 0x4A0E, {0x9C, 0x14, 0xB7, 0x11, 0x7D, 0x33, 0xA8, 0x17, } };

extern usbdevice_id *usb_ll_list_get_id(uint16_t vid, uint16_t pid);
extern void usb_ll_convert_bufffer(uint8_t *psrc, uint8_t *pdest, int32_t len);
typedef int32_t(*fpenum_check)(USB_DEVICE_DESCRIPTOR *pdesc, usbdevice_type *type, usbdevice_type checkparam);
extern vatek_result usb_api_ll_enum_common(fpenum_check fpcheck, usb_handle_list_node **hlist, usbdevice_type checkparam);

extern int32_t usb_enum_check_normal(USB_DEVICE_DESCRIPTOR *pdesc, usbdevice_type *type, usbdevice_type checkparam);
extern int32_t usb_enum_check_id(USB_DEVICE_DESCRIPTOR *pdesc, usbdevice_type *type, usbdevice_type checkparam);

vatek_result usb_api_ll_enum(usbdevice_type type, usb_handle_list_node **hlist)
{
	return usb_api_ll_enum_common(usb_enum_check_normal, hlist, type);
}

vatek_result usb_api_ll_enum_by_id(uint16_t vid, uint16_t pid, usb_handle_list_node **hlist)
{
	return usb_api_ll_enum_common(usb_enum_check_id, hlist, (usbdevice_type)((vid << 16) | pid));
}

int32_t usb_enum_check_normal(USB_DEVICE_DESCRIPTOR *pdesc, usbdevice_type *type, usbdevice_type checkparam)
{
	usbdevice_type utype = (usbdevice_type)checkparam;
	usbdevice_id *puid = usb_ll_list_get_id(pdesc->idVendor, pdesc->idProduct);
	if (puid && (puid->type == utype || utype == usb_type_all))
	{
		*type = puid->type;
		return 1;
	}
	return 0;
}

int32_t usb_enum_check_id(USB_DEVICE_DESCRIPTOR *pdesc, usbdevice_type *type, usbdevice_type checkparam)
{
	uint16_t vid = (checkparam >> 16) & 0xFFFF;
	uint16_t pid = (checkparam & 0xFFFF);

	if (pdesc->idVendor == vid && pdesc->idProduct == pid)
	{
		*type = usb_type_broadcast;
		return 1;
	}
	return 0;
}

vatek_result usb_api_ll_list_get_device(usb_handle_list_node *hlist, int32_t idx, usb_handle_list_node **husb)
{
	usb_handle_list_node *pusbs = (usb_handle_list_node *)hlist;
	USB_DEVICE_DESCRIPTOR deviceDesc;
	ULONG                 lengthReceived;

	int32_t nums = 0;
	while (pusbs)
	{
		if (nums == idx)
		{
			*husb = pusbs;
			return vatek_success;
		}
		pusbs = pusbs->next;
	}
	return vatek_badparam;
}

vatek_result usb_api_ll_open(usb_handle_list_node *husb)
{
	return vatek_success;
}

usbdevice_id *usb_ll_list_get_id(uint16_t vid, uint16_t pid)
{
	int32_t pos = 0;

	while (usb_device_ids[pos].type != usb_type_unknown)
	{
		if (usb_device_ids[pos].vid == vid &&
			usb_device_ids[pos].pid == pid)
			return (usbdevice_id *)&usb_device_ids[pos];
		pos++;
	}

	return NULL;
}

vatek_result usb_api_ll_close(usb_handle_list_node *husb)
{
	usb_handle_list_node *pusb = (usb_handle_list_node *)husb;
	if (pusb->ref == 1)
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)pusb->husb;
		int32_t r = WinUsb_Free(hdevice);
		pusb->ref = 0;
		if (r < 0)return vatek_hwfail;
		return vatek_success;
	}

	return vatek_badstatus;
}

const char *usb_api_ll_get_name(usb_handle_list_node *husb)
{
	usb_handle_list_node *pusb = (usb_handle_list_node *)husb;
	return &pusb->name[0];
}

vatek_result usb_api_ll_set_dma(usb_handle_list_node *husb, int32_t isdma)
{
	vatek_result nres = vatek_success;

	if (isdma)
		nres = husb->usb_api_ll_command(VATCMD_CLASSV2_SET_MODE, CLASSV2_MODE_HWDMA, NULL);
	else
		nres = husb->usb_api_ll_command(VATCMD_CLASSV2_SET_MODE, CLASSV2_MODE_NORMAL, NULL);

	if (is_vatek_success(nres))
		husb->is_dma = isdma;
	else
		husb->is_dma = 0;

	return nres;
}

void usb_api_ll_lock(usb_handle_list_node *husb)
{
	cross_os_lock_mutex(husb->lock);
}

void usb_api_ll_unlock(usb_handle_list_node *husb)
{
	cross_os_release_mutex(husb->lock);
}

vatek_result usb_api_ll_write(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len)
{
	vatek_result nres = vatek_badparam;
	if (len <= CHIP_STREAM_SLICE_LEN && (len % 64) == 0)
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
		int32_t rlen = 0;
		if (!husb->is_dma)
		{
			usb_ll_convert_bufffer(pbuf, husb->none_dmabuf, len);
			pbuf = husb->none_dmabuf;
		}

		nres = (vatek_result)WinUsb_WritePipe(hdevice, USBDEV_BULK_WRITE_EP, pbuf, len, (PULONG)&rlen, NULL);
		if (is_vatek_success(nres))
			nres = (vatek_result)rlen;
	}

	return nres;
}

vatek_result usb_api_ll_read(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len)
{
	vatek_result nres = vatek_success;
	if (len > CHIP_STREAM_SLICE_LEN || (len % 64) != 0)nres = vatek_badparam;
	else
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
		int32_t rlen = 0;
		nres = (vatek_result)WinUsb_ReadPipe(hdevice, USBDEV_BULK_READ_EP, pbuf, len, (PULONG)&rlen, NULL);
		if (is_vatek_success(nres))nres = (vatek_result)rlen;
	}
	return nres;
}

vatek_result usb_api_ll_bulk_get_size(usb_handle_list_node *husb)
{
	return (vatek_result)husb->bulksize;
}

vatek_result usb_api_ll_bulk_send_command(usb_handle_list_node *husb, usbbulk_command *pcmd)
{
	vatek_result nres = vatek_badstatus;
	if (!husb->is_dma)
	{
		nres = usbbulk_command_set(pcmd, husb->none_dmabuf);
		if (is_vatek_success(nres))
		{
			WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
			int32_t rlen = 0;
			uint8_t *prawbuf = &husb->none_dmabuf[husb->bulksize];
			usb_ll_convert_bufffer(husb->none_dmabuf, prawbuf, husb->bulksize);

			nres = (vatek_result)WinUsb_WritePipe(hdevice, USBDEV_BULK_WRITE_EP, prawbuf, husb->bulksize, (PULONG)&rlen, NULL);
			if (is_vatek_success(nres))nres = (vatek_result)rlen;
		}
	}
	return nres;
}

vatek_result usb_api_ll_bulk_get_result(usb_handle_list_node *husb, usbbulk_result *presult)
{
	vatek_result nres = vatek_badstatus;
	if (!husb->is_dma)
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
		int32_t rlen = 0;
		uint8_t *prawbuf = &husb->none_dmabuf[husb->bulksize];
		nres = (vatek_result)WinUsb_ReadPipe(
			hdevice,
			USBDEV_BULK_READ_EP,
			husb->none_dmabuf,
			husb->bulksize,
			(PULONG)&rlen,
			NULL
		);

		if (is_vatek_success(nres))
		{
			usb_ll_convert_bufffer(husb->none_dmabuf, prawbuf, husb->bulksize);
			nres = usbbulk_result_get(presult, prawbuf);
		}
	}

	return nres;
}

#define _align_bulk(p,n)	(((n + (p->bulksize - 1))/p->bulksize) * p->bulksize)

vatek_result usb_api_ll_bulk_write(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len)
{
	vatek_result nres = vatek_badstatus;
	if (!husb->is_dma)
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
		int32_t rlen = 0;
		uint8_t *ptrbuf = pbuf;
		uint8_t *prawbuf = husb->none_dmabuf;
		int32_t pos = 0;
		while (len > pos)
		{
			int32_t nrecv = len - pos;
			int32_t neach = nrecv;
			if (neach >= CHIP_STREAM_SLICE_LEN)neach = CHIP_STREAM_SLICE_LEN;
			else neach = _align_bulk(husb, neach);
			usb_ll_convert_bufffer(&pbuf[pos], prawbuf, nrecv);

			nres = (vatek_result)WinUsb_WritePipe(hdevice, USBDEV_BULK_WRITE_EP, prawbuf, neach, (PULONG)&rlen, NULL);
			if (!is_vatek_success(nres))
			{
				break;
			}
			else pos += neach;
		}
	}
	return nres;
}

vatek_result usb_api_ll_bulk_read(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len)
{
	vatek_result nres = vatek_badstatus;
	if (!husb->is_dma)
	{
		WINUSB_INTERFACE_HANDLE *hdevice = (WINUSB_INTERFACE_HANDLE *)husb->husb;
		int32_t rlen = 0;
		uint8_t *ptrbuf = pbuf;
		uint8_t *prawbuf = husb->none_dmabuf;
		int32_t pos = 0;
		while (len > pos)
		{
			int32_t nrecv = len - pos;
			int32_t neach = nrecv;
			if (neach >= CHIP_STREAM_SLICE_LEN)neach = CHIP_STREAM_SLICE_LEN;
			else neach = _align_bulk(husb, neach);
			nres = (vatek_result)WinUsb_ReadPipe(hdevice, USBDEV_BULK_READ_EP, prawbuf, neach, (PULONG)&rlen, NULL);
			if (!is_vatek_success(nres))break;
			else
			{
				usb_ll_convert_bufffer(prawbuf, &pbuf[pos], nrecv);
				pos += neach;
			}
		}
	}
	return nres;
}

vatek_result usb_api_ll_command_buffer(usb_handle_list_node *husb, uint8_t cmd, uint8_t *pbuf, uint8_t *rxbuf)
{
	vatek_result nres = vatek_success;
	uint16_t wval = (pbuf[1] << 8) | pbuf[0];
	uint16_t widx = (pbuf[3] << 8) | pbuf[2];

	if (husb->husb == INVALID_HANDLE_VALUE)
	{
		return (vatek_result)FALSE;
	}

	WINUSB_SETUP_PACKET SetupPacket_tx;
	WINUSB_SETUP_PACKET SetupPacket_rx;

	ZeroMemory(&SetupPacket_tx, sizeof(WINUSB_SETUP_PACKET));
	ZeroMemory(&SetupPacket_rx, sizeof(WINUSB_SETUP_PACKET));

	ULONG cbSent = 0;

	//Create the setup packet
	SetupPacket_tx.RequestType = 0x80 | 0x40;
	SetupPacket_tx.Request = cmd;
	SetupPacket_tx.Value = wval;
	SetupPacket_tx.Index = widx;
	SetupPacket_tx.Length = 8;

	SetupPacket_rx.RequestType = 0x40;
	SetupPacket_rx.Request = cmd;
	SetupPacket_rx.Value = wval;
	SetupPacket_rx.Index = widx;
	SetupPacket_rx.Length = 0;

	if (rxbuf != NULL)
		nres = (vatek_result)WinUsb_ControlTransfer(husb->husb, SetupPacket_tx, rxbuf, 8, &cbSent, NULL);
	else nres = (vatek_result)WinUsb_ControlTransfer(husb->husb, SetupPacket_rx, NULL, 0, &cbSent, NULL);

	if (!is_vatek_success(nres))nres = vatek_hwfail;
	return nres;
}

void usb_ll_convert_bufffer(uint8_t *psrc, uint8_t *pdest, int32_t len)
{
	int32_t pos = 0;
	while (len > pos)
	{
		pdest[0] = psrc[3];
		pdest[1] = psrc[2];
		pdest[2] = psrc[1];
		pdest[3] = psrc[0];

		pos += 4;
		psrc += 4;
		pdest += 4;
	}
}

VOID
CloseDevice(
	_Inout_ DEVICE_DATA *DeviceData
)
{
	if (FALSE == DeviceData->HandlesOpen)
	{
		//
		// Called on an uninitialized DeviceData
		//
		return;
	}

	WinUsb_Free(DeviceData->WinusbHandle);
	CloseHandle(DeviceData->DeviceHandle);
	DeviceData->HandlesOpen = FALSE;

	return;
}

vatek_result usb_api_ll_enum_common(fpenum_check fpcheck, usb_handle_list_node **hlist, usbdevice_type checkparam)
{
	USB_DEVICE_DESCRIPTOR deviceDesc;
	ULONG                 lengthReceived;
	DEVICE_DATA           deviceData;

	WINUSB_PIPE_INFORMATION  Pipe;
	ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));

	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	ULONG                            length;
	ULONG                            requiredLength = 0;
	ULONG						     index = 0;
	vatek_result nres = vatek_success;
	int32_t i = 0;
	usb_handle_list_node *proot = NULL;
	usb_handle_list_node *pnext = NULL;
	int32_t enumnums = 0;

	deviceData.HandlesOpen = FALSE;
	deviceInfo = SetupDiGetClassDevsA(
		&GUID_DEVINTERFACE_USBApplication1,
		NULL,
		NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);

	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	while (SetupDiEnumDeviceInterfaces(
		deviceInfo,
		NULL,
		&GUID_DEVINTERFACE_USBApplication1,
		index,
		&interfaceData))
	{
		nres = (vatek_result)SetupDiGetDeviceInterfaceDetail(deviceInfo, &interfaceData, NULL, 0, &requiredLength, NULL);

		if (!is_vatek_success(nres))
		{
			nres = (vatek_result)HRESULT_FROM_WIN32(GetLastError());
			SetupDiDestroyDeviceInfoList(deviceInfo);
			return nres;
		}

		detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED, requiredLength);

		if (NULL == detailData)
		{
			nres = vatek_memfail;
			SetupDiDestroyDeviceInfoList(deviceInfo);
			return nres;
		}

		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		length = requiredLength;

		nres = (vatek_result)SetupDiGetDeviceInterfaceDetail(
			deviceInfo,
			&interfaceData,
			detailData,
			length,
			&requiredLength,
			NULL
		);

		nres = (vatek_result)StringCbCopy(
			deviceData.DevicePath,
			sizeof(deviceData.DevicePath),
			detailData->DevicePath
		);

		deviceData.DeviceHandle = CreateFile(
			deviceData.DevicePath,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (INVALID_HANDLE_VALUE == deviceData.DeviceHandle)
		{
			nres = vatek_memfail;
			return nres;
		}

		nres = (vatek_result)WinUsb_Initialize(deviceData.DeviceHandle, &deviceData.WinusbHandle);

		deviceData.HandlesOpen = TRUE;

		if (!is_vatek_success(nres))
		{
			wprintf(L"Device not connected or driver not installed\n");
			return (vatek_result)0;
		}

		nres = (vatek_result)WinUsb_GetDescriptor(
			deviceData.WinusbHandle,
			USB_DEVICE_DESCRIPTOR_TYPE,
			0, 0,
			(PBYTE)&deviceDesc,
			sizeof(deviceDesc),
			&lengthReceived
		);

		usbdevice_type devtype = usb_type_unknown;
		if (fpcheck(&deviceDesc, &devtype, (usbdevice_type)checkparam))
		{
			HANDLE hlock = NULL;

			if (!is_vatek_success(nres) || lengthReceived != sizeof(deviceDesc))
			{
				wprintf(L"Error among LastError %d or lengthReceived %d\n",
						FALSE == nres ? GetLastError() : 0,
						lengthReceived);
				CloseDevice(&deviceData);
				return (vatek_result)0;
			}

			if (is_vatek_success(nres))nres = cross_os_create_mutex(&hlock);
			if (is_vatek_success(nres))
			{
				usb_handle_list_node *newdevice = (usb_handle_list_node *)malloc(sizeof(usb_handle_list_node));
				if (newdevice != NULL)
				{
					WinUsb_QueryPipe(deviceData.WinusbHandle, 0, USBDEV_BULK_WRITE_EP, &Pipe);
					const char *name = VATEK_USB_DEVICE_TAG;
					if (devtype == usb_type_rescure)name = VATEK_USB_RESCUE_TAG;
					memset(newdevice, 0, sizeof(usb_handle_list_node));
					newdevice->husb = (WINUSB_INTERFACE_HANDLE *)deviceData.WinusbHandle;
					newdevice->lock = hlock;
					newdevice->is_dma = 0;
					newdevice->none_dmabuf = (uint8_t *)malloc(CHIP_STREAM_SLICE_LEN);
					newdevice->bulksize = Pipe.MaximumPacketSize;
					sprintf(&newdevice->name[0], "%s", name);

					if (pnext == NULL)proot = newdevice;
					else pnext->next = newdevice;
					pnext = newdevice;
					enumnums++;
				}
				else
					nres = vatek_memfail;

				if (!is_vatek_success(nres))
					cross_os_free_mutex(hlock);
			}
		}
		else
		{
			WinUsb_Free(deviceData.WinusbHandle);
			CloseHandle(deviceData.DeviceHandle);
		}

		if (!is_vatek_success(nres))
			break;
		index++;
	}

	LocalFree(detailData);
	SetupDiDestroyDeviceInfoList(deviceInfo);

	if (is_vatek_success(nres))
		nres = (vatek_result)enumnums;
	else
		nres = vatek_hwfail;

	*hlist = proot;

	return nres;
}