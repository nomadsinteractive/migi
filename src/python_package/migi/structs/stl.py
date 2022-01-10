from ctypes import Structure, c_char, byref, string_at, create_string_buffer, POINTER, memmove, addressof
from typing import Union

import _migi
from migi._internal import get_c_uintptr


class String(Structure):
    BufType = c_char * 16

    def __init__(self, value: Union[str, bytes] = b''):
        super().__init__()
        self._string_ref = None
        self._size = 0
        self._max_size = 15
        self._buf = b''
        self.set(value)

    def c_str(self) -> bytes:
        if self._max_size < 16:
            return self._buf[:self._size]
        return string_at(int.from_bytes(self._buf[:_migi.sizeof_void_p()], byteorder='little'), self._size)

    def str(self, encoding: str = 'utf8') -> str:
        return self.c_str().decode(encoding)

    @property
    def is_reference(self) -> bool:
        return getattr(self, '_string_ref', None) is not None

    def reset(self):
        self._size = 0
        self._max_size = 15
        self._buf = b''

    def set(self, content: Union[str, bytes]):
        content = content if isinstance(content, bytes) else content.encode()
        self._size = len(content)
        self._string_ref = None
        if self._size < self._max_size:
            memmove(byref(self._buf), content + b'\0', self._size + 1)
        else:
            self._string_ref = create_string_buffer(content)
            POINTER(get_c_uintptr()).from_address(byref(self._buf)).value = addressof(self._string_ref)

    def __str__(self) -> str:
        return self.str()

    _fields_ = [
            ('_buf', BufType),
            ('_size', get_c_uintptr()),
            ('_max_size', get_c_uintptr())
    ]


string = String
string_p = POINTER(string)
