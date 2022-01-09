import os
import sys
from importlib.machinery import PathFinder, ModuleSpec, ExtensionFileLoader, EXTENSION_SUFFIXES
from platform import architecture, python_version_tuple
from typing import Optional


class MigiExtensionPathFinder(PathFinder):
    @classmethod
    def find_spec(cls, fullname, path=None, target=None) -> Optional[ModuleSpec]:
        if fullname == '_migi':
            extension_suffixs = list(EXTENSION_SUFFIXES)
            limited_api_ext_suffix = get_python_runtime_limited_api_suffix('pyd')
            if limited_api_ext_suffix not in extension_suffixs:
                extension_suffixs.append(limited_api_ext_suffix)

            dirname, _ = os.path.split(__file__)
            for i in extension_suffixs:
                module_path = os.path.join(dirname, f'bin/{fullname}{i}')
                if os.path.isfile(module_path):
                    return ModuleSpec(fullname, ExtensionFileLoader(fullname, module_path), origin=module_path)
        return None


def get_python_runtime_abi():
    arch_type, _ = architecture()
    return 1 if arch_type == "32bit" else 2


def get_python_runtime_limited_api_suffix(suffix: str):
    version_major, _, _ = python_version_tuple()
    arch_type, _ = architecture()
    abi = get_python_runtime_abi()
    return f'.cp{version_major}-{abi_to_suffix(abi)}.{suffix}'


def abi_to_suffix(abi: int):
    suffix_by_abi = {
        0: "unknown",
        1: "win32",
        2: "win_amd64",
        3: "win32_wow64"
    }
    return suffix_by_abi[abi]


sys.meta_path.insert(0, MigiExtensionPathFinder)
