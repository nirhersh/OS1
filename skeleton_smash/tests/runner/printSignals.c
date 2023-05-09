#include <limits.h>
#include <sys/signal.h>
#include <unistd.h>

#define MSG "received signal number: "

void handler(int sig_num)
{
    sleep(1);
    write(STDOUT_FILENO, MSG, sizeof(MSG) - 1);
    if (sig_num < 10)
    {
        char num = sig_num + 0x30;
        write(STDOUT_FILENO, &num, 1);
    }
    else
    {
        char num = (sig_num / 10) + 0x30;
        write(STDOUT_FILENO, &num, 1);
        num = (sig_num % 10) + 0x30;
        write(STDOUT_FILENO, &num, 1);
    }
    char new_line = '\n';
    write(STDOUT_FILENO, &new_line, 1);
}

int main()
{
    for (int i = 0; i < _NSIG; i++)
    {
        signal(i, handler);
    }
    while (1)
    {
        sleep(UINT_MAX);
    }
}