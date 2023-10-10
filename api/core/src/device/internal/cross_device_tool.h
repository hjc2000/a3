
#ifndef _CROSS_DEVICE_TOOL_
#define _CROSS_DEVICE_TOOL_

#include <cross/cross_os_api.h>
#include <cross/cross_bridge.h>
#include <cross/cross_usb_device.h>
#include <core/base/chip_define.h>

#define CHIP_CMD_RDREG          0x00000001
#define CHIP_CMD_WRREG          0x00000002
#define CHIP_CMD_RDMEM          0x00000003
#define CHIP_CMD_WRMEM          0x00000004
#define CHIP_CMD_RDBUF          0x00000007
#define CHIP_CMD_WRBUF          0x00000008

#define CHIP_CMD_RDCODE         0x00000101
#define CHIP_CMD_WRCODE         0x00000102
#define CHIP_CMD_BRIDGE			0x00000103

enum cross_driver
{
	cdriver_bridge,
	cdriver_usb,
	cdriver_sim,
};

typedef void *hcross_device;
typedef void *hcross_list;

enum cross_stream_mode
{
	stream_mode_idle = 0,
	stream_mode_output = 1,
	stream_mode_input = 2,
	stream_mode_output_nodma = 3,
};

typedef vatek_result(*fp_start_stream)(hcross_device hdev, cross_stream_mode mode);
typedef vatek_result(*fp_read_stream)(hcross_device hdev, uint8_t *pbuf, int32_t len);
typedef vatek_result(*fp_write_stream)(hcross_device hdev, uint8_t *pbuf, int32_t len);
typedef vatek_result(*fp_stop_stream)(hcross_device hdev);

struct cross_stream
{
	fp_start_stream start_stream;
	fp_write_stream write_stream;
	fp_read_stream read_stream;
	fp_stop_stream stop_stream;
};

typedef vatek_result(*fpbulk_get_size)(husb_device husb);
typedef vatek_result(*fpbulk_send_command)(husb_device husb, Pusbbulk_command pcmd);
typedef vatek_result(*fpbulk_get_result)(husb_device husb, Pusbbulk_result presult);
typedef vatek_result(*fpbulk_write)(husb_device husb, uint8_t *pbuf, int32_t len);
typedef vatek_result(*fpbulk_read)(husb_device husb, uint8_t *pbuf, int32_t len);

/* interface for bulk stream used to updated rom or broadcast write aux stream*/

struct cross_usbbulk
{
	fpbulk_get_size get_size;
	fpbulk_send_command send_command;
	fpbulk_get_result get_result;
	fpbulk_write write;
	fpbulk_read read;
};

typedef vatek_result(*fp_read_register)(hcross_device hdev, int32_t addr, uint32_t *val);
typedef vatek_result(*fp_write_register)(hcross_device hdev, int32_t addr, uint32_t val);
typedef vatek_result(*fp_read_memory)(hcross_device hdev, int32_t addr, uint32_t *val);
typedef vatek_result(*fp_write_memory)(hcross_device hdev, int32_t addr, uint32_t val);
typedef vatek_result(*fp_write_buffer)(hcross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen);
typedef vatek_result(*fp_read_buffer)(hcross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen);
typedef vatek_result(*fp_sendcmd)(hcross_device hdev, int32_t cmd, int32_t addr, uint8_t *vals, int32_t wlen);

struct cross_core
{
	fp_read_register read_register;
	fp_write_register write_register;
	fp_read_memory read_memory;
	fp_write_memory write_memory;
	fp_write_buffer write_buffer;
	fp_read_buffer	read_buffer;
	fp_sendcmd sendcmd;
};

/// <summary>
///		储存 cross_device 节点的链表
/// </summary>
class CrossDeviceList
{
public:

};

/// <summary>
///		设备链表的节点。储存着一个设备的数据。
/// </summary>
class cross_device
{
public:
	cross_device *next;
	uint32_t bus;
	cross_driver driver;
	hcross_device hcross;
	hal_service_mode service;
	cross_core *core;
	cross_stream *stream;
	cross_usbbulk *bulk;

	/// <summary>
	///		获取设备名称。
	///		会判断设备是通过桥还是 USB 连接的，从而调用不同的 API 获取名称。
	///		如果是未知的连接类型，则返回 "_unknown" 字符串。
	/// </summary>
	/// <returns>设备名称</returns>
	const char *get_device_name()
	{
		if (driver == cdriver_bridge)
			return bridge_device_get_name((hbridge_device)hcross);
		else if (driver == cdriver_usb)
			return usb_api_ll_get_name((husb_device)hcross);

		return "_unknown";
	}

	hbridge_device cross_get_bridge_device_handle()
	{
		if (driver == cdriver_bridge)
			return (hbridge_device)hcross;

		return NULL;
	}

	husb_device cross_get_usb_device_handle()
	{
		if (driver == cdriver_usb)
			return (husb_device)hcross;
		return NULL;
	}
};

struct vatek_device_list
{
	int32_t nums;
	cross_device *cross;
	cross_device **listdevices;
};

class vatek_device
{
public:
	vatek_device(cross_device *pcross)
	{
		cross = pcross;
		streammode = stream_mode_idle;
	}

	cross_device *cross;
	chip_info info;
	cross_stream_mode streammode;
};

vatek_result cross_devices_create(cross_device **pcross);
vatek_result cross_devices_create_by_usbid(uint16_t vid, uint16_t pid, cross_device **pcross);
vatek_result cross_bridge_open(hbridge_device hbridge, cross_device **pcross);
vatek_result cross_usb_device_open(husb_device husb, cross_device **pcross);

void cross_bridge_close(cross_device *pcross);
void cross_usb_device_close(cross_device *pcross);

vatek_result cross_devices_get_size(cross_device *pcross);
vatek_result cross_devices_free(cross_device *pcross);

vatek_result cdevice_malloc(cross_device **pcross, hal_service_mode hal);

#endif
