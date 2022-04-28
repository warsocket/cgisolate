#ifdef UNSHARE
	#define _GNU_SOURCE
	#include <sched.h>

	//see unshare(2) for information om flags
	const int flags = 0
	| CLONE_FILES
	| CLONE_FS
	| CLONE_NEWCGROUP
	| CLONE_NEWIPC
	| CLONE_NEWNET
	| CLONE_NEWNS
	| CLONE_NEWPID
	// | CLONE_NEWTIME //tad too modern
	// | CLONE_NEWUSER
	| CLONE_NEWUTS
	| CLONE_SYSVSEM
	;
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>


extern const char _binary_shim_py_start[];
extern const char _binary_shim_py_end[];

const char const ext[] = ".py";

int main(int argc, char const *argv[])
{
	if (!argc) return 1; // no execution info

	if (setuid(0)) exit(0xfe); //something no going to plan, ktnxbye
	if (setgid(0)) exit(0xfe); //something no going to plan, ktnxbye

	const size_t const str_size = strlen(argv[0]) + sizeof(ext);
	char script[str_size];
	snprintf(script, str_size, "%s%s", argv[0], ext);
	script[str_size-1] = 0; //enforce termination
	// printf("%s", script);

	#ifndef UNSAFE //check content of .py file for proper jailing and privilige droppping etc
		size_t prologue_size = _binary_shim_py_end - _binary_shim_py_start;
		// write(2,_binary_shim_py_start, prologue_size);
		char cmp[prologue_size];
		int fd = open(script, O_RDONLY);
		if (read(fd, cmp, prologue_size) != prologue_size) exit(2);

		for(int i=0; i<prologue_size; ++i){
			if (_binary_shim_py_start[i] != cmp[i]) exit(2);
		}
	#endif

	#ifdef UNSHARE
		unshare(flags);

		if (fork()) {
			wait(NULL); // Wait for child completion
			return 0;
		}
	#endif

	execv(script, NULL);

	return 0;
}
