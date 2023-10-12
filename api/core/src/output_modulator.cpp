//----------------------------------------------------------------------------
//
// Vision Advance Technology - Software Development Kit
// Copyright (c) 2014-2022, Vision Advance Technology Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//

#include <tool_ofdm.h>
#include <ui_props_modulator.h>

vatek_result modulator_param_reset(modulator_type type, modulator_param * pmod)
{
	Pofdm_modulation pofdm = NULL;
	vatek_result nres = tool_ofdm_get_modulation(type, &pofdm);
	if (is_vatek_success(nres))
	{
		if (type == modulator_isdb_t)
		{
			pmod->ifmode = ifmode_iqoffset;
			pmod->iffreq_offset = 143;
		}
		else
		{
			pmod->ifmode = ifmode_disable;
			pmod->iffreq_offset = 0;
		}

		pmod->dac_gain = 0;
		pmod->bandwidth_symbolrate = default_bw_sb[type];
		pmod->type = type;
		pofdm->checkparam;
		memcpy(&pmod->mod.raw_byte, pofdm->defaultparam, pofdm->defaultsize);
		nres = vatek_success;
	}

	return nres;
}

vatek_result modulator_param_reset_dvbt2(modulator_type type, modulator_param * pmod)
{
	vatek_result nres = vatek_success;

	pmod->ifmode = ifmode_disable;
	pmod->iffreq_offset = 0;

	pmod->dac_gain = 0;
	pmod->bandwidth_symbolrate = default_bw_sb[type];
	pmod->type = type;


	memcpy(&pmod->mod.dvb_t2, &default_dvb_t2_param, sizeof(dvb_t2_param));

	return nres;
}

const Pui_prop_item modulator_param_get_ui_props(modulator_type type)
{
	Pofdm_modulation pofdm = NULL;
	vatek_result nres = tool_ofdm_get_modulation(type, &pofdm);
	if (is_vatek_success(nres))return pofdm->uiprops;
	else VWAR("unsupport modulation type : %d", type);
	return NULL;
}

vatek_result modulator_param_set(vatek_device * hchip, modulator_param * pmod)
{
	Pofdm_modulation pofdm = NULL;
	vatek_result nres = tool_ofdm_get_modulation(pmod->type, &pofdm);
	if (is_vatek_success(nres))
	{
		nres = ui_props_write_hal(hchip, _ui_struct(modulator_param), (uint8_t *)pmod);
		if (is_vatek_success(nres))
			nres = ui_props_write_hal(hchip, pofdm->uiprops, (uint8_t *)&pmod->mod);
	}
	return nres;
}

vatek_result modulator_param_get(vatek_device * hchip, modulator_param * pmod)
{
	vatek_result nres = ui_props_read_hal(hchip, _ui_struct(modulator_param), (uint8_t *)pmod);
	if (is_vatek_success(nres))
	{
		Pofdm_modulation pofdm = NULL;
		nres = tool_ofdm_get_modulation(pmod->type, &pofdm);
		if (is_vatek_success(nres))
			nres = ui_props_read_hal(hchip, pofdm->uiprops, (uint8_t *)&pmod->mod);
	}
	return nres;
}

uint32_t modulator_param_get_bitrate(modulator_param * pmod)
{
	Pofdm_modulation pofdm = NULL;
	vatek_result nres = tool_ofdm_get_modulation(pmod->type, &pofdm);
	if (is_vatek_success(nres))
		return (vatek_result)pofdm->getbitrate(pmod);
	return 0;
}
