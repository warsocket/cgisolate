LIBFLAGS = -lcap -pthread

all: cgiscript cgiscript.py

cgiscript: shim.o shim.py.o
	gcc $(CFLAGS) shim.o shim.py.o -o cgiscript $(LIBFLAGS)

cgiscript.py:
	cat shim.py > cgiscript.py
	cat code.py >> cgiscript.py
	chmod +x cgiscript.py

shim.py.o:
	ld -r -b binary -o shim.py.o shim.py

shim.o:
	gcc $(CFLAGS) -c shim.c -o shim.o $(LIBFLAGS)

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
	@echo "'make CFLAGS='' LIBFLAGS='$(LIBFLAGS) to compile \e[1;32msafe\e[0m version (default)"
	@echo "'make CFLAGS='-DUNSAFE' to compile the \e[1;31munsafe\e[0m version"
	@echo "'make LIBFLAGS='$(LIBFLAGS) -static' to compile the static version (larger but more portable)"
	@echo "CFLAGS and LIBFLAGS can be combined if desired"
	@echo ""
	@echo "Dont't forget to chown root:root and chmod u+s the cgiscript binary after installation"
 