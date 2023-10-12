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
HAL_API const char *usb_api_ll_list_get_name(usb_handle_list_node *hlist, int32_t idx);
HAL_API vatek_result usb_api_ll_free_list(usb_handle_list_node *hlist);
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
HAL_API vatek_result usb_api_ll_command(usb_handle_list_node *husb, uint8_t cmd, uint32_t param0, uint8_t *rxbuf);
HAL_API vatek_result usb_api_ll_command_buffer(usb_handle_list_node *husb, uint8_t cmd, uint8_t *pbuf, uint8_t *rxbuf);

HAL_API vatek_result usb_api_ll_bulk_get_size(usb_handle_list_node *husb);
HAL_API vatek_result usb_api_ll_bulk_send_command(usb_handle_list_node *husb, usbbulk_command *pcmd);
HAL_API vatek_result usb_api_ll_bulk_get_result(usb_handle_list_node *husb, usbbulk_result *presult);
HAL_API vatek_result usb_api_ll_bulk_write(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);
HAL_API vatek_result usb_api_ll_bulk_read(usb_handle_list_node *husb, uint8_t *pbuf, int32_t len);

#endif
