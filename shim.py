#!/usr/bin/env python3

########## WARNING ##########
# This script is run as root using a suid binary shim so it can Isolate itslef.
# Do NOT modify anything between the WARNING labels, You will introduce glaring security holes.
from os import setuid, setgid, chroot, chdir
from sys import exit
NOBODY=0xfffe

try:
	chroot("/var/www/html/")
	chdir("/")
	setgid(NOBODY)
	setuid(NOBODY)
except: #something no going to plan, ktnxbye
	exit(0xfe)
########## WARNING ##########
