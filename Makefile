all: cgiscript cgiscript.py

cgiscript: shim.o shim.py.o
	gcc $(CFLAGS) shim.o shim.py.o -o cgiscript

cgiscript.py:
	cat shim.py > cgiscript.py
	cat code.py >> cgiscript.py
	chmod +x cgiscript.py

shim.py.o:
	ld -r -b binary -o shim.py.o shim.py

shim.o:
	gcc $(CFLAGS) -c shim.c -o shim.o

clean: 
	rm shim.o 2> /dev/null || true
	rm shim.py.o 2> /dev/null || true
	rm cgiscript.py 2> /dev/null || true
	rm cgiscript 2> /dev/null || true

help:
	@echo "'make' to compile normal version"
	@echo "'make CFLAGS=-DUNSAFE' to compile the unsafe version"