#define _GNU_SOURCE

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static pid_t (*libc_fork)() = NULL;
static int pid_file = -1;
static unsigned int fork_counter = 0;
static int activated = 1;

static void init()
{
    libc_fork = (pid_t(*)())dlsym(RTLD_NEXT, "fork");
    if (!libc_fork)
    {
        exit(1);
    }
    unsetenv("LD_PRELOAD");
    char* fname = getenv("FORK_PRELOAD_FILE");
    if (!fname)
    {
        exit(1);
    }
    pid_file = open(fname, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (pid_file < 0)
    {
        exit(1);
    }
    dprintf(pid_file, "%d:%d\n", getpid(), ++fork_counter);
}

void __attribute__((constructor)) fork_preload_runs_first()
{
    init();
}

pid_t fork(void)
{
    if (!libc_fork)
    {
        exit(3);
    }
    if (!activated)
    {
        return libc_fork();
    }
    pid_t pid = libc_fork();
    if (pid > 0)
    {
        dprintf(pid_file, "%d:%d\n", pid, ++fork_counter);
    }
    else if (pid == 0)
    {
        activated = 0; // do not run on childs
        close(pid_file);
    }

    return pid;
}