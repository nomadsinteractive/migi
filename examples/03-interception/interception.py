import os.path
import sys
from typing import List

from migi import attach_process


def main(argv: List[str]):
    attach_process(argv[0], work_dir=os.path.dirname(__file__), python_console=[
        'from api import message_box, restore',
        'message_box("I\'m in", "1")',
        'message_box("Some other text", "Some other title")',
        'restore()',
        'message_box("I\'m in", "1")'
    ])


if __name__ == '__main__':
    main(sys.argv[1:])
