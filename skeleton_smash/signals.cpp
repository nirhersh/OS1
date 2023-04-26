#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
  std::cout << "smash: got ctrl-Z" << std::endl;
  SmallShell& smasholog = SmallShell::getInstance();
  if(smasholog.m_forgroundPid != -1){
    Command* cmd = smasholog.CreateCommand((smasholog.m_forgroundCmdLine).c_str());
    smasholog.get_jobsList()->addJob(cmd, smasholog.m_forgroundPid, true);
    kill(smasholog.m_forgroundPid, SIGSTOP);
    std::cout << "smash: proccess " << smasholog.m_forgroundPid << " was stopped" << std::endl;
  }
  smasholog.m_forgroundPid = -1;
  smasholog.m_forgroundCmdLine = "";
}

void ctrlCHandler(int sig_num) {
  std::cout << "smash: got ctrl-C" << std::endl;
  SmallShell& smashroom = SmallShell::getInstance();
  if(smashroom.m_forgroundPid != -1){
    kill(smashroom.m_forgroundPid, SIGKILL);
    std::cout << "smash: proccess " << smashroom.m_forgroundPid << " was killed" << std::endl;
  }
  smashroom.m_forgroundPid = -1;
  smashroom.m_forgroundCmdLine = "";
}

void alarmHandler(int sig_num) {
  std::cout << "smash: got an alarm" << std::endl;
}

