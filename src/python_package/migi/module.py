from ctypes import c_uint32, string_at
from typing import Optional

from migi import get_module_address


class Module:
    def __init__(self, name: Optional[str] = None):
        self._name = name
        self._address = None

    @property
    def address(self) -> int:
        if self._address is None:
            self._address = get_module_address(self._name, 0) if self._name else 0

        return self._address

    def read_uint32(self, offset: int) -> int:
        return self.read_c_type(c_uint32, offset)

    def read_bytes(self, offset: int, size: int = -1) -> bytes:
        return string_at(self.address + offset, size)

    def read_c_type(self, c_type, offset: int):
        return c_type.from_address(self.address + offset).value