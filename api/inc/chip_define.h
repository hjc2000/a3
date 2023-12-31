#ifndef _CHIP_DEFINE_
#define _CHIP_DEFINE_

#include <vatek_base.h>
#include <halservice_base.h>

#define DEVICE_BUS_UNKNOWN		0x00000000
#define DEVICE_BUS_USB			0x00000001
#define DEVICE_BUS_BRIDGE		0x00000002
#define DEVICE_BUS_ALL			0x00000007

/**
 * @ingroup core_props
 * chip module id define
 */
enum vatek_ic_module
{
	ic_module_nodevice = -1,
	ic_module_a1 = HAL_CHIPID_A1,				/*!< a1 (101) */
	ic_module_b1 = HAL_CHIPID_B1,				/*!< b1 (201) */
	ic_module_b2 = HAL_CHIPID_B2,				/*!< b2 (202) */
	ic_module_b2_plus = HAL_CHIPID_B2_PLUS,		/*!< b2+ (202) */
	ic_module_a3 = HAL_CHIPID_A3,				/*!< a3 (103) */
	ic_module_b3_lite = HAL_CHIPID_B3,			/*!< b3 (203) */
	ic_module_b3_plus = HAL_CHIPID_B3_PLUS,		/*!< b3+ (203) */
	ic_module_e1 = HAL_CHIPID_E1,				/*!< E1 (401) */
	ic_module_unknown = 0x00ffff00,
};

#define is_chip_a_serial(val)					((val & 0x000F0000) == CHIPID_AX_BASE)
#define is_chip_b_serial(val)					((val & 0x000F0000) == CHIPID_BX_BASE)
#define _chip_id(ic)							((ic >> 8) & 0xFFF)

/**
 * @ingroup core_props
 * chip status
 */
enum chip_status
{
	chip_status_last = -4,
	chip_status_badstatus = -4,		/*!< chip status fail */
	chip_status_fail_hw = -3,		/*!< chip hw fail */
	chip_status_fail_service = -2,	/*!< chip service fail */
	chip_status_fail_loader = -1,	/*!< chip loader fail */
	chip_status_unknown = 0,		/*!< chip fail */
	chip_status_waitcmd = 1,		/*!< chip idle wait command */
	chip_status_running = 2,		/*!< chip busy broadcasting */
};

#define is_current_chip_status(status)			(status >= chip_status_badstatus && status <= chip_status_running)
#define is_chip_status_valid(status)    		(status == chip_status_waitcmd || status == chip_status_running)

/// <summary>
///		一个数组，应该是用来代替枚举类型表示芯片状态的。
/// </summary>
const static uint32_t chip_status_tags[] =
{
	SYS_STATUS_BADSTATUS,
	SYS_STATUS_UNKNOWN_FAIL,
	SYS_STATUS_SERVICE_FAIL,
	SYS_STATUS_LOADER_FAIL,
	SYS_STATUS_TAG,
	SYS_STATUS_IDLE,
	SYS_STATUS_RUN,
};

// chip_status_tags 数组的元素个数
#define STATUS_TAGS_NUMS						sizeof(chip_status_tags)/sizeof(uint32_t)

/// <summary>
///		服务模式。有：抢救模式，广播模式，传输模式。
/// </summary>
enum hal_service_mode
{
	service_unknown = 0,
	service_rescue = SERVICE_TAG_RESCUE,			/*!< service rescure */
	service_broadcast = SERVICE_TAG_BROADCAST,		/*!< service broadcast (b-serial) */
	service_transform = SERVICE_TAG_TRANSFORM,		/*!< service transform (a-serial) */
};

/// <summary>
///		芯片信息。
/// </summary>
struct chip_info
{
	chip_status status;				/*!< chip status */
	uint32_t errcode;				/*!< if chip status fail set error code */
	vatek_ic_module chip_module;	/*!< chip id */
	hal_service_mode hal_service;	/*!< chip service mode */
	uint32_t version;				/*!< firmware version */
	uint32_t peripheral_en;			/*!< support peripheral */
	uint32_t input_support;			/*!< support input mode */
	uint32_t output_support;		/*!< support output mode */
};

#define chip_is_halservice(info,en)				(info->hal_service == en)	
#define chip_is_en_output(info,en)				(((info)->output_support | en) == en)
#define chip_is_en_input(info,en)				(((info)->input_support | en) == en)
#define chip_is_en_peripheral(info,en)			((info)->peripheral_en | en) == en)


extern "C"
{
	HAL_API chip_status chip_status_get(vatek_device *hchip, uint32_t *errcode);
	HAL_API vatek_result chip_status_set(vatek_device *hchip, chip_status status, uint32_t errcode);

	HAL_API vatek_result chip_status_check(vatek_device *hchip, chip_status status);

	HAL_API vatek_result chip_info_reset(chip_info *pinfo, hal_service_mode service, vatek_ic_module icmodule);
	HAL_API vatek_result chip_info_get(vatek_device *hchip, chip_info *pinfo);
	HAL_API vatek_result chip_info_set(vatek_device *hchip, chip_info *pinfo);

	HAL_API vatek_result chip_send_command(vatek_device *hchip, uint32_t cmd, uint32_t cmdaddr, uint32_t resultaddr);
	HAL_API vatek_result chip_raise_command(vatek_device *hchip, uint32_t cmdaddr, uint32_t cmd, int32_t iswait);
	HAL_API vatek_result chip_check_command(vatek_device *hchip, uint32_t cmdaddr);
	HAL_API vatek_result chip_result_command(vatek_device *hchip, uint32_t resultaddr, uint32_t *errcode);
}

#endif
