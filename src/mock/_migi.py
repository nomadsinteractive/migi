import struct
from typing import List


def logd(s):
    print(s)


def detach():
    pass


def is_mocked():
    return True


def sleep_for(seconds: float):
    pass


def find_file(filename: str, working_dir: str):
    pass


def sizeof_void_p() -> int:
    return struct.calcsize("P")


def dump_bytes(memory: bytes) -> str:
    pass


def load_library(library_name: str, flags: int) -> int:
    pass


def get_module_addr(module_name: str, offset: int) -> int:
    return 0


def get_module_proc(module_name: str, proc_name: str) -> int:
    return 0


def make_call(proc_addr: int, args: List[int], stack_size: int = 0) -> int:
    pass


def make_thiscall(proc_addr: int, this_ptr: int, args: List[int], stack_size: int = 0) -> int:
    pass


def make_fastcall(proc_addr: int, arg0: int, arg1: int, args: List[int], stack_size: int = 0) -> int:
    pass


def stdcall_to_fastcall(func_addr: int) -> int:
    pass


def stdcall_to_fastcall_recycle(func_addr: int) -> int:
    pass


def stdcall_to_thiscall(func_addr: int) -> int:
    pass


def stdcall_to_thiscall_recycle(func_addr: int) -> int:
    pass


class Interceptor:
    def __init__(self):
        pass

    @property
    def function_entry(self) -> int:
        return 0

    def intercept(self) -> int:
        pass

    def restore(self) -> int:
        pass


def make_interceptor(function_original: int, function_detoure_to: int) -> Interceptor:
    return Interceptor()


def get_module_file_path() -> str:
    pass


def start_console(cmds: List[str]):
    pass


class Console:

    def show(self):
        pass

    def close(self):
        pass

    def read_line(self) -> str:
        pass

    def write(self, output: str):
        pass


class Injector:

    def alloc_memory(self, size: int) -> int:
        pass

    def write_memory(self, remote_addr: int, content: bytes) -> int:
        pass

    def free_memory(self, remote_addr: int):
        pass

    def load_library(self, library_path: str, extra_parameter_ptr: int) -> int:
        pass


class Device:

    def create_console(self) -> Console:
        pass

    def create_injector(self, pid: int) -> Injector:
        pass

    def find_process_by_name(self, name: str) -> int:
        pass


def create_device(device_type: int) -> Device:
    pass
