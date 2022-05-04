#!/usr/bin/env python3

########## WARNING ##########

# This script is run as root using a suid binary shim so it can Isolate itslef.
# Do NOT modify anything between the WARNING labels, 
# This finalizes the isolation
# You can introduce glaring security holes. by doing this.

from os import setuid, setgid, chroot, chdir
from ctypes import CDLL
from sys import exit
NOBODY=0xfffe

try:
	assert( CDLL(None).syscall(155,b'.', b'oldroot')==0 )
	chroot("jail")
	chdir("/")
	setgid(NOBODY)
	setuid(NOBODY)
except: #something no going to plan, ktnxbye
	exit(0xfe)
	
########## WARNING ##########
