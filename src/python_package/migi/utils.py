import os
from typing import Union

import _migi


def get_package_file_path(filename: str):
    dirname, _ = os.path.split(__file__)
    return os.path.abspath(os.path.join(dirname, filename))


def dump_bytes(memory: Union[bytes, str]) -> str:
    return _migi.dump_bytes(memory)
