#ifndef _TOOL_BINARY_CRC_
#define _TOOL_BINARY_CRC_

#include <binary_rom.h>

#define TOOL_BINARY_CRC_HEADER_OFFSET	0x100

HAL_API vatek_result tool_binary_reset_crc(uint32_t *crc);
/* BINARY_SECTION_SIZE */
HAL_API vatek_result tool_binary_crc_sector(int32_t idx, uint8_t *pbuf, uint32_t *crc);

typedef vatek_result(*fpget_section)(void *param, int32_t idx, uint8_t *pbuf);
HAL_API vatek_result tool_binary_app_get_crc(uint32_t *crc, fpget_section fpget, void *param, uint8_t *psection);

#endif
