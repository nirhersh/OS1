#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <time.h>


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class JobsList;
class Command {
  // TODO: add fields 

public:
  std::string m_commandName;
  std::string m_commandLine;
  Command(const char* cmd_line);
  virtual ~Command() {}
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  bool m_isBackground;
  bool m_isComplex;
  JobsList* m_jobs;
 public:
  char* m_command[COMMAND_MAX_ARGS];
  ExternalCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  char* m_cmdLine;
  std::string m_newDear;
  std::string m_lastDir;
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class ChpromptCommand : public BuiltInCommand {
 public:
  char* m_cmdLine;
  ChpromptCommand(const char* cmd_line);
  virtual ~ChpromptCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
  char* m_cmdLine;
  bool m_isKill;
  JobsList* m_jobs;
public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
 public:
  class JobEntry {
  public:
    int m_jobId;
    time_t m_entryTime;
    bool m_isStopped;
    std::string m_commandName;
    pid_t m_pid;
    std::string m_commandLine;
    JobEntry(int jobId, time_t entryTime, Command* command, bool isStopped, pid_t pid) :
    m_jobId(jobId), m_entryTime(entryTime), m_isStopped(isStopped)
    {
      m_commandName = command->m_commandName;
      m_pid = pid;
      m_commandLine = command->m_commandLine;
    }
    bool isJobStopped();
  };
  std::vector<JobEntry*> m_jobsList;
  int m_maxJobId;
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(Command* cmd, pid_t pid,bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  JobEntry *getFirstJob(int *firstJobId);
  int getJobsListSize(){
    return m_jobsList.size();
  }
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 char* m_cmdLine;
 JobsList* m_jobs;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 char* m_cmdLine;
 JobsList* m_jobs;
public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
  char* m_cmdLine;
  JobsList* m_jobs;
public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(const char* cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  char* m_cmdLine;
  JobsList* m_jobs;
public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  std::string m_shellPrompt;
  std::string m_lastDir;
  SmallShell();
  JobsList* m_jobsList;
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  
  std::string get_lastDir();

  std::string get_shellPrompt();

  JobsList* get_jobsList();

  void set_lastDir(const std::string newDir);

  void set_shellPrompt(const std::string newDir);
};

void printInvalidArgumentsMessage(std::string cmdName);

void printInvalidJobId(std::string cmdName, std::string jobId);

void printEmptyJobsListMessage(std::string cmdName);

void printNoStoppedJobsMessage(std::string cmdName);

void printJobAlreadyRunningMessage(std::string cmdName, std::string jobId);

bool isNumber(char* str);

bool isNumberWithDash(char* str);

void trimDash(char* str);

#endif //SMASH_COMMAND_H_
