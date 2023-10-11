#include <service_base.h>
#include <ui/ui_service_base.h>

vatek_result broadcast_info_set(void_vatek_chip hchip, Pbroadcast_info pinfo)
{
	return ui_props_write_hal(hchip, _ui_struct(broadcast_info), (uint8_t *)pinfo);
}

vatek_result broadcast_info_get(void_vatek_chip hchip, Pbroadcast_info pinfo)
{
	return ui_props_read_hal(hchip, _ui_struct(broadcast_info), (uint8_t *)pinfo);
}

vatek_result broadcast_status_get(void_vatek_chip hchip, broadcast_status *status)
{
	uint32_t val = 0;
	vatek_result nres = readhal(HALREG_BCINFO_STATUS, &val);
	if (is_vatek_success(nres))
		*status = (broadcast_status)val;
	else
		*status = (broadcast_status)vatek_hwfail;
	return nres;
}

vatek_result broadcast_status_set(void_vatek_chip hchip, broadcast_status status)
{
	return writehal(HALREG_BCINFO_STATUS, (uint32_t)status);
}
