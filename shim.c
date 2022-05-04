#define _GNU_SOURCE
#include <sched.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <grp.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/limits.h>

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

extern const char _binary_shim_py_start[];
extern const char _binary_shim_py_end[];

const char const ext[] = ".py";

int main(int argc, char const *argv[])
{
	if (!argc) return 1; // no execution info

	if (setuid(0)) exit(0xfe); //something no going to plan, ktnxbye
	if (setgid(0)) exit(0xfe); //something no going to plan, ktnxbye

	// const size_t const str_size = strlen(argv[0]) + sizeof(ext);
	const size_t MAX_PATH_STRING = PATH_MAX+1;
	char script[MAX_PATH_STRING];
	getcwd(script, MAX_PATH_STRING);
	char cdir[strlen(script)+1];
	strncpy(cdir, script, MAX_PATH_STRING);

	if (snprintf(script, PATH_MAX, "%s/%s%s", cdir, argv[0], ext) < 0) return 1;

	#ifndef UNSAFE //check if content of .py file prologue matches (warning labels)
		size_t prologue_size = _binary_shim_py_end - _binary_shim_py_start;
		char cmp[prologue_size];
		int fd = open(script, O_RDONLY);
		if (read(fd, cmp, prologue_size) != prologue_size) exit(2);

		for(int i=0; i<prologue_size; ++i){
			if (_binary_shim_py_start[i] != cmp[i]) exit(2);
		}
	#endif

	//shared struct for 2 semphores
	typedef struct {
		sem_t unshare_wait;
		sem_t idmap_wait;	
	} Semaphores;

	//Init semaphores in shared memmory
	Semaphores* sync = mmap(NULL, sizeof(Semaphores), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0); 
	sem_init(&sync->unshare_wait, 1, 0);
	sem_init(&sync->idmap_wait, 1, 0);

	//Create ramdon directory in /tmp for mounts
	FILE* fp = fopen("/dev/urandom", "r");
	unsigned long long random;
	fread(&random, sizeof(random), 1, fp);
	fclose(fp);
	const size_t len = 5+4+sizeof(random)*3+1;
	char dir[len];
	snprintf(dir, len, "/tmp/iso-%llu", random);

	if (mkdir(dir, 0) == -1){
		printf("mkdir %d\n", errno);
	}

	/* 
	Time for some fancy sync stuff, this needs to happen in order:
	- The child needs to ushare(CLONE_NEWUSER)
	- THe parent needs to write to the uid_map / gid_map files
	- THe child needs to execve
	*/

	unshare(flags & CLONE_NEWPID); //We need a new proces to unshare our pid nmamespace
	int cpid = fork(); //We need a new proces to unshare our pid nmamespace
	if (cpid) { //parent process
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
		rmdir(dir); // remove mount dir in tmp
		return 0;
	}
	//Child process starts here

	unshare(flags & (~CLONE_NEWPID)); // usnahre the rest

	sem_post(&sync->unshare_wait); //child done with unshare
	sem_wait(&sync->idmap_wait); //child waiting for parent to write *id_map files

	//Remoutn root as Private, make new mount points on tmpfs
	if (mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL) == -1){
		printf("mount %d\n", errno);
		return 1;
	}

	if (mount("none", dir, "tmpfs", 0, "") == -1){
		printf("tmpfsmount %d\n", errno);
	}

	chdir(dir);

	if (mkdir("oldroot", 0) == -1){
		printf("mkdir %d\n", errno);
	}
	
	if (mkdir("jail", 0) == -1){
		printf("mkdir %d\n", errno);
	}

	execv(script, NULL); //Execute the script

	//Python script should perform the following after starting:
	/*
		CDLL(None).syscall(155,b'.', b'oldroot')==0 # Pivot root
		chroot("jail")
		chdir("/")
		setgid(NOBODY)
		setuid(NOBODY)
	*/

	return 0;
}

