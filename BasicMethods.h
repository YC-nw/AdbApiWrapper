#pragma once
#include "include/adb_api.h"  
struct Handles
{
	ADBAPIHANDLE interface_handle;
	ADBAPIHANDLE read_endpoint;
	ADBAPIHANDLE write_endpoint;
};
const GUID usb_class_id = ANDROID_USB_CLASS_ID;
Handles* do_usb_open(const wchar_t* device_path);
unsigned long usb_read(ADBAPIHANDLE read_endpoint, void* input, int size, unsigned long timeout);
unsigned long usb_write(ADBAPIHANDLE write_endpoint, void* input, size_t size, unsigned long timeout);
Handles* find_and_open_handles(unsigned long timeout_ms);