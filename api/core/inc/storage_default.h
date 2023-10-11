#include <storage/storage_config.h>
#include <media_audio.h>
#include <media_video.h>

/* default value */

#define M2FLAG_DEF_SD       M2FLAG_QSCALE_NONELINEAR
#define M2FLAG_DEF_HD       M2FLAG_QSCALE_NONELINEAR
#define M2FLAG_DEF_FULLHD   (M2FLAG_QSCALE_NONELINEAR | M2FLAG_EN_QD_CNTL) 

#define M2QD_CNTL_DEFAULT	(QD_INTER_VLC)
#define M2ME_CNTL_DEFAULT	(0)

const static mpeg2_param default_mpeg2_codec_sd =
{
	M2FLAG_DEF_SD,
	mpeg2_profile_main_lmain,
	dc_presision_8,
	search_all,
	qmatrix_en_default,
	M2QD_CNTL_DEFAULT,
	M2ME_CNTL_DEFAULT,
	{0,0},
	{
		{ 8, 16, 19, 22, 26, 27, 29, 34 },
		{ 16, 16, 22, 24, 27, 29, 34, 37 },
		{ 19, 22, 26, 27, 29, 34, 34, 38 },
		{ 22, 22, 26, 27, 29, 34, 37, 40 },
		{ 22, 26, 27, 29, 32, 35, 40, 48 },
		{ 26, 27, 29, 32, 35, 40, 48, 58 },
		{ 26, 27, 29, 34, 38, 46, 56, 69 },
		{ 27, 29, 35, 38, 46, 56, 69, 83 }
	},
	{
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 },
		{ 16, 16, 16, 16, 16, 16, 16, 16 }
	},
};

const static mpeg2_param default_mpeg2_codec_hd =
{
	M2FLAG_DEF_HD,
	mpeg2_profile_main,
	dc_presision_8,
	search_all,
	qmatrix_en_user,
	M2QD_CNTL_DEFAULT,
	M2ME_CNTL_DEFAULT,
	{ 0,0 },
	{
		{ 8,  16, 20, 20, 21, 23, 26, 30 },
		{ 16, 16, 20, 20, 22, 24, 27, 32 },
		{ 20, 20, 21, 22, 24, 27, 31, 36 },
		{ 20, 20, 22, 26, 30, 34, 38, 44 },
		{ 21, 22, 24, 30, 37, 44, 51, 59 },
		{ 23, 24, 27, 34, 44, 56, 68, 81 },
		{ 26, 27, 31, 38, 51, 68, 88, 109 },
		{ 30, 32, 36, 44, 59, 81, 109,144 }
	},
	{
		{ 20, 20, 20, 20, 21, 23, 26, 30 },
		{ 20, 20, 20, 20, 22, 24, 27, 32 },
		{ 20, 20, 21, 22, 24, 27, 31, 36 },
		{ 20, 20, 22, 26, 30, 34, 38, 44 },
		{ 21, 22, 24, 30, 37, 44, 51, 59 },
		{ 23, 24, 27, 34, 44, 56, 68, 81 },
		{ 26, 27, 31, 38, 51, 68, 88, 109 },
		{ 30, 32, 36, 44, 59, 81, 109,144 }
	},
};

const static mpeg2_param default_mpeg2_codec_fullhd =
{
	M2FLAG_DEF_FULLHD,
	mpeg2_profile_main,
	dc_presision_8,
	search_skip,
	qmatrix_en_user,
	M2QD_CNTL_DEFAULT,
	M2ME_CNTL_DEFAULT,
	{ 0,0 },
	{
		{ 8, 17, 21, 23, 33, 37, 49, 49 },
		{ 17, 21, 23, 31, 37, 49, 42, 53 },
		{ 21, 23, 33, 37, 42, 42, 49, 55 },
		{ 23, 29, 35, 42, 42, 49, 53, 44 },
		{ 29, 37, 41, 39, 49, 50, 57, 77 },
		{ 38, 38, 39, 45, 50, 57, 77, 97 },
		{ 38, 43, 45, 49, 55, 77, 97, 109 },
		{ 30, 43, 49, 55, 59, 97, 109,144 }
	},
	{
		{ 16, 17, 21, 23, 33, 37, 49, 49 },
		{ 17, 21, 23, 31, 37, 49, 42, 53 },
		{ 21, 23, 33, 37, 42, 42, 49, 55 },
		{ 23, 29, 35, 42, 42, 49, 53, 44 },
		{ 29, 37, 41, 39, 49, 50, 57, 77 },
		{ 38, 38, 39, 45, 50, 57, 77, 97 },
		{ 38, 43, 45, 49, 55, 77, 97, 109 },
		{ 30, 43, 49, 55, 59, 97, 109,144 }
	},
};

static const h264_param default_h264_codec_param =
{
	h264_profile_high,
	h264_level_4_0,
	H264_FLAG_EN_INTRA_8_8 | H264_FLAG_EN_MV_16_8 | H264_FLAG_EN_MV_8_8,
	0,0,h264_entropy_cabac,search_skip,qmatrix_en_spec,{ 0,0,0,0,0,0,0,0 },
	/* .intra_4x4 = */
	{
		/*.luma       = */{ { 6 ,13,20,28, },{ 13,20,28,32, },{ 20,28,32,37, },{ 28,32,37,42, }, },
		/*.chroma     = */{ { 6 ,13,20,28, },{ 13,20,28,32, },{ 20,28,32,37, },{ 28,32,37,42, }, },
	},
	/*.inter_4x4 = */
	{
		/*.luma       = */{ { 10,14,20,24, },{ 14,20,24,27, },{ 20,24,27,30, },{ 24,27,30,34, }, },
		/*.chroma     = */{ { 10,14,20,24, },{ 14,20,24,27, },{ 20,24,27,30, },{ 24,27,30,34, }, },
	},
	/*.intra_8x8 = */
	{
		/*.luma =*/
		{
			{ 6 ,10,13,16,18,23,25,27, },
			{ 10,11,16,18,23,25,27,29, },
			{ 13,16,18,23,25,27,29,31, },
			{ 16,18,23,25,27,29,31,33, },
			{ 18,23,25,27,29,31,33,36, },
			{ 23,25,27,29,31,33,36,38, },
			{ 25,27,29,31,33,36,38,40, },
			{ 27,29,31,33,36,38,40,42, },
		},
	},
	/*.inter_8x8 = */
	{
		/*.luma = */
		{
			{ 9 ,13,15,17,19,21,22,24, },
			{ 13,13,17,19,21,22,24,25, },
			{ 15,17,19,21,22,24,25,27, },
			{ 17,19,21,22,24,25,27,28, },
			{ 19,21,22,24,25,27,28,30, },
			{ 21,22,24,25,27,28,30,32, },
			{ 22,24,25,27,28,30,32,33, },
			{ 24,25,27,28,30,32,33,35, },
		},
	},
};

static const audio_param default_audio_param =
{
	audio_rc_cbr,audio_quality_middle,0,
};



static const storage_chip_config default_chip_config =
{
	._chip = 
	{
		.config = {
			.tag = CHIP_CFGV1_TAG,
			.functions = CHIP_EN_DAC_EXTR,
			.status_led = 0,
			.usb_vid = 0,
			.usb_pid = 0,
			.usb_str = {0,},
		},
	},
	.srrc =
	{
		{	/* CONFIG_MODSRRC_DVB_T */
			0x00,0x00,0x00,0x00, 0x0f,0xff,0x80,0x02, 0x0f,0xff,0xc0,0x01, 0x00,0x00,0xbf,0xff,
			0x00,0x00,0x7f,0xff, 0x0f,0xff,0xc0,0x01, 0x0f,0xff,0x80,0x02, 0x00,0x00,0x40,0x00,
			0x00,0x00,0xbf,0xfe, 0x00,0x00,0x00,0x00, 0x0f,0xff,0x80,0x02, 0x0f,0xff,0xc0,0x01,
			0x00,0x00,0xbf,0xff, 0x00,0x00,0xbf,0xff, 0x0f,0xff,0xc0,0x01, 0x0f,0xff,0x40,0x02,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0x00,0x03, 0x0f,0xff,0x80,0x01, 0x00,0x00,0xff,0xfd,
			0x00,0x01,0x3f,0xfe, 0x0f,0xff,0x80,0x02, 0x0f,0xfe,0xc0,0x03, 0x00,0x00,0x3f,0xff,
			0x00,0x01,0x7f,0xfc, 0x00,0x00,0xbf,0xff, 0x0f,0xff,0x00,0x04, 0x0f,0xff,0x00,0x02,
			0x00,0x00,0xff,0xfd, 0x00,0x01,0x7f,0xfc, 0x0f,0xff,0xc0,0x01, 0x0f,0xfe,0xc0,0x04,
			0x0f,0xff,0xbf,0xff, 0x0f,0xfc,0x80,0x05, 0x0f,0xff,0x00,0x03, 0x00,0x03,0xbf,0xfb,
			0x00,0x02,0xff,0xfc, 0x0f,0xfd,0x40,0x03, 0x0f,0xfb,0x80,0x06, 0x00,0x01,0x3f,0xfe,
			0x00,0x05,0xff,0xf9, 0x00,0x01,0xbf,0xff, 0x0f,0xfa,0x00,0x08, 0x0f,0xfb,0x40,0x04,
			0x00,0x04,0xbf,0xf8, 0x00,0x07,0xff,0xf8, 0x0f,0xfe,0x80,0x06, 0x0f,0xf6,0x80,0x0b,
			0x00,0x1a,0xbf,0xf4, 0x0f,0xf6,0x00,0x25, 0x0f,0xdb,0x40,0x21, 0x0f,0xf8,0x7f,0xe6,
			0x00,0x28,0xbf,0xce, 0x00,0x1f,0x40,0x04, 0x0f,0xde,0xc0,0x3c, 0x0f,0xc5,0xc0,0x1a,
			0x00,0x0b,0x3f,0xc9, 0x00,0x55,0x3f,0xc6, 0x00,0x22,0x80,0x21, 0x0f,0x94,0x40,0x54,
			0x0f,0x80,0x00,0x05, 0x00,0x7b,0x3f,0xa2, 0x01,0xf9,0xbf,0xc9, 0x02,0xb3,0x00,0x51,
		},
		{	/* CONFIG_MODSRRC_J83A */
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x40,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x40,0x00,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0x80,0x00, 0x00,0x00,0x40,0x00, 0x00,0x00,0x80,0x00,
			0x0f,0xff,0x80,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0xc0,0x00, 0x0f,0xff,0xc0,0x00,
			0x00,0x01,0xbf,0xfd, 0x0f,0xf6,0x40,0x03, 0x00,0x04,0x00,0x02, 0x00,0x0f,0x7f,0xfa,
			0x0f,0xf2,0x00,0x00, 0x0f,0xea,0x80,0x08, 0x00,0x1e,0xff,0xfc, 0x00,0x1b,0xff,0xf7,
			0x0f,0xc6,0xc0,0x0a, 0x0f,0xde,0xc0,0x07, 0x00,0x66,0xff,0xf1, 0x00,0x26,0x3f,0xfd,
			0x0f,0x38,0x40,0x12, 0x0f,0xd7,0x3f,0xfa, 0x02,0x86,0x3f,0xf0, 0x04,0x2a,0x00,0x14,
		},
		{	/* CONFIG_MODSRRC_ATSC */
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x40,0x00, 0x00,0x00,0x40,0x00, 0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x00,
			0x0f,0xff,0xc0,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x40,0x00,
			0x00,0x00,0x80,0x01, 0x0f,0xff,0xc0,0x00, 0x0f,0xff,0x00,0x00, 0x0f,0xff,0x3f,0xff,
			0x0f,0xff,0xbf,0xff, 0x00,0x00,0x80,0x00, 0x00,0x01,0x40,0x01, 0x00,0x01,0x40,0x02,
			0x00,0x00,0x80,0x01, 0x0f,0xff,0x40,0x00, 0x0f,0xfe,0x7f,0xfe, 0x0f,0xfe,0xbf,0xfe,
			0x0f,0xff,0xff,0xfe, 0x00,0x01,0x40,0x00, 0x00,0x02,0x40,0x02, 0x00,0x01,0xc0,0x03,
			0x0f,0xfe,0x80,0x01, 0x0f,0xfa,0x3f,0xf8, 0x0f,0xf8,0xff,0xf4, 0x0f,0xfc,0x7f,0xf7,
			0x00,0x02,0x40,0x00, 0x00,0x07,0xc0,0x0a, 0x00,0x08,0xc0,0x0f, 0x00,0x04,0x40,0x0a,
			0x0f,0xfc,0xff,0xfe, 0x0f,0xf6,0x7f,0xf2, 0x0f,0xf5,0xbf,0xed, 0x0f,0xfb,0x7f,0xf4,
			0x00,0x04,0xc0,0x03, 0x00,0x0c,0x40,0x12, 0x00,0x0c,0xc0,0x17, 0x00,0x05,0x40,0x0e,
			0x0f,0xe8,0xbf,0xe6, 0x0f,0xd4,0xff,0xc3, 0x0f,0xda,0x3f,0xc3, 0x0f,0xf9,0x7f,0xea,
			0x00,0x23,0x40,0x24, 0x00,0x3e,0xc0,0x4d, 0x00,0x36,0x80,0x4a, 0x00,0x06,0xc0,0x18,
			0x0f,0xc4,0x7f,0xcf, 0x0f,0x94,0xff,0x9d, 0x0f,0x9f,0xff,0xa4, 0x0f,0xf9,0x3f,0xe7,
			0x00,0x94,0x40,0x43, 0x01,0x45,0x80,0x80, 0x01,0xd1,0xc0,0x73, 0x02,0x07,0x00,0x1a,
		},
		{	/* CONFIG_MODSRRC_J83BQ64 */
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x40,0x00,
			0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x40,0x00, 0x00,0x00,0x00,0x00,
			0x0f,0xff,0xc0,0x00, 0x00,0x00,0x40,0x00, 0x00,0x00,0x7f,0xff, 0x0f,0xff,0xc0,0x00,
			0x00,0x00,0x00,0x01, 0x00,0x00,0x40,0x00, 0x0f,0xff,0xc0,0x00, 0x0f,0xff,0xc0,0x01,
			0x0f,0xff,0x80,0x01, 0x00,0x00,0xc0,0x00, 0x00,0x00,0x3f,0xff, 0x0f,0xff,0x40,0x00,
			0x00,0x00,0x80,0x01, 0x00,0x00,0xff,0xff, 0x0f,0xff,0x7f,0xff, 0x0f,0xff,0x80,0x02,
			0x00,0x01,0x00,0x00, 0x0f,0xff,0xff,0xfe, 0x0f,0xff,0x00,0x01, 0x00,0x00,0xc0,0x01,
			0x00,0x00,0xbf,0xfe, 0x0f,0xfe,0x80,0x00, 0x00,0x00,0x40,0x02, 0x00,0x01,0xbf,0xff,
			0x00,0x06,0xbf,0xfb, 0x0f,0xfb,0x3f,0xfb, 0x0f,0xfc,0x80,0x08, 0x00,0x0c,0x00,0x01,
			0x0f,0xfb,0xbf,0xf7, 0x0f,0xeb,0xc0,0x04, 0x00,0x14,0x00,0x07, 0x00,0x1d,0x3f,0xf6,
			0x0f,0xd1,0x3f,0xff, 0x0f,0xda,0xc0,0x0f, 0x00,0x5e,0x7f,0xf9, 0x00,0x2c,0x7f,0xf0,
			0x0f,0x3d,0xc0,0x12, 0x0f,0xcf,0x40,0x0b, 0x02,0x84,0x3f,0xe7, 0x04,0x32,0x40,0x00,
		},
		{	/* CONFIG_MODSRRC_DTMB */
			0x0f,0xff,0xc0,0x01, 0x0f,0xff,0xbf,0xff, 0x00,0x00,0x80,0x00, 0x00,0x00,0x40,0x01,
			0x0f,0xff,0x80,0x00, 0x0f,0xff,0xff,0xfe, 0x00,0x00,0x80,0x00, 0x00,0x00,0x40,0x02,
			0x0f,0xff,0x80,0x00, 0x0f,0xff,0xff,0xfe, 0x00,0x00,0x80,0x01, 0x00,0x00,0x00,0x02,
			0x0f,0xff,0x7f,0xff, 0x00,0x00,0x3f,0xfe, 0x00,0x00,0xc0,0x01, 0x0f,0xff,0xc0,0x02,
			0x00,0x00,0x7f,0xfe, 0x00,0x01,0x00,0x01, 0x0f,0xff,0x80,0x02, 0x0f,0xff,0x3f,0xfe,
			0x00,0x00,0xbf,0xfe, 0x00,0x01,0x40,0x02, 0x0f,0xff,0x40,0x02, 0x0f,0xff,0x3f,0xfe,
			0x00,0x01,0x3f,0xff, 0x00,0x01,0x00,0x03, 0x0f,0xfe,0xc0,0x01, 0x0f,0xff,0x3f,0xfd,
			0x00,0x01,0x40,0x00, 0x00,0x01,0x00,0x04, 0x0f,0xfe,0x80,0x00, 0x0f,0xff,0x7f,0xfc,
			0x00,0x01,0x00,0x07, 0x0f,0xfd,0x80,0x02, 0x0f,0xff,0xbf,0xf9, 0x00,0x03,0x3f,0xff,
			0x00,0x00,0x00,0x07, 0x0f,0xfc,0x40,0x00, 0x00,0x00,0xff,0xf8, 0x00,0x04,0x40,0x01,
			0x0f,0xfe,0x80,0x08, 0x0f,0xfb,0x3f,0xfd, 0x00,0x02,0xbf,0xf9, 0x00,0x05,0x80,0x04,
			0x0f,0xfc,0x40,0x07, 0x0f,0xf9,0xff,0xfa, 0x00,0x05,0x7f,0xfa, 0x00,0x07,0x00,0x08,
			0x0f,0xdd,0x7f,0xe5, 0x0f,0xf3,0xbf,0xe1, 0x00,0x2a,0x80,0x23, 0x00,0x0c,0xc0,0x21,
			0x0f,0xcb,0x7f,0xd5, 0x0f,0xf2,0xff,0xdc, 0x00,0x43,0x00,0x35, 0x00,0x0d,0x80,0x27,
			0x0f,0xa7,0x3f,0xbf, 0x0f,0xf2,0x7f,0xd7, 0x00,0x7f,0x40,0x4f, 0x00,0x0d,0xc0,0x2b,
			0x0f,0x28,0xbf,0xa1, 0x0f,0xf2,0x3f,0xd2, 0x02,0x8b,0x40,0x73, 0x04,0x0e,0x00,0x30,
		},
		{	/* CONFIG_MODSRRC_ISDB_T */
			0x00,0x00,0x00,0x01, 0x00,0x00,0x80,0x01, 0x00,0x00,0x40,0x00, 0x0f,0xff,0xff,0xff,
			0x0f,0xff,0xbf,0xff, 0x0f,0xff,0xc0,0x01, 0x00,0x00,0x40,0x01, 0x00,0x00,0x80,0x00,
			0x00,0x00,0x7f,0xff, 0x0f,0xff,0xbf,0xff, 0x0f,0xff,0x40,0x00, 0x0f,0xff,0xc0,0x01,
			0x00,0x00,0x80,0x01, 0x00,0x00,0xc0,0x00, 0x00,0x00,0x7f,0xff, 0x0f,0xff,0x7f,0xff,
			0x0f,0xfe,0xbf,0xfd, 0x0f,0xfe,0x00,0x00, 0x0f,0xff,0xc0,0x04, 0x00,0x02,0x00,0x04,
			0x00,0x02,0x3f,0xff, 0x0f,0xff,0xff,0xfb, 0x0f,0xfd,0xbf,0xfc, 0x0f,0xfe,0x00,0x02,
			0x00,0x00,0xc0,0x06, 0x00,0x03,0x00,0x03, 0x00,0x02,0x3f,0xfd, 0x0f,0xfe,0xff,0xf9,
			0x0f,0xfc,0xbf,0xfd, 0x0f,0xfe,0x40,0x05, 0x00,0x02,0x00,0x07, 0x00,0x03,0xc0,0x02,
			0x00,0x07,0x40,0x05, 0x00,0x03,0x7f,0xf5, 0x0f,0xfb,0x7f,0xef, 0x0f,0xf8,0x3f,0xfd,
			0x0f,0xfd,0x80,0x0f, 0x00,0x06,0x80,0x11, 0x00,0x08,0x80,0x00, 0x00,0x01,0x7f,0xee,
			0x0f,0xf7,0xff,0xee, 0x0f,0xf7,0x00,0x03, 0x00,0x00,0x80,0x16, 0x00,0x0a,0x00,0x11,
			0x00,0x09,0x3f,0xf8, 0x0f,0xfd,0xbf,0xe6, 0x0f,0xf4,0x3f,0xf0, 0x0f,0xf7,0x80,0x0d,
			0x0f,0xe8,0xc0,0x14, 0x00,0x09,0x00,0x38, 0x00,0x24,0x00,0x1f, 0x00,0x18,0xbf,0xe0,
			0x0f,0xed,0xbf,0xc0, 0x0f,0xd0,0x3f,0xe6, 0x0f,0xe6,0x40,0x2e, 0x00,0x22,0x80,0x48,
			0x00,0x44,0xc0,0x12, 0x00,0x1a,0xbf,0xc1, 0x0f,0xbc,0x7f,0xb0, 0x0f,0x8b,0xbf,0xfa,
			0x0f,0xe5,0x00,0x54, 0x00,0xc4,0xc0,0x57, 0x01,0xb4,0x7f,0xf4, 0x02,0x1b,0x3f,0x92,
		},
		{	/* CONFIG_MODSRRC_J83C */
			0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x7f,0xff,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0xc0,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x7f,0xff,
			0x0f,0xff,0xc0,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x7f,0xff, 0x00,0x00,0x00,0x00,
			0x0f,0xff,0xc0,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x7f,0xff, 0x0f,0xff,0xc0,0x00,
			0x00,0x00,0x7f,0xff, 0x0f,0xff,0x80,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0xbf,0xff,
			0x0f,0xff,0xc0,0x00, 0x0f,0xff,0x80,0x01, 0x00,0x00,0x7f,0xff, 0x00,0x00,0x7f,0xff,
			0x0f,0xff,0x80,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0xbf,0xff, 0x00,0x00,0x00,0x00,
			0x0f,0xff,0x80,0x01, 0x00,0x00,0x7f,0xff, 0x00,0x00,0x7f,0xff, 0x0f,0xff,0x80,0x01,
			0x00,0x00,0x3f,0xff, 0x0f,0xfe,0xc0,0x03, 0x00,0x00,0x80,0x00, 0x00,0x01,0x7f,0xfd,
			0x0f,0xff,0x00,0x02, 0x0f,0xff,0x00,0x02, 0x00,0x01,0x7f,0xfd, 0x00,0x00,0xff,0xfe,
			0x0f,0xfe,0x40,0x03, 0x00,0x00,0x00,0x00, 0x00,0x01,0xff,0xfd, 0x0f,0xff,0x40,0x01,
			0x0f,0xfe,0x80,0x03, 0x00,0x01,0xbf,0xfd, 0x00,0x00,0xff,0xfe, 0x0f,0xfd,0xc0,0x04,
			0x0f,0xfc,0x00,0x01, 0x0f,0xf1,0xc0,0x0b, 0x00,0x0b,0xff,0xfa, 0x00,0x13,0x3f,0xf4,
			0x0f,0xe8,0x80,0x0b, 0x0f,0xe8,0x80,0x0a, 0x00,0x28,0xff,0xf0, 0x00,0x1b,0xff,0xfa,
			0x0f,0xbd,0x00,0x13, 0x0f,0xe0,0xff,0xff, 0x00,0x6e,0xff,0xec, 0x00,0x22,0x00,0x0b,
			0x0f,0x33,0x00,0x0f, 0x0f,0xdc,0x7f,0xe8, 0x02,0x87,0xff,0xfc, 0x04,0x24,0x40,0x28,
		},
		{	/* CONFIG_MODSRRC_DVB_T2 */
			0x00,0x00,0x00,0x00, 0x0f,0xff,0x80,0x02, 0x0f,0xff,0xc0,0x01, 0x00,0x00,0xbf,0xff,
			0x00,0x00,0x7f,0xff, 0x0f,0xff,0xc0,0x01, 0x0f,0xff,0x80,0x02, 0x00,0x00,0x40,0x00,
			0x00,0x00,0xbf,0xfe, 0x00,0x00,0x00,0x00, 0x0f,0xff,0x80,0x02, 0x0f,0xff,0xc0,0x01,
			0x00,0x00,0xbf,0xff, 0x00,0x00,0xbf,0xff, 0x0f,0xff,0xc0,0x01, 0x0f,0xff,0x40,0x02,
			0x00,0x00,0x00,0x00, 0x0f,0xff,0x00,0x03, 0x0f,0xff,0x80,0x01, 0x00,0x00,0xff,0xfd,
			0x00,0x01,0x3f,0xfe, 0x0f,0xff,0x80,0x02, 0x0f,0xfe,0xc0,0x03, 0x00,0x00,0x3f,0xff,
			0x00,0x01,0x7f,0xfc, 0x00,0x00,0xbf,0xff, 0x0f,0xff,0x00,0x04, 0x0f,0xff,0x00,0x02,
			0x00,0x00,0xff,0xfd, 0x00,0x01,0x7f,0xfc, 0x0f,0xff,0xc0,0x01, 0x0f,0xfe,0xc0,0x04,
			0x0f,0xff,0xbf,0xff, 0x0f,0xfc,0x80,0x05, 0x0f,0xff,0x00,0x03, 0x00,0x03,0xbf,0xfb,
			0x00,0x02,0xff,0xfc, 0x0f,0xfd,0x40,0x03, 0x0f,0xfb,0x80,0x06, 0x00,0x01,0x3f,0xfe,
			0x00,0x05,0xff,0xf9, 0x00,0x01,0xbf,0xff, 0x0f,0xfa,0x00,0x08, 0x0f,0xfb,0x40,0x04,
			0x00,0x04,0xbf,0xf8, 0x00,0x07,0xff,0xf8, 0x0f,0xfe,0x80,0x06, 0x0f,0xf6,0x80,0x0b,
			0x00,0x1a,0xbf,0xf4, 0x0f,0xf6,0x00,0x25, 0x0f,0xdb,0x40,0x21, 0x0f,0xf8,0x7f,0xe6,
			0x00,0x28,0xbf,0xce, 0x00,0x1f,0x40,0x04, 0x0f,0xde,0xc0,0x3c, 0x0f,0xc5,0xc0,0x1a,
			0x00,0x0b,0x3f,0xc9, 0x00,0x55,0x3f,0xc6, 0x00,0x22,0x80,0x21, 0x0f,0x94,0x40,0x54,
			0x0f,0x80,0x00,0x05, 0x00,0x7b,0x3f,0xa2, 0x01,0xf9,0xbf,0xc9, 0x02,0xb3,0x00,0x51,
		},
		{	/* CONFIG_MODSRRC_J83BQ256 */
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x0f,0xff,0xc0,0x00, 0x0f,0xff,0xc0,0x00, 0x00,0x00,0x7f,0xff, 0x00,0x00,0x00,0x00,
			0x0f,0xff,0xc0,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x7f,0xff, 0x0f,0xff,0xc0,0x00,
			0x0f,0xff,0xc0,0x00, 0x00,0x00,0x7f,0xff, 0x00,0x00,0x40,0x00, 0x0f,0xff,0x80,0x01,
			0x00,0x00,0x00,0x00, 0x00,0x00,0xbf,0xff, 0x0f,0xff,0xc0,0x00, 0x0f,0xff,0x80,0x01,
			0x0f,0xff,0x40,0x01, 0x0f,0xff,0x40,0x01, 0x00,0x01,0x3f,0xfe, 0x00,0x00,0xbf,0xff,
			0x0f,0xfe,0xc0,0x02, 0x0f,0xff,0xc0,0x00, 0x00,0x01,0xbf,0xfe, 0x0f,0xff,0xc0,0x01,
			0x0f,0xfe,0xc0,0x02, 0x00,0x01,0x3f,0xfe, 0x00,0x01,0x3f,0xff, 0x0f,0xfe,0x80,0x03,
			0x0f,0xff,0x80,0x00, 0x00,0x02,0x7f,0xfc, 0x0f,0xff,0x80,0x01, 0x0f,0xfd,0x80,0x04,
			0x0f,0xf8,0xc0,0x06, 0x0f,0xf0,0xc0,0x0a, 0x00,0x0f,0xff,0xf6, 0x00,0x13,0x7f,0xf7,
			0x0f,0xe4,0x80,0x0e, 0x0f,0xe9,0x00,0x05, 0x00,0x2c,0xbf,0xef, 0x00,0x1a,0x80,0x00,
			0x0f,0xb9,0xc0,0x12, 0x0f,0xe2,0xbf,0xf8, 0x00,0x71,0x7f,0xf1, 0x00,0x1f,0xc0,0x13,
			0x0f,0x31,0x40,0x07, 0x0f,0xdf,0x3f,0xe1, 0x02,0x88,0x40,0x07, 0x04,0x21,0x80,0x2e,
		},
	},
	.modulation = 
	{
		{	/* CONFIG_CHIPIDX_B2 */
			.dvb_t = 
			{
				.scl_0 = 0xFFFFFFFF,.scl_1 = 0xFFFFFFFF,.scl_2 = 0xFFFFFFFF,
				.fft_scale = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,},
				.gain = {0x00005555,0x00005555,0x00005555,},
			},
			.j83a = {.gain = {0x00002020,0x00002626,0x00002424,0x00002929,0x00002727},},
			.atsc = {5,0x00004040,},
			.j83b = {0x00005050,0x00005050,},
			.dtmb = {
				.pn_mag = {0x3F1,0x3F1},
				.ofdm_mag = 
				{	/*  qpsk	 qam16	 qam64	 q4-nr	 qam32 */
					{0xfd5	,0x713	,0x3f1	,0xfd5	,0x50A},
					{0x1255	,0x833	,0x400	,0x1255	,0x5CC},
				},
				.tps_scale = {1,3,7,1,3},
				.fft_scale = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
				.gain = 
				{
					{0x00004040, 0x00003d3d},
					{0x00004040	,0x00003e3e	,0x00004040	,0x00004040	,0x00002020},
				},
			},
			.isdb_t = 
			{
				.scl_0		= 0x5a8400,.scl_1 = 0x13c144,.scl_2 = 0x555,
				.fft_scale	= {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,},
				.gain		= {0x00005555,0x00005555,0x00005555,},
			},
			.dvb_t2 = 
			{
				.gain = {0x00005959,0x00005454,0x00005858,0x00005353,0x00004e4e,},
				.fftscale = {0x006c0300,0x006c0300,0x006c0400,0x006c0600,0x006c0900,},
			},
		},
		{	/* CONFIG_CHIPIDX_B2_PLUS */
			.dvb_t =
			{
				.scl_0 = 0x2da200,.scl_1 = 0x09e0a2,.scl_2 = 0x2ab,
				.fft_scale = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,},
				.gain = {0x00005555,0x00005555,0x00005555,},
			},
			.j83a = {.gain = {0x00002020,0x00002626,0x00002424,0x00002929,0x00002727},},
			.atsc = {5,0x00004040,},
			.j83b = {0x00005050,0x00005050,},
			.dtmb = {
				.pn_mag = {0x3F1,0x3F1},
				.ofdm_mag =
				{	/*  qpsk	 qam16	 qam64	 q4-nr	 qam32 */
					{0xfd5	,0x713	,0x3f1	,0xfd5	,0x50A},
					{0x1255	,0x833	,0x400	,0x1255	,0x5CC},
				},
				.tps_scale = {1,3,7,1,3},
				.fft_scale = {0xFFFFFFFF ,0xFFFFFFFF,0xFFFFFFFF	,0xFFFFFFFF	,0xFFFFFFFF},
				.gain =
				{
					{0x00004040, 0x00003d3d},
					{0x00004040	,0x00003e3e	,0x00004040	,0x00004040	,0x00002020},
				},
			},
			.isdb_t =
			{
				.scl_0 = 0x5a8400,.scl_1 = 0x13c144,.scl_2 = 0x555,
				.fft_scale = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,},
				.gain = {0x3035555,0x303aaaa,0x3037676,},
			},
			.dvb_t2 =
			{
				.gain = {0x00005959,0x00005454,0x00005858,0x00005353,0x00004e4e,},
				.fftscale = {0x006c0300,0x006c0300,0x006c0400,0x006c0600,0x006c0900,},
			},
		},
		{	/* CONFIG_CHIPIDX_B3 */
			.dvb_t =
			{
				.scl_0 = 0x2da200,.scl_1 = 0x09e0a2,.scl_2 = 0x2ab,
				.fft_scale = {0x390,0x780,0x780,},
				.gain = {0x00005555,0x00005555,0x00005555,},
			},
			.j83a = {.gain = {0x00002020,0x00002626,0x00002424,0x00002929,0x00002727},},
			.atsc = {5,0x00005050,},
			.j83b = {0x00005050,0x00005050,},
			.dtmb = {
				.pn_mag = {0x3F1,0x3F1},
				.ofdm_mag =
				{	/*  qpsk	 qam16	 qam64	 q4-nr	 qam32 */
					{0x1255	,0x833	,0x400	,0x1255	,0x5CC},
					{0xfd5	,0x713	,0x3f1	,0xfd5	,0x50A},
				},
				.tps_scale = {1,3,7,1,3},
				.fft_scale = {0xFFFFFFFF,0xFFFFFFFF	,0xFFFFFFFF	,0xFFFFFFFF	,0xFFFFFFFF},
				.gain = 
				{
					{0x00004040, 0x00003d3d},
					{0x00004040	,0x00003e3e	,0x00004040 ,0x00004040	,0x00002020},
				},
			},
			.isdb_t =
			{
				.scl_0 = 0x3aa299,.scl_1 = 0x0cc0d2,.scl_2 = 0x377,
				.fft_scale = {0x630,0xC80,0x8F0,},
				.gain = {0x02025555,0x02025555,0x03035555,},
			},
			.dvb_t2 =
			{
				.gain = {0x00005959,0x00005454,0x00005858,0x00005353,0x00004e4e,},
				.fftscale = {0x006c0300,0x006c0300,0x006c0400,0x006c0600,0x006c0900,},
			},
		},
		{	/* CONFIG_CHIPIDX_A3 */
			.dvb_t =
			{
				.scl_0 = 0x2da200,.scl_1 = 0x09e0a2,.scl_2 = 0x2ab,
				.fft_scale = {0x390,0x780,0x720,},
				.gain = {0x00005555,0x00005555,0x00005555,},
			},
			.j83a = {.gain = {0x00002020,0x00002626,0x00002424,0x00002929,0x00002727},},
			.atsc = {5,0x00005050,},
			.j83b = {0x00005050,0x00005050,},
			.dtmb = {
				.pn_mag = {0x3F1,0x3F1},
				.ofdm_mag =
				{	/*  qpsk	 qam16	 qam64	 q4-nr	 qam32 */
					{0x1255	,0x833	,0x400	,0x1255	,0x5cc},
					{0xfd5	,0x713	,0x3f1	,0xfd5	,0x50a},
				},
				.tps_scale = {1,3,7,1,3},
				.fft_scale = {0x240	,0x240	,0x240	,0x240	,0x240},
				.gain = 
				{
					{0x00004040, 0x00003d3d},
					{0x00004040	,0x00003e3e	,0x00004040	,0x00004040	,0x00002020},
				},
			},
			.isdb_t =
			{
				.scl_0 = 0x3aa299,.scl_1 = 0xcc0d2,.scl_2 = 0x377,
				.fft_scale = {0x310,0x630,0x470,},
				.gain = {0x02025555,0x02025555,0x02025555,},
			},
			.dvb_t2 =
			{
				.gain = {0x00005959,0x00005454,0x00005858,0x00005353,0x00004e4e,},
				.fftscale = {0x006c0300,0x006c0300,0x006c0400,0x006c0600,0x006c0900,},
			},
		},
	},
};
