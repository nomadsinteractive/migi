from ctypes import *

from migi.decorators import stdcall


@stdcall('MessageBoxW', 'User32.dll', interceptable=True)
def _native_message_box_w(hwnd: c_void_p, content: c_wchar_p, title: c_wchar_p, flags: c_uint32) -> c_int32:
    if wstring_at(content) == "I'm in":
        return _native_message_box_w.call_original(hwnd, create_unicode_buffer("We're in"), title, flags)
    return _native_message_box_w.call_original(hwnd, content, title, flags)


def message_box(content: str, title: str, flags: int = 0) -> c_int32:
    return _native_message_box_w(None, create_unicode_buffer(content), create_unicode_buffer(title), flags)


def restore():
    _native_message_box_w.restore()


_native_message_box_w.intercept()
