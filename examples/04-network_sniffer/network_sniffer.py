import os.path
import time
from multiprocessing import Process

import host
from migi import attach_process


def main():
    proc = Process(target=host.main, args=(['www.python.org'], ))
    proc.start()
    time.sleep(1)
    attach_process(proc.pid, work_dir=os.path.dirname(__file__))
    proc.join()


if __name__ == '__main__':
    main()
