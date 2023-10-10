#ifndef _CROSS_BRIDGE_
#define _CROSS_BRIDGE_

#include <bridge/bridge_usb.h>
#include <bridge/bridge_device.h>

typedef void *hbridge_device;

/// <summary>
///		设备链表的节点。有指向下一个节点的指针。这里是 void*，在别的地方会被强制转换为
///		具体的类型。
/// </summary>
typedef void *void_bridge_device_list_node;

#define BRIDGE_SETCMD(pcmd,val)             vatek_uint32_2_buffer((uint8_t*)&pcmd->cmd,val)
#define BRIDGE_PARAM_SET(cmd,idx,val)       vatek_uint32_2_buffer((uint8_t*)&cmd->param.base.params[idx],val)
#define BRIDGE_PARAM_RAW_PTR(cmd)           &cmd->param.raw[0]
#define BRIDGE_MOD_SETIDX(cmd,idx)          vatek_uint32_2_buffer((uint8_t*)&cmd->param.mod.index,idx)
#define BRIDGE_MOD_SETADDR(cmd,addr)        vatek_uint32_2_buffer((uint8_t*)&cmd->param.mod.address,addr)
#define BRIDGE_MOD_SETLEN(cmd,lenval)       vatek_uint32_2_buffer((uint8_t*)&cmd->param.mod.len,lenval)
#define BRIDGE_MOD_SETDATATYPE(cmd,type)    vatek_uint32_2_buffer((uint8_t*)&cmd->param.mod.datatype,type)
#define BRIDGE_MOD_GETBUF(cmd)              &cmd->param.mod.buffer[0]
#define BRIDGE_I2C_SETHANDLE(cmd,val)       vatek_uint32_2_buffer((uint8_t*)&cmd->param.i2c.handle,val)
#define BRIDGE_I2C_SETADDR(cmd,addr)        vatek_uint32_2_buffer((uint8_t*)&cmd->param.i2c.address,addr)
#define BRIDGE_I2C_SETLEN(cmd,len)          vatek_uint32_2_buffer((uint8_t*)&cmd->param.i2c.len,len)

#ifdef __cplusplus
extern "C" {
	#endif

	/// <summary>
	///		会根据不同的系统在编译时选择不同的版本。
	///		* 调用 bridge_device_list_enum_usb 函数扫描 USB 设备。
	///		* 会先用新的供应商 ID查找一遍，如果没有找到设备，再用老的供应商 ID 查找一遍。
	/// </summary>
	/// <param name="root_node">找到的设备会被放到链表中，最后将根节点赋值给 root_node</param>
	/// <returns>总共查找到多少个设备</returns>
	HAL_API int bridge_device_list_enum_usb_with_pid_and_old_pid(void_bridge_device_list_node *root_node);

	/// <summary>
	///		根据供应商ID (vid) 和产品ID (pid) 来找出匹配的设备
	/// </summary>
	/// <param name="vid">供应商ID</param>
	/// <param name="pid">产品ID</param>
	/// <param name="root_node">查找到的设备会被放到一个链表中，最后将链表的根节点指针赋值给 root_node</param>
	/// <returns>找到的设备的数量，即从 root_node 开始到最后一个节点一共有多少个节点</returns>
	HAL_API vatek_result bridge_device_list_enum_usb(uint16_t vid, uint16_t pid, void_bridge_device_list_node *root_node);

	/// <summary>
	///		从链表中获取指定索引位置的设备
	/// </summary>
	/// <param name="root_node">需要传进来链表的根节点</param>
	/// <param name="idx"></param>
	/// <param name="hbridge"></param>
	/// <returns></returns>
	HAL_API vatek_result bridge_device_list_get(void_bridge_device_list_node root_node, int32_t idx, hbridge_device *hbridge);
	HAL_API const char *bridge_device_list_get_name(void_bridge_device_list_node hblist, int32_t idx);

	/// <summary>
	///		释放从 root_node 开始的一连串链表结点
	/// </summary>
	/// <param name="root_node">
	///		链表的根节点。
	///		* 不能传非根节点，否则会造成内存泄漏，因为 void_bridge_device_list_node 是单向链表的节点。
	/// </param>
	/// <returns></returns>
	HAL_API vatek_result bridge_device_list_free(void_bridge_device_list_node root_node);

	HAL_API vatek_result bridge_device_open(hbridge_device hbridge);
	HAL_API const char *bridge_device_get_name(hbridge_device hbridge);
	HAL_API vatek_result bridge_device_close(hbridge_device hbridge);
	HAL_API void bridge_device_lock_command(hbridge_device hbridge);
	HAL_API hid_bridge_cmd * bridge_device_get_command(hbridge_device hbridge);
	HAL_API Phid_bridge_result bridge_device_get_result(hbridge_device hbridge);
	HAL_API vatek_result bridge_device_send_bridge_command(hbridge_device hbridge);
	HAL_API void bridge_device_unlock_command(hbridge_device hbridge);

	/* bridge_device base tool */
	HAL_API vatek_result bridge_device_lock(hbridge_device hbridge);
	HAL_API vatek_result bridge_device_unlock(hbridge_device hbridge);
	HAL_API bridge_device_status bridge_device_get_status(hbridge_device hbridge);
	HAL_API vatek_result bridge_device_bulk_transfer(hbridge_device hbridge, uint32_t type, uint32_t addr, uint8_t *pbuf, uint32_t len);

	/* bridge_device board devices tool */

	HAL_API vatek_result bridge_device_get_info(hbridge_device hbridge, Pbdevice_info pdevinfo);

	HAL_API vatek_result bridge_device_get_demod_info(hbridge_device hbridge, Pbdemod_info pinfo);
	HAL_API vatek_result bridge_device_set_demod_mode(hbridge_device hbridge, Pbdemod_mode_param pmode);
	HAL_API vatek_result bridge_device_get_demod_mode(hbridge_device hbridge, Pbdemod_mode_param pmode);

	HAL_API vatek_result bridge_device_start_demod(hbridge_device hbridge, Pbdemod_op_param param);
	HAL_API vatek_result bridge_device_stop_demod(hbridge_device hbridge);

	HAL_API vatek_result bridge_device_get_source(hbridge_device hbridge, int32_t idx, Pbridge_source pphy);
	HAL_API vatek_result bridge_device_start_source(hbridge_device hbridge, Pbridge_source pvideo);
	HAL_API vatek_result bridge_device_stop_source(hbridge_device hbridge);

	HAL_API vatek_result bridge_device_get_source_status(hbridge_device hbridge, Pbridge_source psource);
	HAL_API const char *bridge_device_get_source_name(hbridge_device hbridge, Pbridge_source psource);

	HAL_API vatek_result bridge_device_start_rfmixer(hbridge_device hbridge, Pbrfmixer_op_param pparam);
	HAL_API vatek_result bridge_device_stop_rfmixer(hbridge_device hbridge);

	HAL_API vatek_result bridge_device_storage_write(hbridge_device hbridge, int32_t nsection, uint8_t *pbuf);
	HAL_API vatek_result bridge_device_storage_read(hbridge_device hbridge, int32_t nsection, uint8_t *pbuf);
	HAL_API vatek_result bridge_device_storage_erase(hbridge_device hbridge, int32_t nsection);

	#ifdef __cplusplus
}
#endif

#endif