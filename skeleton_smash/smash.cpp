#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"


int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if(signal(SIGALRM , alarmHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::string cmd_line;
        std::cout << smash.get_shellPrompt() << "> ";
        std::getline(std::cin, cmd_line);
        smash.get_jobsList()->removeFinishedJobs();
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}