/* fail - crash in various possible ways */

#include <linux/seccomp.h>

#include <sys/auxv.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void
segfault()
{
	volatile int *nullp = 0;

	*nullp = 42;
}

void _start();

__attribute__((__constructor__))
void
dlcrash()
{
	volatile int *nullp = 0;

	// don't trigger if we are executed as a program
	if (getauxval(AT_ENTRY) != (unsigned long)_start)
		*nullp = 1337;
}

// can lockup your machine
void
oom()
{
	long c = 0;
	static long last = 0;

	int fd;
	fd = open("/proc/self/oom_score_adj", O_WRONLY);
	write(fd, "1000\n", 5);
	close(fd);

	while (1) {
		long *m = malloc(4096*4096);
		m[0] = last;
		m[1] = c++;
		last = (long)m;
		write(1, ".", 1);
	}
}

void
recurse(char *n)
{
	char m[1024];
	recurse(m);
	if (n)
		m[0] = n[0] = 42;
}

void
abortme()
{
	abort();
}

void
killme()
{
	raise(SIGKILL);
}

void illegalins()
{
#if defined(__x86_64__) || defined(__i386__)
	__asm__ __volatile__ ("ud1" : : : "memory");
#elif defined(__arm__)
	__asm__ __volatile__ (
	#ifndef __thumb__
		".word 0xe7f000f0"
	#else
		".short 0xdeff"
	#endif
		: : : "memory");
#elif defined(__aarch64__)
	__asm__ __volatile__ (".word 0x00800011" : : : "memory");
#else
	#error implement illegalins for this architecture
#endif
}

void trap()
{
	__builtin_trap();
}

int zero = 0;
void divtrap()
{
	zero = 1/zero;
}

void
violate_seccomp()
{
	prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
	chdir("/");
}

int
main(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "123Oacdikrst")) != -1) {
		switch (c) {
		case '1': exit(-1); break;
		case '2': exit(2); break;
		case '3': exit(111); break;
		case 'c': violate_seccomp(); break;
		case 'd': divtrap(); break;
		case 'i': illegalins(); break;
		case 't': trap(); break;
		case 'O': oom(); break;
		case 'a': abortme(); break;
		case 'k': killme(); break;
		case 'r': recurse(0); break;
		case 's': segfault(); break;
		}
	}

	write(2, "Usage: fail [-123Oacdikrst]\n", 28);
	exit(1);
}
