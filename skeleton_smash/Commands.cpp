#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <cstring>
#include <cstdlib>

#include <assert.h>

using namespace std;
#define MAX_LINE_LEN 80
#define SYSCALL_FAILED -1
#define DEFAULT_SHELL_PROMPT "smash"
#define FIRST_ARGUMENT 1
#define MAX_ARGS 80
#define MAX_NUM_OF_PROCESSES 100
#define MAX_NUM_OF_PROCESSES 100

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

//******************************************* Jobs List Implemetation **************************************//


//******************************************* SMALL SHELL IMPLEMENTATION **************************************//


SmallShell::SmallShell() {
// TODO: add your implementation
  set_lastDir("");
  m_shellPrompt = "smash";
  m_jobsList = new JobsList();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  //creating built-in commmands
  if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    char** plastPwd;
    return new ChangeDirCommand(cmd_line, plastPwd);
  }
  else if (firstWord.compare("chprompt") == 0) {
    return new ChpromptCommand(cmd_line);
  }
  // else if (firstWord.compare("jobs") == 0){
  //   return new JobsCommand(cmd_line, this->get_jobsList());
  // }
  //special commands:
  //by default, assuming this is an external command:
  else {
    return new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}


std::string SmallShell::get_lastDir()
{
  //std::cout << "entered getter, path is " << m_lastDir << std::endl;
  return m_lastDir;
}

std::string SmallShell::get_shellPrompt()
{
  return m_shellPrompt;
}

JobsList* SmallShell::get_jobsList()
{
  return m_jobsList;
}

void SmallShell::set_lastDir(const std::string newDir)
{
  m_lastDir = newDir;
  //std::cout << "entered setter, new path is " << m_lastDir << std::endl;
}

void SmallShell::set_shellPrompt(const std::string newPrompt)
{
  m_shellPrompt = newPrompt;
}


//******************************************* Jobs List Implemetation **************************************//

JobsList::JobsList(){
  m_maxJobId = 0;
}

void JobsList::addJob(Command* cmd, bool isStopped){
    m_jobsList.push_back(new JobsList::JobEntry(m_maxJobId+1, time(nullptr), cmd, isStopped));
  m_maxJobId++;
}

void JobsList::printJobsList()
{
  for (JobEntry* job : m_jobsList)
  {
    time_t elapsed = difftime(time(nullptr), job->m_entryTime);
    std::string outputStr = "[" + std::to_string(job->m_jobId) + "] " + job->m_commandName + " : " +
                          std::to_string(job->m_pid) + " " + std::to_string(elapsed) + " secs ";
    if(job->m_isStopped){
      std::cout << outputStr + "(stopped)" << std::endl;
    } else {
      std::cout << outputStr << std::endl;
    }
  }
}

void JobsList::killAllJobs()
{
  for (JobEntry* job : m_jobsList)
  {
    int status = kill(job->m_pid, SIGKILL);
    assert(status != -1);
  }
  m_jobsList.empty(); 
}

void JobsList::removeFinishedJobs()
{
  
}






//******************************************* HELPFULL FUNCTIONS **************************************//



//******************************************* BUILT-IN COMMANDS IMPLEMENTATION **************************************
//Command::~Command() {}
Command::Command(const char* cmd_line)
{
  char* args[MAX_ARGS];
  int argsNum = _parseCommandLine(cmd_line, args);
  m_commandName = args[0];
}


BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line){}


ShowPidCommand::ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void ShowPidCommand::execute()
{
  int pid = getpid();
  std::cout<< "smash pid is " << pid << std::endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute()
{
  char path[MAX_LINE_LEN];
  if (getcwd(path, sizeof(path)) != nullptr) {
      std::cout << std::string(path) << std::endl;
  } else {
      std::cout << "" << std::endl;
  }
}




ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line)
{
  m_cmdLine = new char(strlen(cmd_line) + 1);
  strcpy(m_cmdLine, cmd_line);
}

void ChangeDirCommand::execute()
{
  char* args[MAX_ARGS];
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if (argsNum > 2){
    std::cout << "smash error: cd: too many agruments" << std::endl;
  }
  else if(argsNum < 2){
    std::cout << "what do we suppose to do?" << std::endl;
  } else {
    SmallShell& shell = SmallShell::getInstance();
    char* newPath = args[FIRST_ARGUMENT];
    char currentPath[MAX_LINE_LEN];
    getcwd(currentPath, sizeof(currentPath));
    const char* oldPath = shell.get_lastDir().c_str();
    string oldPathstr = shell.get_lastDir();
    if(strcmp(newPath, "-") == 0)
    {
      if(strlen(oldPath) == 0)
      {
        std::cout << "smash error: cd: OLDPWD not set" << std::endl;
      } else{
        chdir(oldPath);
        shell.set_lastDir(currentPath);
      }
    } else {
      if(chdir(newPath) == SYSCALL_FAILED)
      {
        std::cout << "what do we suppose to do?" << std::endl;
      } else { //if change dir successed
        shell.set_lastDir(currentPath);
      }
    }  
  }  
}

ChpromptCommand::ChpromptCommand(const char* cmd_line):  BuiltInCommand(cmd_line)
{
  m_cmdLine = new char(strlen(cmd_line) + 1);
  strcpy(m_cmdLine, cmd_line);
}

void ChpromptCommand::execute()
{
  char* args[MAX_ARGS];
  int argsNum = _parseCommandLine(m_cmdLine, args);
  SmallShell& shell = SmallShell::getInstance();
  if(argsNum == 1)
  {
    shell.set_shellPrompt(DEFAULT_SHELL_PROMPT);
  } else {
    shell.set_shellPrompt(args[FIRST_ARGUMENT]);
  }
}

// JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs):  BuiltInCommand(cmd_line)
// {
//   m_cmdLine = new char(strlen(cmd_line) + 1);
//   strcpy(m_cmdLine, cmd_line);
//   m_jobs = jobs;
// }




//******************************************* EXTERNAL COMMANDS IMPLEMENTATION **************************************//

char** merge_arguments_arrays(char** arr1, char** arr2) {
    int len1 = 0, len2 = 0;
    while (arr1[len1] != nullptr) len1++;
    while (arr2[len2] != nullptr) len2++;

    char** mergedArray = new char*[len1 + len2 + 1];
    std::memcpy(mergedArray, arr1, len1 * sizeof(char*));
    std::memcpy(mergedArray + len1, arr2, (len2 + 1) * sizeof(char*));

    return mergedArray;
}

bool is_complex_external_command(const char* cmd_line)
{
  std::string str(cmd_line);
    if (str.find('*') != std::string::npos || str.find('?') != std::string::npos) {
      return true;
    }
    return false;
}

ExternalCommand::ExternalCommand(const char* cmd_line): Command(cmd_line)
{
  m_isBackground = _isBackgroundCommand(cmd_line);
  m_isComplex = is_complex_external_command(cmd_line);
  char* commandDup = (char*)malloc(strlen(cmd_line) + 1); //to remove the & sign if exists
  strcpy(commandDup, cmd_line);
  _removeBackgroundSign(commandDup);
  _parseCommandLine(commandDup, m_command);
}

void ExternalCommand::execute()
{
  if(!m_isComplex){
    if(m_isBackground){ //in case of background
      int pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        execvp(m_command[0], m_command);
        return;
      } else { //parent proccess
        return;
      }
    } else { // in case of foreground
      int pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        execvp(m_command[0], m_command);
        return;
      } else { //parent proccess
        waitpid(pid, nullptr, 0);
        return;
      }
    }
  } else { //in case of complex
    char* bashFlag[] = {"-c"};
    if(m_isBackground){ //in case of bachground
      int pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        execvp("bash", merge_arguments_arrays(bashFlag, m_command));
        return;
      } else { //parent proccess
        return;
      }
    } else { // // in case of foreground
      int pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        execvp("bash", merge_arguments_arrays(bashFlag, m_command));
        return;
      } else { //parent proccess
        waitpid(pid, nullptr, 0);
        return;
      }
    }
  }
}

void JobsCommand::execute(){
  return;
}

//******************************************* SPECIAL COMMAND IMPLEMENTATION **************************************//

