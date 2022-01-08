import os.path
import sys
from typing import List

from migi import attach_process


def main(argv: List[str]):
    attach_process(argv[0], work_dir=os.path.dirname(__file__), python_console=[
        'import sys',
        'print(sys.path)'
    ])


if __name__ == '__main__':
    main(sys.argv[1:])
