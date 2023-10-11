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

#ifndef _VATEK_SDK_DEVICE_
#define _VATEK_SDK_DEVICE_

#include <vatek_base.h>
#include <chip_define.h>
#include <output_modulator.h>
#include <calibration_define.h>
#include <device_usb.h>

/// <summary>
///		查找设备
/// </summary>
/// <param name="bus">
///		可以传入以下宏定义
///		#define DEVICE_BUS_UNKNOWN		0x00000000
///		#define DEVICE_BUS_USB			0x00000001
///		#define DEVICE_BUS_BRIDGE		0x00000002
///		#define DEVICE_BUS_ALL			0x00000007
/// </param>
/// <param name="service"></param>
/// <param name="hdevices"></param>
/// <returns></returns>
HAL_API vatek_result vatek_device_list_enum(uint32_t bus, hal_service_mode service, vatek_device_list **hdevices);
HAL_API vatek_result vatek_device_list_enum_by_usbid(uint16_t vid, uint16_t pid, vatek_device_list **hdevices);
HAL_API uint32_t vatek_device_list_get_bus(vatek_device_list *hdevices, int32_t idx);
HAL_API const char *vatek_device_list_get_name(vatek_device_list *hdevices, int32_t idx);
HAL_API hal_service_mode vatek_device_list_get_service(vatek_device_list *hdevices, int32_t idx);
HAL_API void vatek_device_list_free(vatek_device_list *hdevices);
HAL_API vatek_result vatek_device_open(vatek_device_list *hdevices, int32_t idx, void_vatek_chip *hchip);

HAL_API vatek_result vatek_device_start_sine(void_vatek_chip hchip, uint32_t freqkhz);
HAL_API vatek_result vatek_device_start_test(void_vatek_chip hchip, Pmodulator_param pmod, uint32_t freqkhz);
HAL_API vatek_result vatek_device_polling(void_vatek_chip hchip);
HAL_API void vatek_device_stop(void_vatek_chip hchip);

HAL_API uint32_t vatek_device_get_bus(void_vatek_chip hchip);
HAL_API chip_info * vatek_device_get_info(void_vatek_chip hchip);
HAL_API const char *vatek_device_get_name(void_vatek_chip hchip);
HAL_API vatek_result vatek_device_close(void_vatek_chip hchip);

HAL_API vatek_result vatek_device_close_reboot(void_vatek_chip hchip);

HAL_API vatek_result vatek_device_calibration_load(void_vatek_chip hchip, Pcalibration_param pcalibration);
HAL_API vatek_result vatek_device_calibration_apply(void_vatek_chip hchip, Pcalibration_param pcalibration);
HAL_API vatek_result vatek_device_calibration_save(void_vatek_chip hchip, Pcalibration_param pcalibration);
HAL_API vatek_result vatek_device_r2_apply(void_vatek_chip hchip, int r2_power);

/* used with transform service for usb stream */
HAL_API vatek_result vatek_device_stream_start(void_vatek_chip hchip, Pmodulator_param pmod, uint32_t stream_mode);
HAL_API vatek_result vatek_device_stream_write(void_vatek_chip hchip, uint8_t *pbuf, int32_t size);
HAL_API vatek_result vatek_device_stream_stop(void_vatek_chip hchip);

/* usb device bulk operations */
HAL_API vatek_result vatek_device_usbbulk_send(void_vatek_chip hchip, Pusbbulk_command pcmd, Pusbbulk_result presult, uint8_t *pbuf, int32_t len);
HAL_API vatek_result vatek_device_usbbulk_get_result(void_vatek_chip hchip, Pusbbulk_result presult);
HAL_API vatek_result vatek_device_usbbulk_write(void_vatek_chip hchip, uint8_t *pbuf, int32_t len);
HAL_API vatek_result vatek_device_usbbulk_read(void_vatek_chip hchip, uint8_t *pbuf, int32_t len);
HAL_API vatek_result vatek_device_usbbulk_get_size(void_vatek_chip hchip);

#endif
