import sys
import time
from http.client import HTTPSConnection
from typing import List


def main(argv: List[str]):
    time.sleep(3)
    for i in range(3):
        for j in argv:
            connection = HTTPSConnection(j)
            connection.request('GET', '/')
            connection.getresponse().read()
            time.sleep(5)


if __name__ == '__main__':
    main(sys.argv[1:])
