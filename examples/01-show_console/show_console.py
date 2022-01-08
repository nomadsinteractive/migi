import sys
from typing import List

from migi import attach_process


def main(argv: List[str]):
    attach_process(argv[0], python_console=[])


if __name__ == '__main__':
    main(sys.argv[1:])
