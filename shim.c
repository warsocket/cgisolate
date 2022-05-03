#ifdef UNSHARE
	#define _GNU_SOURCE
	#include <sched.h>

	#include <semaphore.h>
	#include <sys/mman.h>
	#include <grp.h>
	#include <sys/mount.h>

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
	| CLONE_NEWUSER
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
	script[str_size-1] = 0; //enforce string termination

	#ifndef UNSAFE //check content of .py file for proper jailing and privilige droppping etc
		size_t prologue_size = _binary_shim_py_end - _binary_shim_py_start;
		char cmp[prologue_size];
		int fd = open(script, O_RDONLY);
		if (read(fd, cmp, prologue_size) != prologue_size) exit(2);

		for(int i=0; i<prologue_size; ++i){
			if (_binary_shim_py_start[i] != cmp[i]) exit(2);
		}
	#endif

	#ifdef UNSHARE

		unshare(flags & (~CLONE_NEWUSER)); //Unshare all stuff except CLONE_NEWUSER
		mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);

		typedef struct {
			sem_t unshare_wait;
			sem_t idmap_wait;	
		} Semaphores;

		Semaphores* sync = mmap(NULL, sizeof(Semaphores), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
		sem_init(&sync->unshare_wait, 1, 0);
		sem_init(&sync->idmap_wait, 1, 0);

		/* 
		Time for some fancy sync stuff, this needs to happen in order:
		- The child needs to ushare(CLONE_NEWUSER)
		- THe parent needs to write to the uid_map / gid_map files
		- THe child needs to execve
		*/

		int cpid = fork();
		if (cpid) {
			sem_wait(&sync->unshare_wait); //parent waiting for chile to unshare
			char filename[22];
			FILE* fp;
			snprintf(filename, sizeof(filename), "/proc/%d/uid_map", cpid);
			fp = fopen(filename, "w");
			fprintf(fp, "0 0 65535\n");
			fclose(fp);

			snprintf(filename, sizeof(filename), "/proc/%d/gid_map", cpid);
			fp = fopen(filename, "w");
			fprintf(fp, "0 0 65535\n");
			fclose(fp);

			sem_post(&sync->idmap_wait); //parent realeasing the *id_map semaphore
			wait(NULL); // Wait for child completion
			return 0;
		}
		
		unshare(flags & CLONE_NEWUSER); //THis breaks some permissions it seems
		sem_post(&sync->unshare_wait); //child done with unshare
		sem_wait(&sync->idmap_wait); //child waiting for parent to write *id_map files
		sethostname("xxx", 3); // eg setting hostname
	#endif

	// execv("/bin/bash", NULL);
	execv(script, NULL);

	return 0;
}

