#include "BasicMethods.h"

using namespace System;
namespace AsrClientWrapper{
	public ref class AsrNativeAdbDevice
	{
	private:
		unsigned long _timeout = 5000;
		void* _nativeHandles;

		AsrNativeAdbDevice(Handles* handles)
		{
			_nativeHandles = handles;
		}

	public:
		//timeout = -1 -----> infinite loop
		static AsrNativeAdbDevice^ FindAndOpen(unsigned long timeout)
		{
			Handles* nativeHandles = find_and_open_handles(timeout);

			if (nativeHandles == nullptr) {
				throw gcnew InvalidOperationException("Open usb handles failed");
			}

			AsrNativeAdbDevice^ d = gcnew AsrNativeAdbDevice(nativeHandles);
			return d;
		}
		property bool IsOpen
		{
			bool get() {
				return _nativeHandles != nullptr;
			}
		}
		property unsigned long Timeout
		{
			unsigned long get() {
				return _timeout;
			}
			void set(unsigned long value) {
				_timeout = value;
			}
		}
		property unsigned long LastErrorCode
		{
			unsigned long get() {
				if (_nativeHandles == nullptr) return 0;
				return GetLastError();
			}
		}
		unsigned long Read(array<System::Byte>^ buffer, int offset, int count)
		{
			if (!IsOpen) return -1;
			if (buffer == nullptr) throw gcnew ArgumentNullException("buffer");
			if (offset < 0 || count < 0 || offset + count > buffer->Length) throw gcnew ArgumentOutOfRangeException();

			pin_ptr<System::Byte> pinned = &buffer[offset];
			void* nativePtr = pinned;
			return usb_read(((Handles*)_nativeHandles)->read_endpoint, nativePtr, count, _timeout);;
		}
		unsigned long Write(array<System::Byte>^ buffer, int offset, int count)
		{
			if (!IsOpen) return -1;
			if (buffer == nullptr) throw gcnew ArgumentNullException("buffer");
			if (offset < 0 || count < 0 || offset + count > buffer->Length) throw gcnew ArgumentOutOfRangeException();

			pin_ptr<System::Byte> pinned = &buffer[offset];
			void* nativePtr = pinned;
			return usb_write(((Handles*)_nativeHandles)->write_endpoint, nativePtr, (size_t)count, _timeout);
		}

		// ÏÔÊ½¹Ø±Õ
		void Close()
		{
			if (_nativeHandles != nullptr) {
				Handles* handles = (Handles*)_nativeHandles;
				if (handles->read_endpoint != nullptr) {
					AdbCloseHandle(handles->read_endpoint);
				}
				if (handles->write_endpoint != nullptr) {
					AdbCloseHandle(handles->write_endpoint);
				}
				if (handles->interface_handle != nullptr) {
					AdbCloseHandle(handles->interface_handle);
				}
				delete handles;
				_nativeHandles = nullptr;
			}
		}

		// IDisposable & finalizer
		~AsrNativeAdbDevice()
		{
			Close();
			GC::SuppressFinalize(this);
		}

		!AsrNativeAdbDevice()
		{
			Close();
		}
	};
}