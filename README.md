# MIGI (My Ideas Got Incepted)

A powerful and versatile dynamic instrumentation toolkit.

## How it works
By injecting Python scripts into target host, migi makes host manipulation possible.

To get started, follow the steps below.

### 1. Install from PyPI
```shell
pip install migi
```

### 2. Attach to target host
```python
import migi

session = migi.attach_process('notepad.exe', python_console=[])
```

An interactive Python console, which runs inside the host app, will show up if nothing goes wrong. 

### 3. Make function calls
```python
from ctypes import *
from migi.decorators import stdcall

@stdcall('MessageBoxW', 'User32.dll')
def _native_message_box_w(hwnd: c_void_p, content: c_wchar_p, title: c_wchar_p, flags: c_uint32) -> c_int32:
    pass


def message_box(content: str, title: str, flags: int = 0) -> c_int32:
    return _native_message_box_w(None, create_unicode_buffer(content), create_unicode_buffer(title), flags)

message_box("I'm in", '1')
```

### 4. Learn more

Learn more about calling convention, api interception and others by referring to docs and examples

## How to build

### 1. Platform support
- Windows
```shell
cmake --build out/windows --target all path_to_src
```
- Other platform plans in roadmap.md
