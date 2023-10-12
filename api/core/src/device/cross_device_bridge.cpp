#include "./internal/cross_device_tool.h"

extern vatek_result bridge_read_register(void_cross_device hdev, int32_t addr, uint32_t *val);
extern vatek_result bridge_write_register(void_cross_device hdev, int32_t addr, uint32_t val);
extern vatek_result bridge_read_memory(void_cross_device hdev, int32_t addr, uint32_t *val);
extern vatek_result bridge_write_memory(void_cross_device hdev, int32_t addr, uint32_t val);
extern vatek_result bridge_sendcmd(void_cross_device hdev, int32_t cmd, int32_t addr, uint8_t *vals, int32_t wlen);
extern vatek_result bridge_write_buffer(void_cross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen);
extern vatek_result bridge_read_buffer(void_cross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen);

static cross_core bridge_core =
{
	.read_register = bridge_read_register,
	.write_register = bridge_write_register,
	.read_memory = bridge_read_memory,
	.write_memory = bridge_write_memory,
	.write_buffer = bridge_write_buffer,
	.read_buffer = bridge_read_buffer,
	.sendcmd = bridge_sendcmd,
};

vatek_result cross_bridge_open(win_hid_device_list_node *hbridge, cross_device **pcross)
{
	vatek_result nres = bridge_device_open(hbridge);
	if (is_vatek_success(nres))
	{
		nres = bridge_device_lock(hbridge);
		if (is_vatek_success(nres))
		{
			uint32_t val = 0;
			nres = bridge_core.read_memory((void_cross_device)hbridge, HALREG_SERVICE_MODE, &val);
			if (is_vatek_success(nres))
			{
				hal_service_mode halservice = (hal_service_mode)val;
				cross_device *newdev = NULL;
				nres = cross_device_malloc(&newdev, halservice);
				if (is_vatek_success(nres))
				{
					newdev->driver = cdriver_bridge;
					newdev->bus = DEVICE_BUS_BRIDGE;
					newdev->service = halservice;
					newdev->hcross = hbridge;
					newdev->core = &bridge_core;
					newdev->stream = NULL;
					*pcross = newdev;
				}
			}

			if (!is_vatek_success(nres))
				bridge_device_unlock(hbridge);
		}

		if (!is_vatek_success(nres))
			bridge_device_close(hbridge);
	}

	return nres;
}

void cross_bridge_close(cross_device *pcross)
{
	win_hid_device_list_node *hbridge = (win_hid_device_list_node *)pcross->hcross;
	bridge_device_unlock(hbridge);
	bridge_device_close(hbridge);
	delete pcross;
}

vatek_result bridge_read_register(void_cross_device hdev, int32_t addr, uint32_t *val)
{
	uint8_t buf[4];
	vatek_result nres = bridge_device_bulk_transfer((win_hid_device_list_node *)hdev, MOD_RD_REG, addr, (uint8_t *)buf, 1);
	if (is_vatek_success(nres))
	{
		*val = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
	}

	return nres;
}

vatek_result bridge_write_register(void_cross_device hdev, int32_t addr, uint32_t val)
{
	uint8_t buf[4];
	/* tip : when used bulk transfer buffer order is follow i2c order (_vatek_chip is big endian) */
	buf[3] = val >> 24;
	buf[2] = val >> 16;
	buf[1] = val >> 8;
	buf[0] = val;
	return bridge_device_bulk_transfer((win_hid_device_list_node *)hdev, MOD_WR_REG, addr, buf, 1);
}

vatek_result bridge_read_memory(void_cross_device hdev, int32_t addr, uint32_t *val)
{
	uint8_t buf[4];
	vatek_result nres = bridge_device_bulk_transfer((win_hid_device_list_node *)hdev, MOD_RD_MEM, addr, (uint8_t *)buf, 1);
	if (is_vatek_success(nres))
	{
		*val = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
	}

	return nres;
}

vatek_result bridge_write_memory(void_cross_device hdev, int32_t addr, uint32_t val)
{
	uint8_t buf[4];
	buf[3] = val >> 24;
	buf[2] = val >> 16;
	buf[1] = val >> 8;
	buf[0] = val;
	return bridge_device_bulk_transfer((win_hid_device_list_node *)hdev, MOD_WR_MEM, addr, buf, 1);
}

extern vatek_result bridge_cmd_ip_transfer(void_cross_device hdev, int32_t cmd, uint8_t *pbuf);

vatek_result bridge_sendcmd(void_cross_device hdev, int32_t cmd, int32_t addr, uint8_t *vals, int32_t wlen)
{
	uint32_t bridgecmd = 0;

	if (cmd == CHIP_CMD_RDREG)
		bridgecmd = MOD_RD_REG;
	else if (cmd == CHIP_CMD_WRREG)
		bridgecmd = MOD_WR_REG;
	else if (cmd == CHIP_CMD_RDMEM)
		bridgecmd = MOD_RD_MEM;
	else if (cmd == CHIP_CMD_WRMEM)
		bridgecmd = MOD_WR_MEM;
	else if (cmd == CHIP_CMD_RDCODE || cmd == CHIP_CMD_RDBUF)
		bridgecmd = MOD_RD_CODE;
	else if (cmd == CHIP_CMD_WRCODE || cmd == CHIP_CMD_WRBUF)
		bridgecmd = MOD_WR_CODE;
	else if (cmd == CHIP_CMD_BRIDGE)
		return bridge_cmd_ip_transfer((win_hid_device_list_node *)hdev, addr, vals);
	else
		return vatek_badparam;

	return bridge_device_bulk_transfer((win_hid_device_list_node *)hdev, bridgecmd, addr, vals, wlen);
}

vatek_result bridge_write_buffer(void_cross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen)
{
	return bridge_sendcmd((win_hid_device_list_node *)hdev, CHIP_CMD_WRBUF, addr, buf, wlen);
}

vatek_result bridge_read_buffer(void_cross_device hdev, int32_t addr, uint8_t *buf, int32_t wlen)
{
	return bridge_sendcmd((win_hid_device_list_node *)hdev, CHIP_CMD_RDBUF, addr, buf, wlen);
}

vatek_result bridge_cmd_ip_transfer(void_cross_device hdev, int32_t cmd, uint8_t *pbuf)
{
	win_hid_device_list_node *hhid = (win_hid_device_list_node *)hdev;
	hid_bridge_cmd *pcmd = bridge_device_get_command(hhid);
	vatek_result nres = vatek_unknown;

	bridge_device_lock_command((win_hid_device_list_node *)hdev);

	BRIDGE_SETCMD(pcmd, cmd);

	memcpy(&pcmd->param.raw[0], pbuf, BRIDGE_PARAM_MAX_LEN);
	nres = bridge_device_send_bridge_command(hhid);

	if (is_vatek_success(nres))
	{
		Phid_bridge_result presult = bridge_device_get_result(hhid);
		nres = (vatek_result)presult->result;
		if (is_vatek_success(nres))
			memcpy(pbuf, &presult->data.raw[0], BRIDGE_PARAM_MAX_LEN);
	}

	bridge_device_unlock_command((win_hid_device_list_node *)hdev);
	return nres;
}
