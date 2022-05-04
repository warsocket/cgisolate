#!/usr/bin/env python3
import os
print("Content-type: text/plain\r")
print("\r")
print(f"process uid: {os.getuid()}")
print(f"pid: {os.getpid()}")