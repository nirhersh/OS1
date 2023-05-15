#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
  std::cout << "smash: got ctrl-Z" << std::endl;
  SmallShell& smasholog = SmallShell::getInstance();
  if(smasholog.m_forgroundPid != -1){
    if(smasholog.m_isForeGround){
      smasholog.get_jobsList()->getJobById(smasholog.m_forgroundJobid)->m_entryTime = time(nullptr);
      smasholog.get_jobsList()->getJobById(smasholog.m_forgroundJobid)->m_isStopped = true;
    }else{
      Command* cmd = smasholog.CreateCommand((smasholog.m_forgroundCmdLine).c_str());
      smasholog.get_jobsList()->addJob(cmd, smasholog.m_forgroundPid, true);
    }
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
  SmallShell& smashulash = SmallShell::getInstance();
  int index = -1;
  for(Alarm& alrm : smashulash.m_timeoutCommands)
  {
    index++;
    if(alrm.timeUntilAlarm() <= 0){
      std::cout << "smash: got an alarm" << std::endl;
      if(alrm.m_pid != -1 && waitpid(alrm.m_pid, nullptr, WNOHANG) == 0){
        kill(alrm.m_pid, 9);
        smashulash.get_jobsList()->removeFinishedJobs();
        std::cout << "smash: " + alrm.m_cmdLine + " timed out!" << std::endl;
      }
      smashulash.m_timeoutCommands.erase(smashulash.m_timeoutCommands.begin() + index);
      break;
    }
  }
  if(smashulash.m_timeoutCommands.empty()){
      return;
    }
    time_t min_diff = smashulash.m_timeoutCommands.front().timeUntilAlarm();
    for(Alarm& alrm : smashulash.m_timeoutCommands)
    {
      if(alrm.timeUntilAlarm() < min_diff){
        min_diff = alrm.timeUntilAlarm();
      }
    }
    alarm(min_diff);
}

