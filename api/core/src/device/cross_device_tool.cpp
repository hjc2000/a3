#include "./internal/cross_device_tool.h"

/// <summary>
///		这个似乎才是链表类，而里面的 win_hid_device_list_node * 和 void_usb_device_list 其实应该
///		是链表结点。
/// </summary>
struct cross_handle
{
	int32_t reference = 0;
	cross_device *root = nullptr;
	cross_device *last = nullptr;

	/// <summary>
	///		bridge 设备的链表
	/// </summary>
	win_hid_device_list_node * bridges = nullptr;

	/// <summary>
	///		usb 设备的链表
	/// </summary>
	void_usb_device_list usbdevices = nullptr;
};

static cross_handle m_cdevices{};

/// <summary>
///		pcross 是指针的指针，因为本函数需要更改来自调用者的 cross_device* 变量，让它
///		指向别的地方。
/// </summary>
/// <param name="pcross"></param>
/// <returns></returns>
vatek_result cross_devices_create(cross_device **pcross)
{
	int nres = vatek_success;
	if (!m_cdevices.root)
	{
		int32_t i = 0;
		int32_t nums = 0;

		nres = bridge_device_list_enum_usb_with_pid_and_old_pid(&m_cdevices.bridges);
		if (nres > vatek_success)
		{
			nums = nres;
			for (i = 0; i < nums; i++)
			{
				win_hid_device_list_node * hbridge = NULL;
				nres = bridge_device_list_get(m_cdevices.bridges, i, &hbridge);
				if (is_vatek_success(nres))
				{
					nres = cross_bridge_open(hbridge, pcross);
					if (is_vatek_success(nres))
					{
						if (!m_cdevices.root)
							m_cdevices.root = *pcross;
						else
							m_cdevices.last->next = *pcross;
						m_cdevices.last = *pcross;
					}
				}

				if (!is_vatek_success(nres))
					VWAR("cross_devices_create - bridge fail [%d:%d]", i, nres);
			}
		}

		nres = usb_api_ll_enum(usb_type_all, &m_cdevices.usbdevices);
		if (nres > vatek_success)
		{
			nums = nres;
			for (i = 0; i < nums; i++)
			{
				usb_handle * husb = NULL;
				nres = usb_api_ll_list_get_device(m_cdevices.usbdevices, i, &husb);
				if (is_vatek_success(nres))
				{
					nres = cross_usb_device_open(husb, pcross);
					if (is_vatek_success(nres))
					{
						if (!m_cdevices.root)
							m_cdevices.root = *pcross;
						else
							m_cdevices.last->next = *pcross;

						m_cdevices.last = *pcross;
					}
				}
			}
		}
	}

	if (m_cdevices.root)
	{
		m_cdevices.reference++;
		*pcross = m_cdevices.root;
		nres = (vatek_result)cross_devices_get_size(m_cdevices.root);
	}
	else
	{
		nres = vatek_success;
	}

	return (vatek_result)nres;
}

vatek_result cross_devices_create_by_usbid(uint16_t vid, uint16_t pid, cross_device **pcross)
{
	vatek_result nres = vatek_success;
	if (!m_cdevices.root)
	{
		m_cdevices.bridges = NULL;
		nres = usb_api_ll_enum_by_id(vid, pid, &m_cdevices.usbdevices);
		if (nres > vatek_success)
		{
			int32_t nums = nres;
			int32_t i = 0;
			cross_device *newcross = NULL;
			for (i = 0; i < nums; i++)
			{
				usb_handle * husb = NULL;
				nres = usb_api_ll_list_get_device(m_cdevices.usbdevices, i, &husb);
				if (is_vatek_success(nres))
				{
					nres = cross_usb_device_open(husb, &newcross);
					if (is_vatek_success(nres))
					{
						if (!m_cdevices.root)
							m_cdevices.root = newcross;
						else
							m_cdevices.last->next = newcross;

						m_cdevices.last = newcross;
					}
				}
			}
		}
	}

	if (m_cdevices.root)
	{
		m_cdevices.reference++;
		*pcross = m_cdevices.root;
		nres = (vatek_result)cross_devices_get_size(m_cdevices.root);
	}
	else
	{
		nres = vatek_success;
	}

	return nres;
}

vatek_result cross_devices_free(cross_device *pcross)
{
	vatek_result nres = vatek_badstatus;
	if (m_cdevices.reference)
	{
		m_cdevices.reference--;
		nres = vatek_success;
		if (!m_cdevices.reference)
		{
			cross_device *ptrdev = m_cdevices.root;
			while (ptrdev)
			{
				cross_device *pnext = ptrdev->next;
				if (ptrdev->driver == cdriver_bridge)
					cross_bridge_close(ptrdev);
				else if (ptrdev->driver == cdriver_usb)
					cross_usb_device_close(ptrdev);
				else
					VWAR("bad memory unknown cross device - %08x", ptrdev->driver);

				ptrdev = pnext;
			}

			if (m_cdevices.usbdevices)
				usb_api_ll_free_list(m_cdevices.usbdevices);
			if (m_cdevices.bridges)
				bridge_device_list_free(m_cdevices.bridges);
			m_cdevices.usbdevices = NULL;
			m_cdevices.bridges = NULL;
			m_cdevices.root = NULL;
			m_cdevices.last = NULL;
		}
	}
	else
	{
		VWAR("cross_devices_free ref underflow");
	}

	return nres;
}

vatek_result cross_devices_get_size(cross_device *pcross)
{
	int32_t nums = 0;
	while (pcross)
	{
		nums++;
		pcross = pcross->next;
	}

	return (vatek_result)nums;
}

vatek_result cross_device_malloc(cross_device **pcross, hal_service_mode hal)
{
	vatek_result nres = vatek_unsupport;
	if (hal == service_rescue || hal == service_broadcast || hal == service_transform)
	{
		cross_device *newdev = new cross_device;
		nres = vatek_memfail;
		if (newdev)
		{
			memset(newdev, 0, sizeof(cross_device));
			nres = vatek_success;
			*pcross = newdev;
		}
	}

	return nres;
}
