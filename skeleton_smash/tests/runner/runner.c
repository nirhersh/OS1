#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PRELOAD_ENV "RUNNER_PRELOAD"
#define MAX_WAIT_TIME 30

static size_t read_full(int fd, char* buf, size_t count)
{
    size_t total_read = 0;
    do
    {
        size_t res = read(fd, buf + total_read, count - total_read);
        if (res == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("read");
            exit(33);
        }
        if (res == 0)
        {
            break;
        }
        total_read += res;
    } while (total_read < count);
    return total_read;
}

static void write_full(int fd, const char* buf, size_t count)
{
    size_t total_wrote = 0;
    do
    {
        size_t res = write(fd, buf + total_wrote, count - total_wrote);
        if (res == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("write");
            exit(44);
        }
        total_wrote += res;
    } while (total_wrote < count);
}

int child_code(int fds[2], char** argv)
{
    if (dup2(fds[0], STDIN_FILENO) < 0)
    {
        perror("dup2");
        return 2;
    }
    if (close(fds[0]) < 0)
    {
        perror("close");
        return 3;
    }
    if (close(fds[1]) < 0)
    {
        perror("close");
        return 4;
    }
    char* preload = getenv(PRELOAD_ENV);
    if (preload)
    {
        setenv("LD_PRELOAD", preload, 1);
    }
    execv(argv[0], argv);
    perror("execv");
    return 5;
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void wait_child_to_read_pipe(int fd_pipe_read, int pid)
{
    int err;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_pipe_read, &fds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    time_t started_waiting = time(NULL);
    while ((err = select(fd_pipe_read + 1, &fds, NULL, NULL, &timeout)) == 1) /* there is data available */
    {
        if (waitpid(-1, NULL, WNOHANG) != 0)
        {
            exit(13);
        }
        if (time(NULL) - started_waiting > MAX_WAIT_TIME) // child is probably stuck
        {
            fprintf(stderr, "runner: Timeout, killing child\n");
            kill(pid, SIGKILL);
            exit(75); // child is probably stuck
        }
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;
        msleep(100);
    }
    if (err == -1)
    {
        perror("select");
        exit(55);
    }
}

static int waitpid_with_timeout(pid_t pid, int timeout_sec, int* status)
{
    sigset_t mask;
    sigset_t orig_mask;
    struct timespec timeout;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0)
    {
        perror("sigprocmask");
        return 1;
    }

    timeout.tv_sec = timeout_sec;
    timeout.tv_nsec = 0;

    do
    {
        if (sigtimedwait(&mask, NULL, &timeout) < 0)
        {
            if (errno == EINTR)
            {
                /* Interrupted by a signal other than SIGCHLD. */
                continue;
            }
            else if (errno == EAGAIN)
            {
                fprintf(stderr, "runner: Timeout, killing child\n");
                kill(pid, SIGKILL);
            }
            else
            {
                perror("sigtimedwait");
                return 1;
            }
        }
        break;
    } while (1);

    if (waitpid(pid, status, 0) < 0)
    {
        perror("waitpid");
        return 1;
    }
    return 0;
}

int parent_code(int pid, int fds[2])
{
    char c;
    int started_caret = 0;
    int skip_until_line_break = 0;
    while (read_full(STDIN_FILENO, &c, 1) != 0)
    {
        if (skip_until_line_break && c != '\n')
        {
            continue;
        }
        else if (skip_until_line_break && c == '\n')
        {
            skip_until_line_break = 0;
        }
        else if (c == '^')
        {
            if (started_caret)
            {
                write_full(fds[1], "^", 1);
            }
            started_caret = 1;
        }
        else if (c == 'C' && started_caret)
        {
            wait_child_to_read_pipe(fds[0], pid);
            msleep(400);
            if (kill(pid, SIGINT) == -1)
            {
                perror("kill");
                return 12;
            }
            started_caret = 0;
            skip_until_line_break = 1;
        }
        else if (c == 'Z' && started_caret)
        {
            wait_child_to_read_pipe(fds[0], pid);
            msleep(400);
            if (kill(pid, SIGTSTP) == -1)
            {
                perror("kill");
                return 12;
            }
            started_caret = 0;
            skip_until_line_break = 1;
        }
        else if (isdigit(c) && started_caret)
        {
            wait_child_to_read_pipe(fds[0], pid);
            sleep(c - 0x30);
            started_caret = 0;
            skip_until_line_break = 1;
        }
        else
        {
            if (started_caret)
            {
                write_full(fds[1], "^", 1);
            }
            write_full(fds[1], &c, 1);
        }
        int res = waitpid(pid, NULL, WNOHANG);
        if (res == pid)
        {
            return 13;
        }
        else if (res == -1)
        {
            perror("waitpid");
            return 14;
        }
    }
    close(fds[0]);
    close(fds[1]);

    return waitpid_with_timeout(pid, MAX_WAIT_TIME, NULL);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <executable> <args>\n", argv[0]);
        return 1;
    }
    int fds[2];
    if (pipe(fds) < 0)
    {
        return 1;
    }
    int pid = fork();
    if (pid == 0)
    {
        return child_code(fds, &argv[1]);
    }
    else if (pid > 0)
    {
        return parent_code(pid, fds);
    }
    else
    {
        perror("fork");
        return 10;
    }
}