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

#include "stream_handle.h"
#include <tool_crc32.h>

#define TEST_PACKET_USB_PAT		1
#define TEST_PACKET_PCR_PID		0x100

struct handle_test
{
	cstream_handler handle;
	mux_time_tick time;
	uint32_t slice_ns;
	uint8_t buffer[TSSLICE_BUFFER_LEN];
	uint32_t packetnums;
	uint32_t tick;
};

extern vatek_result cstream_test_start(hcstream hstream);
extern vatek_result cstream_test_get_slice(hcstream hstream, uint8_t **pslice);
extern uint32_t cstream_test_get_bitrate(hcstream hstream);
extern void cstream_test_stop(hcstream hstream);
extern void cstream_test_close(hcstream hstream);

vatek_result cross_stream_test_get(uint32_t bitrate, cstream_handler * *pcstream)
{
	handle_test *ptest = (handle_test *)malloc(sizeof(handle_test));
	vatek_result nres = vatek_memfail;

	if (ptest)
	{
		memset(ptest, 0, sizeof(handle_test));
		ptest->slice_ns = (1000000000 / (bitrate / (TS_PACKET_LEN * 8))) * TSSLICE_PACKET_NUM;
		ptest->handle.hstream = ptest;
		ptest->handle.start = cstream_test_start;
		ptest->handle.stop = cstream_test_stop;
		ptest->handle.get_bitrate = cstream_test_get_bitrate;
		ptest->handle.get_slice = cstream_test_get_slice;
		ptest->handle.close = cstream_test_close;
		*pcstream = &ptest->handle;
		nres = vatek_success;
	}

	return nres;
}

vatek_result cstream_test_start(hcstream hstream)
{
	handle_test *ptest = (handle_test *)hstream;
	ptest->time.ms = 0;
	ptest->time.ns = 0;
	ptest->tick = cross_os_get_tick_ms();
	return vatek_success;
}

uint32_t cstream_test_get_bitrate(hcstream hstream)
{
	handle_test *ptest = (handle_test *)hstream;
	int32_t eclipse = cross_os_get_tick_ms() - ptest->tick;
	if (eclipse)
	{
		uint32_t bitrate = (ptest->packetnums * TS_PACKET_LEN * 8 * 1000) / eclipse;
		ptest->tick = cross_os_get_tick_ms();
		ptest->packetnums = 0;
		return bitrate;
	}

	return 0;
}

vatek_result cstream_test_get_slice(hcstream hstream, uint8_t **pslice)
{
	handle_test *ptest = (handle_test *)hstream;
	int32_t nums = 0;
	uint8_t *ptr = &ptest->buffer[0];

	while (nums < TSSLICE_PACKET_NUM)
	{
		if (nums == 0)memcpy(ptr, tspacket_get_pcr(&ptest->time), TS_PACKET_LEN);
		else memcpy(ptr, tspacket_get_suffing(), TS_PACKET_LEN);
		ptr += TS_PACKET_LEN;
		nums++;
	}

	ptest->time.ns += ptest->slice_ns;
	if (ptest->time.ns >= 1000000)
	{
		ptest->time.ms += ptest->time.ns / 1000000;
		ptest->time.ns = ptest->time.ns % 1000000;
		ptest->packetnums += TSSLICE_PACKET_NUM;
	}

	*pslice = &ptest->buffer[0];
	return (vatek_result)1;
}

void cstream_test_stop(hcstream hstream)
{

}

void cstream_test_close(hcstream hstream)
{
	handle_test *ptest = (handle_test *)hstream;
	free(hstream);
}

uint8_t *tspacket_get_pcr(Pmux_time_tick ptime)
{
	static uint8_t fake_pcr_packet[188] =
	{
		0x47,0x40 | (uint8_t)(TEST_PACKET_PCR_PID >> 8),(uint8_t)TEST_PACKET_PCR_PID,0x20,
		0xB7,0x10,0x00,0x00,0x00,0x00,0x00,0x00,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	};

	uint64_t t90k = (uint64_t)ptime->ms * 90;
	uint32_t t27m = ptime->ns / 27;
	uint32_t hi90k = 0;

	t90k += (t27m / 300);
	t27m = t27m % 300;
	hi90k = (uint32_t)(t90k >> 1);

	fake_pcr_packet[6] = hi90k >> 24;
	fake_pcr_packet[7] = hi90k >> 16;
	fake_pcr_packet[8] = hi90k >> 8;
	fake_pcr_packet[9] = hi90k;

	hi90k = t90k & 0x01;
	fake_pcr_packet[10] = 0x7E | (hi90k << 7) | ((t27m >> 8) & 1);
	fake_pcr_packet[11] = t27m;
	return &fake_pcr_packet[0];
}

uint8_t *tspacket_get_suffing(void)
{
	static uint8_t fake_suffing_packet[2][188] =
	{

#if TEST_PACKET_USB_PAT
		{
			0x47 ,0x40 ,0x00 ,0x10 ,0x00 ,0x00 ,0x80 ,0x0D ,0x01 ,0x11 ,0xDF ,0x00 ,0x00 ,0x01 ,0x03 ,0xE3,
			0x00 ,0x19 ,0x3F ,0xB7 ,0xCD ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
		},
		{
			0x47 ,0x43 ,0x00 ,0x10 ,0x00 ,0x02 ,0xB0 ,0x5A ,0x01 ,0x03 ,0xC3 ,0x00 ,0x00 ,0xE3 ,0x01 ,0xF0,
			0x00 ,0x02 ,0xE3 ,0x01 ,0xF0 ,0x05 ,0x02 ,0x03 ,0x9A ,0x48 ,0x5F ,0x03 ,0xE3 ,0x11 ,0xF0 ,0x09,
			0x0A ,0x04 ,0x63 ,0x7A ,0x65 ,0x00 ,0x52 ,0x01 ,0x02 ,0x03 ,0xE3 ,0x13 ,0xF0 ,0x11 ,0x0A ,0x04,
			0x63 ,0x7A ,0x65 ,0x03 ,0x52 ,0x01 ,0x07 ,0x7F ,0x06 ,0x06 ,0x05 ,0x63 ,0x7A ,0x65 ,0x03 ,0x06,
			0xE3 ,0x21 ,0xF0 ,0x1A ,0x45 ,0x11 ,0x01 ,0x0F ,0xE7 ,0xE8 ,0xE9 ,0xEA ,0xEC ,0xED ,0xEE ,0xEF,
			0xC7 ,0xC8 ,0xC9 ,0xCA ,0xCC ,0xCD ,0xCE ,0x56 ,0x05 ,0x63 ,0x7A ,0x65 ,0x09 ,0x00 ,0x58 ,0x5E,
			0xC6 ,0xF0 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF, 0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
			0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
		},
#else
		{
			0x47,0x40,0x1F,0x10,0x00,0x72,0xB0,0xB0,0xB4,0x08,0xC1,0x00,0x00,0xE3,0x00,0xF0,
			0x1a,0x86,0x4d,0xb2,0x1e,0x47,0x50,0x05,0x26,0x08,0xed,0xb8,0x22,0xc9,0xf0,0x0f,
			0x00,0x02,0xE1,0x00,0xF0,0x06,0x52,0x01,0x00,0xC8,0x01,0x47,0x0F,0xE2,0x00,0xF0,
			0x03,0x52,0x01,0x10,0xA6,0xFE,0xB9,0xBC,0xFF,0xFF,0xFF,0x6b,0x9b,0x03,0x15,0xd6,
			0x6b,0x93,0xdd,0xdb,0x6f,0x52,0xc0,0x6c,0x62,0x11,0xe6,0xb5,0x66,0xd0,0xfb,0x02,
			0x3f,0x9b,0x76,0x2c,0x3b,0x5a,0x6b,0x9b,0x03,0x15,0xd6,0x26,0x07,0xd4,0xcb,0x91,
			0x00,0x02,0xE1,0x00,0xF0,0x06,0x52,0x01,0x00,0xC8,0x01,0x47,0x0F,0xE2,0x00,0xF0,
			0x2d,0x15,0xeb,0xe3,0x29,0xd4,0xf6,0x54,0xc5,0xa9,0x26,0x79,0xc1,0x68,0x3b,0xce,
			0x00,0x02,0xE1,0x00,0xF0,0x06,0x52,0x01,0x00,0xC8,0x01,0x47,'W' ,'U' ,0x00,0xF0,
			0xea,0x23,0xf0,0xaf,0xee,0xe2,0xed,0x18,0xf0,0xa5,0xbd,0x1d,0xf4,0x64,0xa0,0xaa,
			0x93,0x3e,'C' ,'H' ,'E' ,'N' ,0xad,0x0c,0xaf,'H' ,'S' ,'U' ,'I' ,0x71,0x0d,0x06,
			0xa6,0x32,0x2b,0xdf,0xa2,0xf3,0x36,0x68,0xFF,0xFF,0xFF,0xFF,
		},
		{
			0x47,0x40,0x1F,0x10,0x00,0x72,0xB0,0xB0,0xF4,0x08,0xC1,0x00,0x00,0xE3,0x00,0xF0,
			0x1a,0x86,0x4d,0xb2,0x1e,0xF7,0x50,0xF5,0xF6,0x08,0xed,0xb8,0x22,0xc9,0xf0,0x0f,
			0x00,0x02,0xE1,'V' ,'A' ,'T' ,'E' ,'K' ,'-' ,'S' ,'D' ,'K' ,0x00,0xE2,0x00,0xF0,
			0x03,0x52,0x01,0x10,0xA6,0xFE,0xB9,0xBC,0xFF,0xFF,0xFF,0x6b,0x9b,0x03,0x15,0xd6,
			0x6b,0x93,0xdd,0xdb,0xFf,0x52,0xF0,0x6c,0x12,0x41,0x46,0xb5,0x66,0xd0,0xfb,0x02,
			0x3f,0x9b,0x76,0x2c,0x3b,0x5a,0x6b,0x9b,0x03,0x15,0xd6,0x26,0x07,0xd4,0xcb,0x91,
			0x00,0x02,0xE1,0x00,0xF0,0x06,0x52,0x01,0x00,0xC8,0x01,0x47,0x0F,0xE2,0x00,0xF0,
			0x2d,0x15,0xeb,0xe3,0x29,0xd4,0xf6,0x54,0xc5,0xa9,0x26,0x79,0xc1,0x68,0x3b,0xce,
			0x00,0x02,0xE1,0x00,0xF0,0x06,0x52,0x01,0x00,0xC8,0x01,0x47,0x0F,0xE2,0x00,0xF0,
			0xea,0x23,0xf0,0xaf,0xee,0xe2,0xed,0x18,0xf0,0xa5,0xbd,0x1d,0xf4,0x64,0xa0,0xaa,
			0x93,0x3e,0xb0,0xbb,0x97,0xff,0xad,0x0c,0xaf,0xb0,0x10,0xb1,0xab,0x71,0x0d,0x06,
			0xa6,0x32,0x2b,0xdf,0xa2,0xf3,0x36,0x68,0xFF,0xFF,0xFF,0xFF,
		},
#endif
	};

	static uint32_t tscont = 0;
	static int32_t packetidx = 0;

	#if !TEST_PACKET_USB_PAT
	uint32_t crc = tool_crc32(&fake_suffing_packet[0][4], 180);
	fake_suffing_packet[0][188 - 4] = (uint8_t)(crc >> 24);
	fake_suffing_packet[0][188 - 3] = (uint8_t)(crc >> 16);
	fake_suffing_packet[0][188 - 2] = (uint8_t)(crc >> 8);
	fake_suffing_packet[0][188 - 1] = (uint8_t)(crc);

	crc = tool_crc32(&fake_suffing_packet[1][4], 180);
	fake_suffing_packet[1][188 - 4] = (uint8_t)(crc >> 24);
	fake_suffing_packet[1][188 - 3] = (uint8_t)(crc >> 16);
	fake_suffing_packet[1][188 - 2] = (uint8_t)(crc >> 8);
	fake_suffing_packet[1][188 - 1] = (uint8_t)(crc);
	#endif


	uint8_t *packet = &fake_suffing_packet[packetidx][0];
	packetidx = (uint32_t)(!packetidx);
	packet[3] = (packet[3] & 0xF0) | (tscont++ & 0xF);
	return &packet[0];
}

uint8_t *tspacket_get_null(void)
{
	static uint8_t fake_null_packet[188] =
	{
		0x47,0x1F,0xFF,0x20,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	};

	return &fake_null_packet[0];
}
