#ifndef _CROSS_USB_DEVICE_WINUSB_
#define _CROSS_USB_DEVICE_WINUSB_

#include <core/vatek_base.h>
#include <core/base/device_usb.h>

#include <initguid.h>

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winusb.h>
#include <usb.h>

#include <SetupAPI.h>
#include <stdlib.h>

typedef void *husb_device;
typedef void *void_usb_device_list;

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

#define _usb_table 			((const Pusbdevice_id)&usb_device_ids[0])

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

HAL_API vatek_result usb_api_ll_enum(usbdevice_type type, void_usb_device_list *hlist);
HAL_API vatek_result usb_api_ll_enum_by_id(uint16_t vid, uint16_t pid, void_usb_device_list *hlist);
HAL_API vatek_result usb_api_ll_list_get_device(void_usb_device_list hlist, int32_t idx, husb_device *husb);
HAL_API const char *usb_api_ll_list_get_name(void_usb_device_list hlist, int32_t idx);
HAL_API vatek_result usb_api_ll_free_list(void_usb_device_list hlist);
HAL_API vatek_result usb_api_ll_open(husb_device husb);
HAL_API vatek_result usb_api_ll_close(husb_device husb);
HAL_API const char *usb_api_ll_get_name(husb_device husb);

/* usb_device bulk transfer */
HAL_API void usb_api_ll_lock(husb_device husb);
HAL_API void usb_api_ll_unlock(husb_device husb);
HAL_API vatek_result usb_api_ll_set_dma(husb_device husb, int32_t isdma);
HAL_API vatek_result usb_api_ll_write(husb_device husb, uint8_t *pbuf, int32_t len);
HAL_API vatek_result usb_api_ll_read(husb_device husb, uint8_t *pbuf, int32_t len);

/* usb_device control transfer */
HAL_API vatek_result usb_api_ll_command(husb_device husb, uint8_t cmd, uint32_t param0, uint8_t *rxbuf);
HAL_API vatek_result usb_api_ll_command_buffer(husb_device husb, uint8_t cmd, uint8_t *pbuf, uint8_t *rxbuf);

HAL_API vatek_result usb_api_ll_bulk_get_size(husb_device husb);
HAL_API vatek_result usb_api_ll_bulk_send_command(husb_device husb, Pusbbulk_command pcmd);
HAL_API vatek_result usb_api_ll_bulk_get_result(husb_device husb, Pusbbulk_result presult);
HAL_API vatek_result usb_api_ll_bulk_write(husb_device husb, uint8_t *pbuf, int32_t len);
HAL_API vatek_result usb_api_ll_bulk_read(husb_device husb, uint8_t *pbuf, int32_t len);

#endif
