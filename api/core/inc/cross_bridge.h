#ifndef _CROSS_BRIDGE_
#define _CROSS_BRIDGE_

#include <bridge_usb.h>
#include <bridge_device.h>

struct win_hid_device_list_node;

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

/// <summary>
///		会根据不同的系统在编译时选择不同的版本。
///		* 调用 bridge_device_list_enum_usb 函数扫描 USB 设备。
///		* 会先用新的供应商 ID查找一遍，如果没有找到设备，再用老的供应商 ID 查找一遍。
/// </summary>
/// <param name="root_node">找到的设备会被放到链表中，最后将根节点赋值给 root_node</param>
/// <returns>总共查找到多少个设备</returns>
HAL_API int bridge_device_list_enum_usb_with_pid_and_old_pid(win_hid_device_list_node **root_node);

/// <summary>
///		根据供应商ID (vid) 和产品ID (pid) 来找出匹配的设备
/// </summary>
/// <param name="vid">供应商ID</param>
/// <param name="pid">产品ID</param>
/// <param name="root_node">查找到的设备会被放到一个链表中，最后将链表的根节点指针赋值给 root_node</param>
/// <returns>找到的设备的数量，即从 root_node 开始到最后一个节点一共有多少个节点</returns>
HAL_API vatek_result bridge_device_list_enum_usb(uint16_t vid, uint16_t pid, win_hid_device_list_node **root_node);

/// <summary>
///		从链表中获取指定索引位置的设备
/// </summary>
/// <param name="root_node">需要传进来链表的根节点</param>
/// <param name="idx"></param>
/// <param name="hbridge"></param>
/// <returns></returns>
HAL_API vatek_result bridge_device_list_get(win_hid_device_list_node *root_node, int32_t idx, win_hid_device_list_node **hbridge);
HAL_API const char *bridge_device_list_get_name(win_hid_device_list_node *hblist, int32_t idx);

/// <summary>
///		释放从 root_node 开始的一连串链表结点
/// </summary>
/// <param name="root_node">
///		链表的根节点。
///		* 不能传非根节点，否则会造成内存泄漏，因为 win_hid_device_list_node * 是单向链表的节点。
/// </param>
/// <returns></returns>
HAL_API vatek_result bridge_device_list_free(win_hid_device_list_node *root_node);

HAL_API vatek_result bridge_device_open(win_hid_device_list_node *hbridge);
HAL_API const char *bridge_device_get_name(win_hid_device_list_node *hbridge);
HAL_API vatek_result bridge_device_close(win_hid_device_list_node *hbridge);
HAL_API void bridge_device_lock_command(win_hid_device_list_node *hbridge);
HAL_API hid_bridge_cmd *bridge_device_get_command(win_hid_device_list_node *hbridge);
HAL_API Phid_bridge_result bridge_device_get_result(win_hid_device_list_node *hbridge);
HAL_API vatek_result bridge_device_send_bridge_command(win_hid_device_list_node *hbridge);
HAL_API void bridge_device_unlock_command(win_hid_device_list_node *hbridge);

/* bridge_device base tool */
HAL_API vatek_result bridge_device_lock(win_hid_device_list_node *hbridge);
HAL_API vatek_result bridge_device_unlock(win_hid_device_list_node *hbridge);
HAL_API bridge_device_status bridge_device_get_status(win_hid_device_list_node *hbridge);
HAL_API vatek_result bridge_device_bulk_transfer(win_hid_device_list_node *hbridge, uint32_t type, uint32_t addr, uint8_t *pbuf, uint32_t len);

/* bridge_device board devices tool */

HAL_API vatek_result bridge_device_get_info(win_hid_device_list_node *hbridge, Pbdevice_info pdevinfo);

HAL_API vatek_result bridge_device_get_demod_info(win_hid_device_list_node *hbridge, Pbdemod_info pinfo);
HAL_API vatek_result bridge_device_set_demod_mode(win_hid_device_list_node *hbridge, Pbdemod_mode_param pmode);
HAL_API vatek_result bridge_device_get_demod_mode(win_hid_device_list_node *hbridge, Pbdemod_mode_param pmode);

HAL_API vatek_result bridge_device_start_demod(win_hid_device_list_node *hbridge, Pbdemod_op_param param);
HAL_API vatek_result bridge_device_stop_demod(win_hid_device_list_node *hbridge);

HAL_API vatek_result bridge_device_get_source(win_hid_device_list_node *hbridge, int32_t idx, Pbridge_source pphy);
HAL_API vatek_result bridge_device_start_source(win_hid_device_list_node *hbridge, Pbridge_source pvideo);
HAL_API vatek_result bridge_device_stop_source(win_hid_device_list_node *hbridge);

HAL_API vatek_result bridge_device_get_source_status(win_hid_device_list_node *hbridge, Pbridge_source psource);
HAL_API const char *bridge_device_get_source_name(win_hid_device_list_node *hbridge, Pbridge_source psource);

HAL_API vatek_result bridge_device_start_rfmixer(win_hid_device_list_node *hbridge, Pbrfmixer_op_param pparam);
HAL_API vatek_result bridge_device_stop_rfmixer(win_hid_device_list_node *hbridge);

HAL_API vatek_result bridge_device_storage_write(win_hid_device_list_node *hbridge, int32_t nsection, uint8_t *pbuf);
HAL_API vatek_result bridge_device_storage_read(win_hid_device_list_node *hbridge, int32_t nsection, uint8_t *pbuf);
HAL_API vatek_result bridge_device_storage_erase(win_hid_device_list_node *hbridge, int32_t nsection);

#endif