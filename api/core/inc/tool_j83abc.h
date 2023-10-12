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

#ifndef _TOOL_J83ABC_
#define _TOOL_J83ABC_

#include <tool_ofdm.h>

#define J83A_SYMBOL_MIN		5000
#define J83A_SYMBOL_MAX		8000

#define is_j83a_valid_sb(sb)	(sb >= J83A_SYMBOL_MIN && sb <= J83A_SYMBOL_MAX)

#define J83B_Q64_SYMBOL		5056941
#define J83B_Q256_SYMBOL	5360537

#define J83C_SYMBOL_RATE	5274

#ifdef __cplusplus
extern "C" {
#endif

	HAL_API vatek_result tool_j83_j83frame_reset(modulator_param * pmod, Pofdm_frame pframe);
	HAL_API uint32_t tool_j83_get_bitrate(modulator_param * pmod);
	HAL_API vatek_result tool_j83_check_param(modulator_param * pmod);

#ifdef __cplusplus
}
#endif

#endif
