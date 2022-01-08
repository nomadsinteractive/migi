from typing import Optional, List

import _migi


def log(s, *args):
    _migi.logd(str(s) % args if args else str(s))


def start(cmds: Optional[List[str]] = None):
    _migi.start_console(cmds or [])
