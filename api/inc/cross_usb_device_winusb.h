#ifndef _CROSS_USB_DEVICE_WINUSB_
#define _CROSS_USB_DEVICE_WINUSB_

#include <vatek_base.h>
#include <device_usb.h>

#include <initguid.h>

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winusb.h>
#include <usb.h>

#include <SetupAPI.h>
#include <stdlib.h>

struct usb_handle_list_node;

struct DEVICE_DATA
{
	BOOL                    HandlesOpen;
	WINUSB_INTERFACE_HANDLE WinusbHandle;
	HANDLE                  DeviceHandle;
	TCHAR                   DevicePath[MAX_PATH];
};

struct PIPE_ID
{
	UCHAR  PipeInId;
	UCHAR  PipeOutId;
};

#define _usb_table_start	static const  usbdevice_id usb_device_ids[] = {
#define _usb_broadcast(pid)	{ usb_type_broadcast	,USBDEV_VID	,pid,},
#define _usb_rescure(pid)	{ usb_type_rescure		,USBDEV_VID	,pid,},
#define _usb_table_end		{ usb_type_unknown,0,0, },};

#define _usb_table 			((const usbdevice_id *)&usb_device_ids[0])

_usb_table_start
_usb_broadcast(0x2011)
_usb_broadcast(0x2021)
_usb_broadcast(0x1011)
_usb_broadcast(0x1031)
_usb_broadcast(0x2031)
_usb_rescure(0x1010)
_usb_rescure(0x1030)
_usb_rescure(0x2010)
_usb_rescure(0x2030)
_usb_table_end

#define USBDEVICE_ID_NUMS		(sizeof(usb_device_ids)/sizeof(usbdevice_id))

HRESULT
RetrieveDevicePath(
	_Out_bytecap_(BufLen) LPTSTR DevicePath,
	_In_                  ULONG  BufLen,
	_Out_opt_             PBOOL  FailureDeviceNotFound
);

VOID
CloseDevice(
	_Inout_ DEVICE_DATA *DeviceData
);

HAL_API vatek_result usb_api_ll_enum(usbdevice_type type, usb_handle_list_node **hlist);
HAL_API vatek_result usb_api_ll_enum_by_id(uint16_t vid, uint16_t pid, usb_handle_list_node **hlist);
HAL_API vatek_result usb_api_ll_list_get_device(usb_handle_list_node *hlist, int32_t idx, usb_handle_list_node **husb);
HAL_API vatek_result usb_api_ll_open(usb_handle_list_node *husb);
HAL_API vatek_result usb_api_ll_close(usb_handle_list_node *husb);
HAL_API const char *usb_api_ll_get_name(usb_handle_list_node *husb);

/* usb_device bulk transfer */
HAL_API void usb_api_ll_lock(usb_handle_list_node *husb);
HAL_API void usb_api_ll_unlock(usb_handle_list_node *husb);
HAL_API vatek_result usb_api_ll_set_dma(usb_handle_list_node *husb, int32_t isdma);
HAL_API vatek_result usb_api_ll_write(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);
HAL_API vatek_result usb_api_ll_read(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);

/* usb_device control transfer */
HAL_API vatek_result usb_api_ll_command_buffer(usb_handle_list_node *husb, uint8_t cmd, uint8_t *pbuf, uint8_t *rxbuf);

HAL_API vatek_result usb_api_ll_bulk_get_size(usb_handle_list_node *husb);
HAL_API vatek_result usb_api_ll_bulk_send_command(usb_handle_list_node *husb, usbbulk_command *pcmd);
HAL_API vatek_result usb_api_ll_bulk_get_result(usb_handle_list_node *husb, usbbulk_result *presult);
HAL_API vatek_result usb_api_ll_bulk_write(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);
HAL_API vatek_result usb_api_ll_bulk_read(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);

#endif

class usb_handle_list_node
{
public:
	usb_handle_list_node *next;
	char name[32];
	WINUSB_INTERFACE_HANDLE *husb;
	HANDLE lock;
	int32_t ref;
	int32_t is_dma;
	int32_t epsize;
	int32_t bulksize;
	uint8_t *none_dmabuf;

	void usb_api_ll_free_list()
	{
		usb_handle_list_node *hlist = this;
		while (hlist)
		{
			usb_handle_list_node *pnext = hlist->next;
			WinUsb_Free((WINUSB_INTERFACE_HANDLE *)hlist->husb);
			cross_os_free_mutex(hlist->lock);
			free(hlist->none_dmabuf);
			free(hlist);
			hlist = pnext;
		}
	}

	const char *usb_api_ll_list_get_name(int32_t idx)
	{
		usb_handle_list_node *husb = NULL;
		vatek_result nres = usb_api_ll_list_get_device(this, idx, &husb);
		if (is_vatek_success(nres))
		{
			return &((usb_handle_list_node *)husb)->name[0];
		}

		return NULL;
	}

	vatek_result usb_api_ll_command(uint8_t cmd, uint32_t param0, uint8_t *rxbuf)
	{
		vatek_result nres = vatek_success;
		uint16_t wval = ((param0 >> 16) << 8) | ((param0 >> 24) & 0xFF);
		uint16_t widx = ((param0 & 0xFF) << 8) | ((param0 >> 8) & 0xFF);

		if (husb == INVALID_HANDLE_VALUE)
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
			nres = (vatek_result)WinUsb_ControlTransfer(husb, SetupPacket_tx, rxbuf, 8, &cbSent, NULL);
		else
			nres = (vatek_result)WinUsb_ControlTransfer(husb, SetupPacket_rx, NULL, 0, &cbSent, NULL);

		if (!is_vatek_success(nres))
			nres = vatek_hwfail;

		return nres;
	}

};
