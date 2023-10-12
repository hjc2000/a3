#include <tool_binary_crc.h>
#include <tool_crc32.h>

vatek_result tool_binary_reset_crc(uint32_t* crc)
{
	tool_crc32_step_reset(crc);
	return vatek_success;
}

vatek_result tool_binary_crc_sector(int32_t idx, uint8_t* pbuf, uint32_t* crc)
{
	vatek_result nres = vatek_success;

	if(idx == 0)memset(&pbuf[0], 0, TOOL_BINARY_CRC_HEADER_OFFSET);
	nres = tool_crc32_step_put_buffer(crc, pbuf, BINARY_SECTION_SIZE);
	return nres;
}

static uint8_t binary_app_crc_sections[] = { 0,1,16,17, };

#define APP_CRC_SECTION_NUMS	sizeof(binary_app_crc_sections)

vatek_result tool_binary_app_get_crc(uint32_t* crc, fpget_section fpget,void* param,uint8_t* psection)
{
	int32_t i = 0;
	vatek_result nres = vatek_success;

	tool_crc32_step_reset(crc);

	for (i = 0; i < APP_CRC_SECTION_NUMS; i++)
	{
		nres = (vatek_result)fpget(param,binary_app_crc_sections[i],psection);
		if (is_vatek_success(nres))
			nres = tool_binary_crc_sector(i, psection, crc);
		if (!is_vatek_success(nres))break;
	}
	return nres;
}
