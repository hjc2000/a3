#include <cross_bridge.h>
#include <cross_os_api.h>

#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>
#include<Exception.h>
#include<memory>

using namespace std;

#define HID_DEVICE_MAX_NUMS         32
/* not used DDK to compiler so hid define self. (orange in ddk/hid.h) */

typedef USHORT USAGE, *PUSAGE;

typedef struct _HIDD_ATTRIBUTES
{
	ULONG  Size;
	USHORT VendorID;
	USHORT ProductID;
	USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef struct _HIDP_CAPS
{
	USAGE  Usage;
	USAGE  UsagePage;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	USHORT FeatureReportByteLength;
	USHORT Reserved[17];
	USHORT NumberLinkCollectionNodes;
	USHORT NumberInputButtonCaps;
	USHORT NumberInputValueCaps;
	USHORT NumberInputDataIndices;
	USHORT NumberOutputButtonCaps;
	USHORT NumberOutputValueCaps;
	USHORT NumberOutputDataIndices;
	USHORT NumberFeatureButtonCaps;
	USHORT NumberFeatureValueCaps;
	USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_PREPARSED_DATA *PHIDP_PREPARSED_DATA;

typedef BOOLEAN(__stdcall *HidD_GetAttributes_)(HANDLE device, PHIDD_ATTRIBUTES attrib);
typedef BOOLEAN(__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN(__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN(__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
typedef BOOLEAN(__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
typedef NTSTATUS(__stdcall *HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS *caps);
typedef BOOLEAN(__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);

static HidD_GetAttributes_ HidD_GetAttributes;
static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
static HidD_GetManufacturerString_ HidD_GetManufacturerString;
static HidD_GetProductString_ HidD_GetProductString;
static HidD_SetFeature_ HidD_SetFeature;
static HidD_GetFeature_ HidD_GetFeature;
static HidD_GetIndexedString_ HidD_GetIndexedString;
static HidD_GetPreparsedData_ HidD_GetPreparsedData;
static HidD_FreePreparsedData_ HidD_FreePreparsedData;
static HidP_GetCaps_ HidP_GetCaps;
static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;

#define HID_PACKET_LEN              65
#define HID_PACKET_BUFFER_LEN       68
#define HID_PACKET_DATA_OFFSET      4
#define HID_PACKET_START_OFFSET     3
#define HID_TIMEOUT                 1000

/// <summary>
///		储存 windows 下的 HID（Human Interface Device，即人机交互设备）的信息的
///		链表节点。
/// </summary>
struct win_hid_device_list_node
{
	win_hid_device_list_node *pnext;
	uint16_t hid_vid;
	uint16_t hid_pid;
	HANDLE hid_handle;
	void_cross_mutex _mutex;
	char hid_path[MAX_PATH];
	OVERLAPPED hid_overlapped;
	uint8_t rawbuf_tx[HID_PACKET_BUFFER_LEN];
	uint8_t rawbuf_rx[HID_PACKET_BUFFER_LEN];
};

#pragma comment(lib, "Setupapi.lib")

const GUID hid_class_guid = { 0x4d1e55b2, 0xf16f, 0x11cf,{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

extern vatek_result win_hid_api_write(win_hid_device_list_node *pdevice, uint8_t *ppacket);
extern vatek_result win_hid_api_read(win_hid_device_list_node *pdevice, uint8_t *ppacket);

class HidApiModule
{
	HMODULE hid_api_lib = nullptr;

	template<typename FuncType>
	vatek_result LoadFunctionFromLib(const char *funName, FuncType &funcPointer, HMODULE hid_api_lib)
	{
		funcPointer = reinterpret_cast<FuncType>(GetProcAddress(hid_api_lib, funName));

		// 检查是否成功加载
		if (!funcPointer)
		{
			return vatek_unknown;
		}

		return vatek_success;
	}

	void bridge_device_free(void)
	{
		if (hid_api_lib)
		{
			FreeLibrary(hid_api_lib);
			hid_api_lib = nullptr;
		}
	}

	/// <summary>
	///		加载 hid.dll 并加载其中的函数。加载成功返回 0，加载失败返回 vatek_memfail
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	vatek_result bridge_device_init(void)
	{
		#define LOADFUN(fun) fun = (fun##_)GetProcAddress(hid_api_lib,#fun); if(fun == NULL)return vatek_unknown;

		vatek_result nres = vatek_success;
		if (hid_api_lib == nullptr)
		{
			hid_api_lib = LoadLibraryA("hid.dll");
			if (hid_api_lib != nullptr)
			{
				LoadFunctionFromLib("HidD_GetAttributes", HidD_GetAttributes, hid_api_lib);
				LoadFunctionFromLib("HidD_GetSerialNumberString", HidD_GetSerialNumberString, hid_api_lib);
				LoadFunctionFromLib("HidD_GetManufacturerString", HidD_GetManufacturerString, hid_api_lib);
				LoadFunctionFromLib("HidD_GetProductString", HidD_GetProductString, hid_api_lib);
				LoadFunctionFromLib("HidD_SetFeature", HidD_SetFeature, hid_api_lib);
				LoadFunctionFromLib("HidD_GetFeature", HidD_GetFeature, hid_api_lib);
				LoadFunctionFromLib("HidD_GetIndexedString", HidD_GetIndexedString, hid_api_lib);
				LoadFunctionFromLib("HidD_GetPreparsedData", HidD_GetPreparsedData, hid_api_lib);
				LoadFunctionFromLib("HidD_FreePreparsedData", HidD_FreePreparsedData, hid_api_lib);
				LoadFunctionFromLib("HidP_GetCaps", HidP_GetCaps, hid_api_lib);
				LoadFunctionFromLib("HidD_SetNumInputBuffers", HidD_SetNumInputBuffers, hid_api_lib);
			}
			else
			{
				nres = vatek_memfail;
			}
		}

		return nres;
	}

public:
	HidApiModule()
	{
		if (bridge_device_init() < 0)
		{
			throw Exception();
		}
	}

	~HidApiModule()
	{
		bridge_device_free();
	}
};

int bridge_device_list_enum_usb_with_pid_and_old_pid(win_hid_device_list_node * *hblist)
{
	/* bridge_device_list_enum_usb 函数如果没有发生错误，会返回找到的设备的数量，发生错误会
	* 返回错误代码。
	*/
	int amount_of_devices = bridge_device_list_enum_usb(USB_BRIDGE_VID, USB_BRIDGE_PID, hblist);

	/* 找到的设备数量为 0 */
	if (amount_of_devices == 0)
	{
		// 如果找到的设备数量为 0，就换成老的供应商 ID （USB_BRIDGE_VID_OLD），然后再次查找
		amount_of_devices = bridge_device_list_enum_usb(USB_BRIDGE_VID_OLD, USB_BRIDGE_PID, hblist);
	}

	return amount_of_devices;
}

vatek_result bridge_device_list_enum_usb(uint16_t vid, uint16_t pid, win_hid_device_list_node * *root_node)
{
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	HDEVINFO hinfo = INVALID_HANDLE_VALUE;
	vatek_result nres = vatek_success;

	static shared_ptr<HidApiModule> hid_api_module;
	if (!hid_api_module)
	{
		hid_api_module = shared_ptr<HidApiModule>{
			new HidApiModule{}
		};
	}

	memset(&devinfo_data, 0, sizeof(SP_DEVINFO_DATA));
	devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	hinfo = SetupDiGetClassDevsA(&hid_class_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hinfo != INVALID_HANDLE_VALUE)
	{
		int32_t index = 0;
		int32_t datalen = -1;
		int32_t nums = 0;
		win_hid_device_list_node *proot = NULL;
		win_hid_device_list_node *pnext = NULL;
		SP_DEVICE_INTERFACE_DETAIL_DATA_A *intf_detail_info = NULL;

		nres = vatek_success;

		while (SetupDiEnumDeviceInterfaces(hinfo, NULL,
										   &hid_class_guid, index,
										   &device_interface_data))
		{
			BOOL bres = SetupDiGetDeviceInterfaceDetailA(
				hinfo,
				&device_interface_data,
				NULL,
				0,
				(PDWORD)&datalen,
				NULL
			);

			index++;
			if (datalen > 0)
			{
				intf_detail_info = (SP_DEVICE_INTERFACE_DETAIL_DATA_A *)(new uint8_t[datalen]);
				if (!intf_detail_info)
				{
					throw Exception();
				}

				intf_detail_info->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
				bres = SetupDiGetDeviceInterfaceDetailA(
					hinfo,
					&device_interface_data,
					intf_detail_info,
					datalen,
					NULL,
					NULL
				);

				if (bres)
				{
					HANDLE hhid = CreateFileA(
						intf_detail_info->DevicePath,
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
						0
					);

					if (hhid != INVALID_HANDLE_VALUE)
					{
						HIDD_ATTRIBUTES hidattr;
						hidattr.Size = sizeof(HIDD_ATTRIBUTES);
						bres = HidD_GetAttributes(hhid, &hidattr);
						if (bres && (vid == hidattr.VendorID) && (pid == hidattr.ProductID))
						{
							win_hid_device_list_node *pnewhid = (win_hid_device_list_node *)malloc(sizeof(win_hid_device_list_node));
							nres = vatek_memfail;
							if (pnewhid)
							{
								memset(pnewhid, 0, sizeof(win_hid_device_list_node));
								strncpy_s(&pnewhid->hid_path[0], MAX_PATH, &intf_detail_info->DevicePath[0], MAX_PATH);
								pnewhid->hid_vid = hidattr.VendorID;
								pnewhid->hid_pid = hidattr.ProductID;

								if (pnext == NULL)proot = pnewhid;
								else pnext->pnext = pnewhid;
								pnext = pnewhid;
								nums++;
								nres = vatek_success;
							}
						}

						CloseHandle(hhid);
					}
				}

				delete[] intf_detail_info;
			}
		}

		if (is_vatek_success(nres))
		{
			nres = (vatek_result)nums;
			*root_node = proot;
		}
	}

	return nres;
}

vatek_result bridge_device_list_free(win_hid_device_list_node * root_node)
{
	win_hid_device_list_node *proot = (win_hid_device_list_node *)root_node;

	while (proot)
	{
		if (proot->hid_handle != NULL)
			return vatek_badstatus;
		proot = proot->pnext;
	}

	proot = (win_hid_device_list_node *)root_node;
	while (proot)
	{
		win_hid_device_list_node *pnext = proot->pnext;
		cross_os_free_mutex(proot->_mutex);
		free(proot);
		proot = pnext;
	}

	return vatek_success;
}

vatek_result bridge_device_list_get(win_hid_device_list_node *root_node, int32_t idx, win_hid_device_list_node * *hbridge)
{
	int32_t nums = 0;
	while (root_node)
	{
		if (nums == idx)
		{
			*hbridge = root_node;
			return vatek_success;
		}

		// 前往链表的下一个节点
		root_node = root_node->pnext;
		nums++;
	}

	return vatek_badparam;
}

const char *bridge_device_list_get_name(win_hid_device_list_node *hblist, int32_t idx)
{
	win_hid_device_list_node * hbridge = NULL;
	vatek_result nres = bridge_device_list_get(hblist, idx, &hbridge);
	if (is_vatek_success(nres))
		return &((win_hid_device_list_node *)hbridge)->hid_path[0];

	return NULL;
}

vatek_result bridge_device_open(win_hid_device_list_node * hbridge)
{
	vatek_result nres = vatek_badstatus;
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	if (phid->hid_handle == NULL)
	{
		void_cross_mutex hlock;
		nres = cross_os_create_mutex(&hlock);
		if (is_vatek_success(nres))
		{
			phid->hid_handle = CreateFileA(
				&phid->hid_path[0],
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
				0
			);

			if (phid->hid_handle != INVALID_HANDLE_VALUE)
			{
				phid->hid_overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				if (phid->hid_overlapped.hEvent != INVALID_HANDLE_VALUE)
				{
					hid_bridge_cmd *pcmd = (hid_bridge_cmd *)&phid->rawbuf_tx[HID_PACKET_DATA_OFFSET];
					memset(&phid->rawbuf_tx[0], 0, HID_PACKET_BUFFER_LEN);
					memset(&phid->rawbuf_rx[0], 0, HID_PACKET_BUFFER_LEN);
					memcpy(&pcmd->tag[0], &hid_bridge_tag[0], 4);
					phid->_mutex = hlock;
					return vatek_success;
				}

				CloseHandle(phid->hid_handle);
				phid->hid_handle = NULL;
			}

			if (!is_vatek_success(nres))
				cross_os_free_mutex(hlock);
		}
	}

	return nres;
}

void bridge_device_lock_command(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	cross_os_lock_mutex(phid->_mutex);
}

void bridge_device_unlock_command(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	cross_os_release_mutex(phid->_mutex);
}

const char *bridge_device_get_name(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	return &phid->hid_path[0];
}

vatek_result bridge_device_close(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	if (phid->hid_handle != NULL)
	{
		cross_os_free_mutex(phid->_mutex);
		CloseHandle(phid->hid_overlapped.hEvent);
		CloseHandle(phid->hid_handle);
	}

	phid->hid_overlapped.hEvent = NULL;
	phid->hid_handle = NULL;
	phid->_mutex = NULL;
	return vatek_success;
}


hid_bridge_cmd *bridge_device_get_command(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	return (hid_bridge_cmd *)&phid->rawbuf_tx[HID_PACKET_DATA_OFFSET];
}

hid_bridge_result * bridge_device_get_result(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	return (hid_bridge_result *)&phid->rawbuf_rx[HID_PACKET_DATA_OFFSET];
}

vatek_result bridge_device_send_bridge_command(win_hid_device_list_node * hbridge)
{
	win_hid_device_list_node *phid = (win_hid_device_list_node *)hbridge;
	vatek_result nres = win_hid_api_write(phid, &phid->rawbuf_tx[HID_PACKET_START_OFFSET]);
	if (is_vatek_success(nres))
	{
		nres = win_hid_api_read(phid, &phid->rawbuf_rx[HID_PACKET_START_OFFSET]);
		if (is_vatek_success(nres))
		{
			hid_bridge_result * presult = bridge_device_get_result(hbridge);
			presult->result = vatek_buffer_2_uint32((uint8_t *)&presult->result);
			presult->cmd = vatek_buffer_2_uint32((uint8_t *)&presult->cmd);
			if (strncmp((char *)&presult->tag[0], &hid_bridge_tag[0], 4) != 0)nres = vatek_badparam;
			else nres = (vatek_result)presult->result;
		}
	}
	return nres;
}

vatek_result win_hid_api_write(win_hid_device_list_node *pdevice, uint8_t *ppacket)
{
	uint32_t nwrite = 0;
	vatek_result nres = vatek_badstatus;
	if (pdevice->hid_handle)
	{
		BOOL bres = WriteFile(pdevice->hid_handle, ppacket, HID_PACKET_LEN, (LPDWORD)&nwrite, &pdevice->hid_overlapped);
		nres = vatek_hwfail;
		if (bres || (GetLastError() == ERROR_IO_PENDING))
		{
			int32_t nerr = (vatek_result)WaitForSingleObject(pdevice->hid_overlapped.hEvent, INFINITE);
			if (nerr == WAIT_OBJECT_0)
			{
				bres = GetOverlappedResult(pdevice->hid_handle, &pdevice->hid_overlapped, (LPDWORD)&nwrite, TRUE);
				if (bres && nwrite == HID_PACKET_LEN)nres = vatek_success;
			}
		}

		if (!is_vatek_success(nres))
			CancelIo(pdevice->hid_handle);
	}

	return nres;
}

vatek_result win_hid_api_read(win_hid_device_list_node *pdevice, uint8_t *ppacket)
{
	uint32_t nread = 0;
	vatek_result nres = vatek_badstatus;
	if (pdevice->hid_handle)
	{
		BOOL bres = ReadFile(pdevice->hid_handle, ppacket, HID_PACKET_LEN, (LPDWORD)&nread, &pdevice->hid_overlapped);
		nres = vatek_hwfail;
		if (bres || (GetLastError() == ERROR_IO_PENDING))
		{
			int32_t nerr = (vatek_result)WaitForSingleObject(pdevice->hid_overlapped.hEvent, INFINITE);
			if (nerr == WAIT_OBJECT_0)
			{
				bres = GetOverlappedResult(pdevice->hid_handle, &pdevice->hid_overlapped, (LPDWORD)&nread, TRUE);
				if (bres && nread == HID_PACKET_LEN)nres = vatek_success;
			}
		}
		if (!is_vatek_success(nres))CancelIo(pdevice->hid_handle);
	}
	return nres;
}
