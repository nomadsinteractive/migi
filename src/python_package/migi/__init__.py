import os
import sys
from ctypes import c_uint32, addressof, c_void_p, cast, c_int64, c_uint64, \
    c_ulonglong, c_longlong
from importlib.machinery import PathFinder, ModuleSpec, ExtensionFileLoader, EXTENSION_SUFFIXES
from platform import python_version_tuple, architecture
from typing import List, Optional, Any, Union


class MigiExtensionPathFinder(PathFinder):
    @classmethod
    def find_spec(cls, fullname, path=None, target=None) -> Optional[ModuleSpec]:
        if fullname == '_migi':
            extension_suffixs = list(EXTENSION_SUFFIXES)
            version_major, _, _ = python_version_tuple()
            arch_type, _ = architecture()
            limited_api_ext_suffix = f'.cp{version_major}-{"win32" if arch_type == "32bit" else "win_amd64"}.pyd'
            if limited_api_ext_suffix not in extension_suffixs:
                extension_suffixs.append(limited_api_ext_suffix)

            dirname, _ = os.path.split(__file__)
            for i in extension_suffixs:
                module_path = os.path.join(dirname, f'bin/{fullname}{i}')
                if os.path.isfile(module_path):
                    return ModuleSpec(fullname, ExtensionFileLoader(fullname, module_path), origin=module_path)
        return None


sys.meta_path.insert(0, MigiExtensionPathFinder)


import _migi
from migi.manifest import Manifest
from migi.platform import Injector, Device
from migi.session import Session
from migi.utils import get_package_file_path

_manifest_file_path = ''


def find_file(filename: str):
    working_dir, _ = os.path.split(_manifest_file_path)
    return _migi.find_file(filename, working_dir)


def detach():
    _migi.detach()


def is_mocked() -> bool:
    return _migi.is_mocked()


def make_call(proc_addr: int, *args, stack_size: int = 0):
    packed_args = sum([_pack_one_argument(i) for i in args], [])
    return _migi.make_call(proc_addr, packed_args, stack_size)


def make_thiscall(proc_addr: int, this_ptr: int, *args, stack_size: int = 0):
    packed_args = sum([_pack_one_argument(i) for i in args], [])
    return _migi.make_thiscall(proc_addr, this_ptr, packed_args, stack_size)


def make_fastcall(proc_addr: int, arg0: int, arg1: int, *args, stack_size: int = 0):
    packed_args = sum([_pack_one_argument(i) for i in args], [])
    return _migi.make_fastcall(proc_addr, arg0, arg1, packed_args, stack_size)


def get_module_file_path() -> str:
    return _migi.get_module_file_path()


def get_module_address(module_name: str, symbol: Union[int, str]) -> int:
    return _migi.get_module_addr(module_name, symbol) if type(symbol) is int else _migi.get_module_proc(module_name, symbol)


def attach_process(pid_or_proc_name: Union[int, str], *, device: Union[int, Device] = Device.DEVICE_TYPE_LOCAL_MACHINE, **kwargs):
    if isinstance(device, int):
        device = Device(_migi.create_device(device))

    version_major, _, _ = python_version_tuple()
    arch_type, _ = architecture()
    library_name = f'bin/migi.cp{version_major}-{"win32" if arch_type == "32bit" else "win_amd64"}.dll'
    library_path = get_package_file_path(library_name)

    injector = device.create_injector(pid=pid_or_proc_name) if isinstance(pid_or_proc_name, int) else device.create_injector(process_name=pid_or_proc_name)
    injector.inject(library_path, Manifest(**kwargs) if kwargs else None)
    return Session(device, injector)


def _pack_one_argument(obj: Any, obj_signature: Optional[Any] = None) -> List[int]:
    if type(obj) is int and obj_signature in (c_uint64, c_int64, c_ulonglong, c_longlong):
        max_value = 1 << (_migi.sizeof_void_p() * 8)
        if obj >= max_value:
            return [obj & (max_value - 1), obj >> (_migi.sizeof_void_p() * 8)]
        return [obj, 0]

    if type(obj) is int:
        return [obj if obj >= 0 else c_uint32(obj).value]

    try:
        value = obj.value
    except AttributeError:
        try:
            return [addressof(obj._obj)]
        except AttributeError:
            return [0 if obj is None else cast(obj, c_void_p).value]

    if value is None:
        return [0]
    if type(value) is int:
        return [value]
    if type(value) in (bytes, str):
        return [addressof(obj)]

    return [cast(obj, c_void_p).value]