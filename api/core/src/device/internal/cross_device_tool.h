
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

typedef cross_stream *Pcross_stream;

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

typedef cross_usbbulk *Pcross_usbbulk;

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

typedef cross_core *Pcross_core;

struct cross_device
{
	cross_device *next;
	uint32_t bus;
	cross_driver driver;
	hcross_device hcross;
	hal_service_mode service;
	Pcross_core core;
	Pcross_stream stream;
	Pcross_usbbulk bulk;
};

typedef cross_device *Pcross_device;

struct vatek_device
{
	Pcross_device cross;
	chip_info info;
	cross_stream_mode streammode;
};

typedef vatek_device *Pvatek_device;

#ifdef __cplusplus
extern "C" {
	#endif

	vatek_result cross_devices_create(Pcross_device *pcross);
	vatek_result cross_devices_create_by_usbid(uint16_t vid, uint16_t pid, Pcross_device *pcross);
	vatek_result cross_bridge_open(hbridge_device hbridge, Pcross_device *pcross);
	vatek_result cross_usb_device_open(husb_device husb, Pcross_device *pcross);

	hbridge_device cross_get_bridge_handle(Pcross_device pcross);
	husb_device cross_get_usb_device(Pcross_device pcross);

	void cross_bridge_close(Pcross_device pcross);
	void cross_usb_device_close(Pcross_device pcross);

	vatek_result cross_devices_get_size(Pcross_device pcross);
	vatek_result cross_devices_free(Pcross_device pcross);

	/// <summary>
	///		获取设备名称。
	///		会判断设备是通过桥还是 USB 连接的，从而调用不同的 API 获取名称。
	///		如果是未知的连接类型，则返回 "_unknown" 字符串。
	/// </summary>
	/// <param name="pcross"></param>
	/// <returns></returns>
	const char *cdevice_get_name(Pcross_device pcross);
	vatek_result cdevice_malloc(Pcross_device *pcross, hal_service_mode hal);
	void cdevice_free(Pcross_device pcross);

	#ifdef __cplusplus
}
#endif

#endif
