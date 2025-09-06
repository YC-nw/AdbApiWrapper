#include "BasicMethods.h"
#include <iostream>
using namespace std;
Handles* do_usb_open(const wchar_t* device_path)  
{  
    Handles* handles = new Handles();  
    handles->interface_handle = AdbCreateInterfaceByName(device_path);  
    if (!handles->interface_handle) {  
        DWORD error = GetLastError();  
        std::cerr << "AdbCreateInterfaceByName failed. Error: " << error << std::endl;  
        AdbCloseHandle(handles->interface_handle);  
        delete handles;
        return nullptr;  
    }  
    handles->read_endpoint = AdbOpenDefaultBulkReadEndpoint(handles->interface_handle, AdbOpenAccessTypeReadWrite, AdbOpenSharingModeReadWrite);  
    if (!handles->read_endpoint) {  
        DWORD error = GetLastError();  
        std::cerr << "AdbOpenDefaultBulkReadEndpoint failed. Error: " << error << std::endl;  
        AdbCloseHandle(handles->interface_handle);
        delete handles;
        return nullptr;  
    }  
    handles->write_endpoint = AdbOpenDefaultBulkWriteEndpoint(handles->interface_handle, AdbOpenAccessTypeReadWrite, AdbOpenSharingModeReadWrite);  
    if (!handles->write_endpoint) {  
        DWORD error = GetLastError();  
        std::cerr << "AdbOpenDefaultBulkWriteEndpoint failed. Error: " << error << std::endl;  
        AdbCloseHandle(handles->read_endpoint);
        AdbCloseHandle(handles->interface_handle);
        delete handles;
        return nullptr;  
    }  
    return handles;  
}
unsigned long usb_read(ADBAPIHANDLE read_endpoint, void* input, int size, unsigned long timeout)
{
	char* ptr = (char*)input;//强制转换输入的类型为char*
	int remaining = size;//剩余100%
	while (remaining >= 0)
	{
		unsigned long chunk = (remaining > 0x100000) ? 0x100000 : remaining;//计算每次读取的块的大小
		unsigned long read = 0;//每次循环前清零上次循环的读取的字节数

		BOOL success = AdbReadEndpointSync
		(
			read_endpoint,//读取端点句柄
			ptr,//将要读取数据的缓冲区指针，指向读取容器
			chunk,//将要读取的块的字节数
			&read,//返回的读取数量
			timeout//600秒超时
		);

		if (!success) //错误读取
		{
			DWORD err = GetLastError();
			printf("Read error: 0x%08lX\n", err);
			return 0;
		}
		if (!read) return 0;
		ptr = ptr + read;//已经读取的字节数
		remaining = remaining - read;//计算剩余
	}

	return size;
}
unsigned long usb_write(ADBAPIHANDLE write_endpoint, void* input, size_t size, unsigned long timeout)
{
	char* ptr = (char*)input;
	size_t remaining = size;
	while (remaining > 0)
	{
		unsigned long chunk = (remaining > 0x100000) ? 0x100000 : remaining;
		unsigned long written = 0;

		BOOL success = AdbWriteEndpointSync
		(
			write_endpoint,
			ptr, 
			chunk,
			&written,
			timeout
		);

		if (!success)
		{
			DWORD err = GetLastError();
			printf("Write error: 0x%08lX\n", err);
			return 0;
		}

		ptr += written;
		remaining -= written;
	}
	return size;
}
Handles* find_and_open_handles(unsigned long timeout_ms)
{
	char* buf = new char[4096];
	unsigned long size = 4096;
	unsigned long long start = GetTickCount64();
	while(1)
	{
		if (AdbNextInterface(AdbEnumInterfaces(usb_class_id, true, true, true), (AdbInterfaceInfo*)buf, &size))
		{
			wchar_t* device_path = reinterpret_cast<wchar_t*>(buf + 280);
			char out[35] = { 0 };
			WideCharToMultiByte(CP_UTF8, 0, device_path, -1, out, 33, NULL, NULL);
			char* last = strrchr(out, '#');
			if (!last)
			{
				return nullptr;
			}
			*(last + 1) = '\0';
			std::wcout << L"device path: " << out << std::endl;
			Handles* handles = do_usb_open(device_path);
			delete[] buf;
			return handles;
		}
		if (timeout_ms != (unsigned long)-1 && GetTickCount64() - start >= timeout_ms)
		{
			std::cout << "timeout reached" << std::endl;
			delete[] buf;
			return (Handles*)-1;
		}
		Sleep(10);
	}
}
