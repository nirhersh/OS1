#include <iostream>
#include <signal.h>
#include <sys/wait.h>
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
    std::cout << "smash: process " << smasholog.m_forgroundPid << " was stopped" << std::endl;
  }
  smasholog.m_forgroundPid = -1;
  smasholog.m_forgroundCmdLine = "";
}

void ctrlCHandler(int sig_num) {
  std::cout << "smash: got ctrl-C" << std::endl;
  SmallShell& smashroom = SmallShell::getInstance();
  if(smashroom.m_forgroundPid != -1){
    kill(smashroom.m_forgroundPid, SIGKILL);
    std::cout << "smash: process " << smashroom.m_forgroundPid << " was killed" << std::endl;
  }
  smashroom.m_forgroundPid = -1;
  smashroom.m_forgroundCmdLine = "";
}

void alarmHandler(int sig_num) {
  SmallShell& smashichan = SmallShell::getInstance();
  for(auto& pair : smashichan.m_childAlarm){
    time_t diff = difftime(time(nullptr), std::get<2>(pair.second));
    //std::cout << "diff: " << diff << "pid: " << pair.first << std::endl;
    if(diff >= std::get<1>(pair.second)){
      std::cout << "smash: got an alarm" << std::endl;
      std::cout << "smash: " + std::get<0>(pair.second) + " timed out!" << std::endl;
      int result = kill(pair.first, SIGKILL);
      if(result == -1){
        perror("smash error: kill failed");
        return;
      }else{
        smashichan.m_childAlarm.erase(pair.first);
        break;
      }
    }
    for(auto& pair : smashichan.m_childAlarm){
      int status;
      pid_t result = waitpid(pair.first, &status, WNOHANG | WUNTRACED);
      if(WIFEXITED(status) || WIFSIGNALED(status)){ //process finished
          smashichan.m_childAlarm.erase(pair.first);
          break;
      }
    }
  }
}

