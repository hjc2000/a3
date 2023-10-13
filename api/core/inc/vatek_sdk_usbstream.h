#ifndef _VATEK_SDK_USBSTREAM_
#define _VATEK_SDK_USBSTREAM_

#include <service_transform.h>
#include<functional>
using namespace std;

/// <summary>
///		void * 类型
/// </summary>
typedef void *void_vatek_usbstream;

struct usbstream_slice
{
	int32_t packet_len;			/*!< recv packet length (packet numbers)*/
	int32_t packet_pos;			/*!< current buffer pos */
	uint8_t *buf;				/*!< raw buffer */
	uint8_t *ptrbuf;			/*!< buffer pointer */
};

enum usbstream_status
{
	usbstream_err_unknown = -1,		/*!< unknown fail */
	usbstream_status_idle = 0,		/*!< idle */
	usbstream_status_running = 1,	/*!< running (streaming)*/
	usbstream_status_moredata = 2,	/*!< more data (need more data to start) */
	usbstream_status_stopping = 3,	/*!< wait stop */
	usbstream_status_stop = 4,		/*!< stop finish */
};

/// <summary>
///		USB 流模式，有同步、异步两种
/// </summary>
enum usbstream_mode
{
	ustream_mode_async = 0,			/*!< async */
	ustream_mode_sync = 1,			/*!< sync */
};

/// <summary>
///		USB 流混合模式。有重新复用 PCR 和透传两种。
/// </summary>
enum usbstream_remux
{
	ustream_remux_pcr,
	ustream_remux_passthrough,
};

typedef vatek_result(*fpsync_get_buffer)(void *param, uint8_t **slicebuf);

struct usbstream_sync
{
	void *void_param = nullptr;
	fpsync_get_buffer getbuffer = nullptr;
};

enum uasync_status
{
	uasync_status_unsupport = -1,
	uasync_status_idle = 0,
	uasync_status_prepare,
	uasync_status_streaming,
	uasync_status_pause,
};

enum uasync_mode
{
	uasync_mode_normal = 0,
	uasync_mode_cbr,
};

struct usbstream_async
{
	uasync_mode mode;
	uint32_t bitrate;			/* config source bitrate	[0: mean auto]*/
	uint32_t prepare_ms;		/* config prepare time ms	[0: mean auto]*/
};

class usbstream_param
{
public:
	usbstream_param() {}

	usbstream_mode mode;
	usbstream_remux remux;
	pcr_adjust_mode pcradjust;
	//uint32_t freq_khz;
	r2_param r2param;
	modulator_param modulator;
	union
	{
		usbstream_sync sync;
		usbstream_async async;
	};
};

HAL_API vatek_result vatek_usbstream_open(vatek_device *hchip, void_vatek_usbstream *husstream);
HAL_API vatek_result vatek_usbstream_check(void_vatek_usbstream husstream);
HAL_API Pbroadcast_info vatek_usbstream_get_info(void_vatek_usbstream husstream);
HAL_API vatek_result vatek_usbstream_start(void_vatek_usbstream husstream, usbstream_param *puparam);
HAL_API usbstream_status vatek_usbstream_get_status(void_vatek_usbstream husstream, Ptransform_info *ptrinfo);

HAL_API uasync_status vatek_ustream_async_get_status(void_vatek_usbstream hustream);
HAL_API vatek_result vatek_ustream_async_get_buffer(void_vatek_usbstream husstream, usbstream_slice **pslicebuf);
HAL_API vatek_result vatek_ustream_async_commit_buffer(void_vatek_usbstream husstream, usbstream_slice *pslicebuf);

HAL_API vatek_result vatek_usbstream_stop(void_vatek_usbstream husstream);
HAL_API void vatek_usbstream_close(void_vatek_usbstream husstream);

#endif
