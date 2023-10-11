/* 在本文件中进行函数声明，函数的定义有针对不同操作系统的不同版本。选择这些版本是在 CMakeLists.txt 中。
* 使用不同编译器会编译出不同的版本。
*/

#ifndef _CROSS_OS_API_
#define _CROSS_OS_API_

#include <core/vatek_base.h>

#define TIME_SECOND_TO_NS		1000000000
#define TIME_SECOND_TO_US		1000000

#ifdef __cplusplus
extern "C" {
	#endif

	/// <summary>
	///		尝试从键盘缓冲区中获取一个字符。
	///		* 此函数不会等待用户输入，所以不会阻塞，仅仅检查键盘缓冲区。如果键盘缓冲区中有字符，
	///		  就读取并返回此字符，如果键盘缓冲区中没有字符，则返回 -1。
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	HAL_API int32_t try_getchar(void);
	HAL_API void cross_os_printf(const char *fmt, ...);
	HAL_API void cross_os_error(const char *fmt, ...);

	/* cross_os_filesystem_api */

	HAL_API void cross_os_get_current_path(char *path, int32_t buflen);
	HAL_API void cross_os_append_path(char *path, const char *append);

	typedef void *void_cross_ffind;
	#define FF_TYPE_FOLDER	0x00000001

	struct cross_ffind
	{
		uint32_t ff_type;
		char *filename;
		char *fullpath;
	};

	HAL_API vatek_result cross_os_findfile_first(void_cross_ffind *hffind, const char *path, cross_ffind **pfind);
	HAL_API vatek_result cross_os_findfile_next(void_cross_ffind hffind, cross_ffind **pfind);
	HAL_API void cross_os_findfile_close(void_cross_ffind hffind);

	/// <summary>
	///		睡眠指定的毫秒数
	/// </summary>
	/// <param name="ms"></param>
	/// <returns></returns>
	HAL_API void cross_os_sleep(int32_t ms);

	/// <summary>
	///		让出 CPU
	/// </summary>
	/// <returns></returns>
	HAL_API void cross_os_yield();

	/// <summary>
	///		获取 UTC 的毫秒数
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	HAL_API uint32_t cross_os_get_tick_ms(void);

	/// <summary>
	///		睡眠指定的微秒数
	/// </summary>
	/// <param name="us"></param>
	/// <returns></returns>
	HAL_API void cross_os_usleep(uint32_t us);
	HAL_API void cross_os_wait_unit(timespec *target);
	HAL_API void cross_os_time_plus_ms(timespec *tp, int32_t ms);
	HAL_API void cross_os_time_plus_time(timespec *tp, timespec *tappend);

	/// <summary>
	///		获取当前 UTC 时间
	/// </summary>
	/// <param name="tp"></param>
	/// <returns></returns>
	HAL_API vatek_result cross_os_get_time(timespec *tp);

	/// <summary>
	///		以微秒数获取当前 UTC
	/// </summary>
	/// <returns></returns>
	HAL_API uint64_t cross_os_get_time_us();
	HAL_API uint64_t cross_os_time_to_us(timespec *tp);

	HAL_API vatek_result cross_os_time_eclipse(timespec *st, timespec *eclipse);

	/* cross_os_thread_api */
	typedef void *void_cross_thread;

	struct cross_thread_param
	{
		void *userparam;
		vatek_result result;
	};

	typedef cross_thread_param *Pcross_thread_param;

	typedef void (*fpcross_thread_function)(Pcross_thread_param param);

	HAL_API void_cross_thread cross_os_create_thread(fpcross_thread_function fpfun, void *userparam);
	HAL_API vatek_result cross_os_join_thread(void_cross_thread hthread);
	HAL_API vatek_result cross_os_free_thread(void_cross_thread hthread);
	HAL_API vatek_result cross_os_get_thread_result(void_cross_thread hthread);

	/* cross_os_mutex_api */
	typedef void *void_cross_mutex;

	HAL_API vatek_result cross_os_create_mutex_name(const char *tag, void_cross_mutex *hmutex);
	HAL_API vatek_result cross_os_open_mutex_name(const char *tag, void_cross_mutex *hmuxtex);
	HAL_API vatek_result cross_os_create_mutex(void_cross_mutex *hmutex);
	HAL_API void cross_os_lock_mutex(void_cross_mutex hmutex);
	HAL_API vatek_result cross_os_lock_mutex_timeout(void_cross_mutex hmutex, uint32_t ms);
	HAL_API vatek_result cross_os_trylock_mutex(void_cross_mutex hmutex);
	HAL_API void cross_os_release_mutex(void_cross_mutex hmutex);
	HAL_API vatek_result cross_os_free_mutex(void_cross_mutex hmutex);


	/* cross_os_event_api */
	typedef void *hcross_event;

	HAL_API vatek_result cross_os_create_event(const char *tag, hcross_event *hevent);
	HAL_API vatek_result cross_os_open_event(const char *tag, hcross_event *hevent);

	HAL_API vatek_result cross_os_wait_event_timeout(hcross_event hevent, int32_t ms);
	HAL_API vatek_result cross_os_wait_event(hcross_event hevent);
	HAL_API vatek_result cross_os_raise_event(hcross_event hevent);

	HAL_API void cross_os_free_event(hcross_event hevent);

	/* cross_os_share_memory_api */

	#define CROSS_OS_SMEM_PAGE_SIZE			4096 

	typedef void *hcross_smem;

	HAL_API vatek_result cross_os_create_smem(const char *tag, hcross_smem *hsmem, int32_t size);
	HAL_API vatek_result cross_os_open_smem(const char *tag, hcross_smem *hsmem, int32_t size);

	HAL_API uint8_t *cross_os_get_smem(hcross_smem hsmem);
	HAL_API vatek_result cross_os_close_smem(hcross_smem hsmem);

	/* cross_os_dynamic_library_api */

	typedef void *void_cross_dll;

	HAL_API void_cross_dll cross_os_dll_load(const char *dllfile);
	HAL_API void *cross_os_dll_get_function(void_cross_dll hdll, const char *name);
	HAL_API vatek_result cross_os_dll_free(void_cross_dll hdll);
	HAL_API vatek_result cross_os_dll_valid(const char *name);

	/* cross_os_process_api */

	typedef void *void_cross_process;
	typedef void(*fpprocess_parser)(void *param, uint8_t *ptr, int32_t len);

	struct cross_proccess_param
	{
		char *path;
		char *command;
		void *param;
		fpprocess_parser parser;
	};

	typedef cross_proccess_param *Pcross_proccess_param;

	HAL_API void_cross_process cross_os_create_process(Pcross_proccess_param pprocess);
	HAL_API vatek_result cross_os_check_process(void_cross_process hprocess);
	HAL_API void cross_os_free_process(void_cross_process hprocess);

	/* cross_os_socket_api */

	typedef void *hcross_socket;

	enum socket_mode
	{
		socket_client,
		socket_service,
	};

	enum socket_protocol
	{
		protocol_unknown = -1,
		protocol_rtp = 0,
		protocol_udp = 1,
		protocol_tcp = 2,
	};

	struct socket_param
	{
		socket_mode mode;
		const char *url;
		int32_t buffer_len;
	};

	/*
		supported :
			rtp://xxx.xxx.xxx.xxx:pppp
			udp://xxx.xxx.xxx.xxx:pppp
			//xxx.xxx.xxx.xxx:PPPP			[_tcp]
	*/

	HAL_API vatek_result cross_os_create_socket(socket_param *param, hcross_socket *hsocket);
	HAL_API socket_protocol cross_os_get_protocol_socket(hcross_socket hsocket);
	HAL_API vatek_result cross_os_connect_socket(hcross_socket hsocket);
	HAL_API vatek_result cross_os_recv_socket(hcross_socket hsocket, uint8_t *pbuf, int32_t len, int32_t timeout);

	/// <summary>
	///		用套接字将数据发送出去。只能用于 TCP 协议的套接字，内部会判断是不是 protocol_tcp。
	/// </summary>
	/// <param name="hsocket"></param>
	/// <param name="pbuf"></param>
	/// <param name="len"></param>
	/// <param name="timeout"></param>
	/// <returns>发送完成返回 0，发生错误返回 -1</returns>
	HAL_API vatek_result cross_os_send_socket(hcross_socket hsocket, uint8_t *pbuf, int32_t len, int32_t timeout);
	HAL_API vatek_result cross_os_disconnect_socket(hcross_socket hsocket);
	HAL_API void cross_os_free_socket(hcross_socket hsocket);

	#ifdef _MSC_VER
	#define _cos_log(format,...) cross_os_printf("[%s:%d] " format "\r\n",__func__, __LINE__, ##__VA_ARGS__)
	#define _cos_err(format,...) cross_os_error("[%s:%d] error - " format "\r\n",__func__, __LINE__,##__VA_ARGS__)
	#else
	#define _cos_log(format, args...) cross_os_printf("[%s:%d] " format "\r\n",__func__, __LINE__, ##args)
	#define _cos_err(format, args...) cross_os_error("[%s:%d] error - " format "\r\n",__func__, __LINE__,##args)
	#endif

	#ifdef __cplusplus
}
#endif

#endif
