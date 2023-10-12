#include <vatek_sdk_bridge.h>
#include <cross_bridge.h>
#include "cross_device_tool.h"

struct bridge_handle
{
	vatek_device * hchip;
	cross_device *hcross;
	win_hid_device_list_node * hbridge;
	bdevice_info info;
};

vatek_result vatek_bridge_open(vatek_device * hchip, hvatek_bridge *hbridge)
{
	cross_device *pcross = ((vatek_device *)hchip)->cross;
	win_hid_device_list_node * hdevice = pcross->cross_get_bridge_device_handle();
	vatek_result nres = vatek_unsupport;
	if (hdevice)
	{
		bridge_handle *newbridge = (bridge_handle *)malloc(sizeof(bridge_handle));
		nres = vatek_memfail;
		if (newbridge)
		{
			memset(newbridge, 0, sizeof(bridge_handle));
			newbridge->hchip = hchip;
			newbridge->hcross = pcross;
			newbridge->hbridge = hdevice;
			nres = bridge_device_lock(hdevice);
			if (is_vatek_success(nres))
				nres = bridge_device_get_info(hdevice, &newbridge->info);
			if (is_vatek_success(nres))*hbridge = newbridge;
			else free(newbridge);
		}
	}
	return nres;
}

void vatek_bridge_close(hvatek_bridge hbridge)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	bridge_device_unlock(pbridge->hbridge);
	free(pbridge);
}

Pbdevice_info vatek_bridge_get_info(hvatek_bridge hbridge)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	return &pbridge->info;
}

vatek_result vatek_bridge_get_av_source(hvatek_bridge hbridge, int32_t idx, Pbridge_source psource)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	vatek_result nres = vatek_badparam;
	if (idx < (int32_t)pbridge->info.source_nums)
		nres = bridge_device_get_source(pbridge->hbridge, idx, psource);
	return nres;
}

const char *vatek_bridge_get_av_source_name(hvatek_bridge hbridge, Pbridge_source psource)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	return bridge_device_get_source_name(pbridge->hbridge, psource);
}

vatek_result vatek_bridge_start_av_source(hvatek_bridge hbridge, Pbridge_source psource)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	return bridge_device_start_source(pbridge->hbridge, psource);
}

vatek_result vatek_bridge_get_av_source_status(hvatek_bridge hbridge, Pbridge_source psource)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	return bridge_device_get_source_status(pbridge->hbridge, psource);
}

vatek_result vatek_bridge_stop_av_source(hvatek_bridge hbridge)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	return bridge_device_stop_source(pbridge->hbridge);
}

vatek_result vatek_bridge_write_section(hvatek_bridge hbridge, int32_t section, uint8_t *pbuffer)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	vatek_result nres = bridge_device_storage_write(pbridge->hbridge, section, pbuffer);
	return nres;
}

vatek_result vatek_bridge_read_section(hvatek_bridge hbridge, int32_t section, uint8_t *pbuffer)
{
	bridge_handle *pbridge = (bridge_handle *)hbridge;
	vatek_result nres = bridge_device_storage_read(pbridge->hbridge, section, pbuffer);
	return nres;
}