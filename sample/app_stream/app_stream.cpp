#include <vatek_sdk_usbstream.h>
#include <vatek_sdk_device.h>
#include <cross_os_api.h>

#include "../common/inc/tool_printf.h"
#include "../common/inc/tool_tspacket.h"
#include "../common/inc/tool_stream.h"
#include <tool_dvb_t2.h>
#include<Exception.h>

static usbstream_param usbcmd;

extern vatek_result source_sync_get_buffer(void *param, uint8_t **pslicebuf);

/// <summary>
///		解析命令行
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="stream_source"></param>
/// <param name="pustream"></param>
/// <returns></returns>
extern vatek_result parser_cmd_source(int32_t argc, char **argv, tsstream_source *psource, usbstream_param *pustream);

int main(int argc, char *argv[])
{
	/* ./app_stream dvbt file qq.ts */
	//const char *cmd[] = {
	//	"./app_stream",
	//	"dvbt",
	//	"file",
	//	"qq.ts",
	//};

	/* ./app_stream dvbt udp udp://127.0.0.1:40000 */
	const char *cmd[] = {
		"./app_stream",
		"dvbt",
		"udp",
		"udp://127.0.0.1:1234",
	};

	vatek_device * hchip = NULL;
	hvatek_usbstream hustream = NULL;
	tsstream_source streamsource;
	vatek_result nres = vatek_success;
	hmux_core hmux = NULL;
	hmux_channel m_hchannel = NULL;

	usbcmd.mode = usbstream_mode::ustream_mode_sync;
	usbcmd.remux = usbstream_remux::ustream_remux_pcr;
	usbcmd.pcradjust = pcr_adjust_mode::pcr_adjust;
	usbcmd.r2param.freqkhz = 473000; /* output _rf frequency */
	usbcmd.r2param.mode = r2_cntl_mode::r2_cntl_path_0;
	usbcmd.modulator.bandwidth_symbolrate = 6;
	usbcmd.modulator.type = modulator_type::modulator_dvb_t;
	usbcmd.modulator.ifmode = dac_ifmode::ifmode_disable;
	usbcmd.modulator.iffreq_offset = 0;
	usbcmd.modulator.dac_gain = 0;
	usbcmd.modulator.mod.dvb_t.constellation = constellation_mode::dvb_t_qam64;
	usbcmd.modulator.mod.dvb_t.fft = fft_mode::fft_8k;
	usbcmd.modulator.mod.dvb_t.guardinterval = guard_interval::guard_interval_1_16;
	usbcmd.modulator.mod.dvb_t.coderate = code_rate::coderate_5_6;
	usbcmd.sync = usbstream_sync{};

	nres = parser_cmd_source(4, (char **)cmd, &streamsource, &usbcmd);
	/*
		step 1 :
		- initialized supported device and open
	*/

	vatek_device_list *hdevlist = nullptr;
	if (is_vatek_success(nres))
	{
		nres = vatek_device_list_enum(DEVICE_BUS_USB, service_transform, &hdevlist);
		if (is_vatek_success(nres))
		{
			if (nres == 0)
			{
				nres = vatek_nodevice;
				_disp_err("can not found device.");
			}
			else
			{
				nres = vatek_device_open(hdevlist, 0, &hchip);
				if (!is_vatek_success(nres))
				{
					_disp_err("open device fail : %d", nres);
				}
				else
				{
					chip_info *pinfo = vatek_device_get_info(hchip);
					printf_chip_info(pinfo);
					nres = streamsource.start(streamsource.hsource);
				}
			}
		}
		else
		{
			_disp_err("enum device fail : %d", nres);
		}
	}

	/*
		step 2:
			- open usb_stream open
			- config used usb_stream sync mode and start stream
	*/

	if (is_vatek_success(nres))
	{
		nres = vatek_usbstream_open(hchip, &hustream);
		if (!is_vatek_success(nres))
			_disp_err("open usb_stream fail - %d", nres);
		else
		{
			usbcmd.mode = ustream_mode_sync;
			usbcmd.sync.void_param = &streamsource;
			usbcmd.sync.getbuffer = source_sync_get_buffer;

			nres = vatek_usbstream_start(hustream, &usbcmd);

			printf_modulation_param(usbcmd);

			if (!is_vatek_success(nres))
				_disp_err("start usb_stream fail : %d", nres);
		}
	}

	/*
		step 3 :
			- source_sync_get_buffer would call from internal when usb_stream had valid buffer
			- main loop could polling usb_stream status and check information
			- when finished close usb_stream
	*/

	if (is_vatek_success(nres))
	{
		_disp_l("USB Stream Start.");
		_disp_l("Press any key to stop.\r\n");
		_disp_l("=====================================\r\n");
		int32_t is_stop = 0;
		Ptransform_info pinfo = NULL;
		uint32_t ntickms = cross_os_get_tick_ms();
		int count = 0;
		int error = 0;
		while (!is_stop)
		{
			usbstream_status status = vatek_usbstream_get_status(hustream, &pinfo);
			if (status == usbstream_status_running)
			{
				if (cross_os_get_tick_ms() - ntickms > 1000)
				{
					ntickms = cross_os_get_tick_ms();
					_disp_l("Data:[%d]  Current:[%d]",
							pinfo->info.data_bitrate,
							pinfo->info.cur_bitrate);
					if (pinfo->info.data_bitrate == 0 || pinfo->info.cur_bitrate == 0)
					{
						error++;
					}
					count++;
					if (error >= 30)
					{
						_disp_l("A3 Fail. Press any key to stop.\r\n");
					}
				}
			}

			if (!is_vatek_success(nres))
				break;
			if (try_getchar() != -1)
				is_stop = 1;
			else
				cross_os_sleep(1);
		}

		nres = vatek_usbstream_stop(hustream);
		if (!is_vatek_success(nres))
			_disp_err("stop usb_stream fail : %d", nres);
		vatek_usbstream_close(hustream);
	}

	/*
		setp 4 :
		before quit demo stop and free both device and source
	*/

	if (hchip)
	{
		// reboot chip
		vatek_device_close_reboot(hchip);
	}

	if (hdevlist)
	{
		vatek_device_list_free(hdevlist);
	}

	if (streamsource.hsource)
	{
		streamsource.free(streamsource.hsource);
	}

	printf_app_end();
	cross_os_sleep(10);
	return (int32_t)1;
}

vatek_result source_sync_get_buffer(void *param, uint8_t **pslicebuf)
{
	tsstream_source *ptssource = (tsstream_source *)param;
	vatek_result nres = ptssource->check(ptssource->hsource);
	if (nres > vatek_success)
	{
		*pslicebuf = ptssource->get(ptssource->hsource);
		nres = (vatek_result)1;
	}

	return nres;
}

vatek_result parser_cmd_source(int32_t argc, char **argv, tsstream_source *stream_source, usbstream_param *pustream)
{
	vatek_result nres = vatek_result::vatek_unsupport;

	// 如果参数大于等于 2，第二个参数必须是制式选择。在这里比较字符串来判断选中了哪个制式。
	if (argc >= 2)
	{
		if (strcmp(argv[1], "atsc") == 0)
		{
			modulator_param_reset(modulator_atsc, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "dvbt") == 0)
		{
			modulator_param_reset(modulator_dvb_t, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "isdbt") == 0)
		{
			modulator_param_reset(modulator_isdb_t, &usbcmd.modulator);
			usbcmd.modulator.ifmode = ifmode_iqoffset;
			usbcmd.modulator.iffreq_offset = 143;
		}
		else if (strcmp(argv[1], "j83a") == 0)
		{
			modulator_param_reset(modulator_j83a, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "j83b") == 0)
		{
			modulator_param_reset(modulator_j83b, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "j83c") == 0)
		{
			modulator_param_reset(modulator_j83c, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "dtmb") == 0)
		{
			modulator_param_reset(modulator_dtmb, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "dvbt2") == 0)
		{
			modulator_param_reset(modulator_dvb_t2, &usbcmd.modulator);
		}
		else if (strcmp(argv[1], "test") == 0)
		{
			nres = stream_source_test_get(&usbcmd.modulator, stream_source);
		}
		else
		{
			nres = vatek_unsupport;
		}

		// 如果参数大于等于 4，第 3 个参数必须是视频源的协议，第 4 个参数是视频源的 URL
		if (argc >= 4)
		{
			if (strcmp(argv[2], "file") == 0)
				nres = stream_source_file_get(argv[3], stream_source);
			else if (strcmp(argv[2], "udp") == 0 || strcmp(argv[2], "rtp") == 0)
				nres = stream_source_udp_get(argv[3], stream_source);
			else
				nres = vatek_unsupport;
		}

		// 第 5 个参数用来选择 PCR 是穿透还是需要进行校正
		if (argc == 5)
		{
			if (strcmp(argv[4], "passthrough") == 0)
				pustream->remux = ustream_remux_passthrough;
			else
				pustream->remux = ustream_remux_pcr;
		}
	}

	if (nres == vatek_unsupport || strcmp(argv[1], "--help") == 0 || argc == 1)
	{
		_disp_l("support command below : ");
		_disp_l("	- app_stream test: test stream mode in app_stream.c");
		_disp_l("	- app_stream [modulation] file [*.ts|*.trp] [remux|passthrough]");
		_disp_l("	- app_stream [modulation] udp  [ip address] [remux|passthrough]");
		_disp_l("	- app_stream [modulation] rtp  [ip address] [remux|passthrough]");
	}

	return nres;
}
