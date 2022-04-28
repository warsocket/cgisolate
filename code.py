#!/usr/bin/env python3
import os
print("\r")
print(f"process uid: {os.getuid()}")
print(f'current dir: {" ".join(os.listdir())}')
print(f'root dir (/): {" ".join(os.listdir("/"))}')
print(f"pid: {os.getpid()}")
exit()