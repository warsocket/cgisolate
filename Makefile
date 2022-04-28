CFLAGS = -DUNSHARE

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

.htaccess:
	@echo '<Files cgiscript>'
	@echo '	SetHandler cgi-script'
	@echo '	Options +ExecCGI'
	@echo '</Files>'
	@echo '<Files cgiscript.py>'
	@echo '	Require all denied'
	@echo '</Files>'

help:
	@echo "'make' to compile \e[1;32mnormal version\e[0m"
	@echo "'make CFLAGS='-DUNSHARE' to compile \e[1;32msafe\e[0m version \e[1;32mwith unsharing\e[0m (default)"
	@echo "'make CFLAGS='' to compile the \e[1;32msafe\e[0m version \e[1;31mwithout unsharing\e[0m"
	@echo "'make CFLAGS='-DUNSAFE -UNSHARE' to compile the \e[1;31munsafe\e[0m version \e[1;32mwith unsharing\e[0m"
	@echo "'make CFLAGS='-DUNSAFE' to compile the \e[1;31munsafe\e[0m version \e[1;31mwithout unsharing\e[0m"
