import os
from platform import architecture
from typing import Optional

import _migi
from migi import Manifest
from migi._internal import get_python_runtime_abi, abi_to_suffix
from migi.exceptions import ProcessDoesNotExist, ABIMismatchException


class Injector:
    def __init__(self, platform_injector: _migi.Injector):
        self._platform_injector = platform_injector

    def inject(self, library_path: str, manifest: Optional[Manifest] = None) -> int:
        extra_parameter_ptr = 0
        if manifest:

            if manifest.python_home:
                self._load_python_shared_libraries(manifest.python_home)

            manifest_str = manifest.to_json_str()
            remote_addr = self._platform_injector.alloc_memory(len(manifest_str) + 1)
            self._platform_injector.write_memory(remote_addr, manifest_str.encode('utf8') + b'\0')
            extra_parameter_ptr = remote_addr
        return self._platform_injector.load_library(library_path, extra_parameter_ptr)

    def _load_python_shared_libraries(self, python_home: str):
        for i in [i for i in os.listdir(python_home) if i.startswith('python') and i.endswith('.dll') and i != 'python3.dll']:
            self._platform_injector.load_library(os.path.join(python_home, i), 0)
        self._platform_injector.load_library(os.path.join(python_home, 'python3.dll'), 0)


class Device:
    DEVICE_TYPE_LOCAL_MACHINE = 0

    PROCESS_ARCHITECTURE_UNKNOWN = 0
    PROCESS_ARCHITECTURE_ABI_32 = 1
    PROCESS_ARCHITECTURE_ABI_64 = 2
    PROCESS_ARCHITECTURE_WOW_64 = 3

    def __init__(self, platform_device: _migi.Device):
        self._platform_device = platform_device

    def create_injector(self, pid: int = -1, process_name: Optional[str] = None):
        assert pid >= 0 or process_name, 'At least one of the arguments(pid, process_name) should be specified'
        if pid < 0:
            pid = self._platform_device.find_process_by_name(process_name)
            if pid == -1:
                raise ProcessDoesNotExist(f'Process "{process_name}" does not exist')
        process_architecture = self._platform_device.get_process_architecture(pid)
        python_runtime_abi = get_python_runtime_abi()
        is_wow64_match = process_architecture == Device.PROCESS_ARCHITECTURE_WOW_64 and python_runtime_abi == Device.PROCESS_ARCHITECTURE_ABI_32
        if not (process_architecture == python_runtime_abi or is_wow64_match):
            arch_type, _ = architecture()
            raise ABIMismatchException(f'Mismatch between Process[{pid}] abi({abi_to_suffix(process_architecture)}) and Python runtime abi({arch_type})')
        injector = self._platform_device.create_injector(pid)
        return Injector(injector)
