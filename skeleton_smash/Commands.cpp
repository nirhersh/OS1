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
#include <cctype>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <algorithm>

using namespace std;
#define MAX_LINE_LEN 80
#define SYSCALL_FAILED -1
#define DEFAULT_SHELL_PROMPT "smash"
#define FIRST_ARGUMENT 1
#define MAX_ARGS 80
#define MAX_NUM_OF_PROCESSES 100
#define BUFF_SIZE 100

void splitString(const char* str1, char* str2, char* str3, const char* symbol);

const std::string WHITESPACE = " \n\r\t\f\v";

char* cutUpToChar(char* str, char ch);

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
  m_forgroundPid = -1;
  m_forgroundCmdLine = "";
  m_isForeGround = false;
  m_forgroundJobid = -1;
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

  //special commands:
  if (std::strchr(cmd_line, '>') != nullptr) {
    return new RedirectionCommand(cmd_line);
  }
  if (std::strchr(cmd_line, '|') != nullptr) {
    return new PipeCommand(cmd_line);
  }
  else if (firstWord.compare("getfileinfo") == 0) {
    return new GetFileTypeCommand(cmd_line);
  }
  else if(firstWord.compare("setcore") == 0){
    return new SetcoreCommand(cmd_line, m_jobsList);
  }
  else if(firstWord.compare("chmod") == 0){
    return new ChmodCommand(cmd_line);
  }
  //creating built-in commmands
  else if (firstWord.compare("") == 0) {
    //std::cout << "command empty" << std::endl;
    return nullptr;
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
    return new ChpromptCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0){
    return new JobsCommand(cmd_line, m_jobsList);
  }
  else if(firstWord.compare("quit") == 0){
    return new QuitCommand(cmd_line, m_jobsList);
  }
  else if(firstWord.compare("fg") == 0){
    return new ForegroundCommand(cmd_line, m_jobsList);
  }
  else if(firstWord.compare("bg") == 0){
    return new BackgroundCommand(cmd_line, m_jobsList);
  }
  else if(firstWord.compare("kill") == 0){
    return new KillCommand(cmd_line, m_jobsList);
  }
  //special commands:
  //by default, assuming this is an external command:
  else {
    return new ExternalCommand(cmd_line, this->get_jobsList());
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  Command* cmd = CreateCommand(cmd_line);
  if(cmd == nullptr){
    return;
  }
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

bool JobsList::JobEntry::isJobStopped(){
  int status;
  int res = waitpid(m_pid, &status, WNOHANG | WUNTRACED);
  if(res == -1){
    perror("smash error: waitpid failed");
    return false; 
  }
  return(WIFSTOPPED(status));
}

JobsList::JobsList(){
  m_maxJobId = 0;
}

void JobsList::addJob(Command* cmd, pid_t pid, bool isStopped){
  removeFinishedJobs();
  m_jobsList.push_back(new JobsList::JobEntry(m_maxJobId+1, time(nullptr), cmd, isStopped, pid));
  m_maxJobId++;
}

void JobsList::printJobsList()
{
  for (JobEntry* job : m_jobsList)
  {
    time_t elapsed = difftime(time(nullptr), job->m_entryTime);
    std::string outputStr = "[" + std::to_string(job->m_jobId) + "] " + job->m_commandLine + " : " +
                          std::to_string(job->m_pid) + " " + std::to_string(elapsed) + " secs";
    if(job->m_isStopped){
      std::cout << outputStr + " (stopped)" << std::endl;
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
  m_maxJobId = 0; 
}

void JobsList::removeFinishedJobs()
{
  for(long unsigned int i=0; i<m_jobsList.size(); ){
    int status;
    pid_t result = waitpid(m_jobsList[i]->m_pid, &status, WNOHANG | WUNTRACED);
    if(result == 0){ //process is still running
      ++i;
      continue;
    }else if(WIFEXITED(status) || WIFSIGNALED(status)){ //process finished
      m_jobsList.erase(m_jobsList.begin() + i);
    }else{
      perror("smash error: waitpid failed");
    }
    if(m_jobsList.size() != 0){
      m_maxJobId = m_jobsList.back()->m_jobId;
    }else{
      m_maxJobId = 0;
    }
    
  }
}

JobsList::JobEntry * JobsList::getJobById(int jobId){
  for(JobEntry* job : m_jobsList){
    if (job->m_jobId == jobId){
      return job;
    }
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId){
  for(long unsigned int i=0; i<m_jobsList.size(); i++){
    if(m_jobsList[i]->m_jobId == jobId){
      m_jobsList.erase(m_jobsList.begin() + i);
      break;
    }
  }
  if(!m_jobsList.empty()){
    m_maxJobId = m_jobsList.back()->m_jobId;
  }else{
    m_maxJobId = 0;
  }
  
}

JobsList::JobEntry * JobsList::getLastJob(int* lastJobId){
  if(m_jobsList.size() == 0){
    return nullptr;
  }else{
    if(lastJobId){
      *lastJobId = m_jobsList.back()->m_jobId;
    }
    return m_jobsList.back();
  }
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId){
  JobEntry* lastStoppedJob = nullptr;
  for(long unsigned int i=0; i<m_jobsList.size(); i++){
    bool result =m_jobsList[i]->m_isStopped;
    if(result == false){ //process is still running
      continue;
    }else{//process stopped
      lastStoppedJob = m_jobsList[i];
    }
  }
  *jobId = lastStoppedJob->m_jobId;
  return lastStoppedJob;
}

JobsList::JobEntry *JobsList::getFirstJob(int *firstJobId){
  if(m_jobsList.size() == 0){
    return nullptr;
  }else{
    *firstJobId = m_jobsList.front()->m_jobId;
    return m_jobsList.front();
  }
}


//******************************************* HELPFULL FUNCTIONS **************************************//

void printInvalidArgumentsMessage(std::string cmdName){
  std::cerr << "smash error: " + cmdName +": invalid arguments" << std::endl;
}

void printInvalidJobId(std::string cmdName, std::string jobId){
  std::cerr << "smash error: " + cmdName + ": job-id " + jobId + " does not exist" << std::endl;
}

void printInvalidCoreNumMessage(std::string cmdName){
  std::cerr << "smash error: " + cmdName +": invalid core number" << std::endl;
}

void printEmptyJobsListMessage(std::string cmdName){
  std::cerr << "smash error: " + cmdName + ": jobs list is empty" << std::endl;
}

void printNoStoppedJobsMessage(std::string cmdName){
  std::cerr << "smash error: " + cmdName + ": there is no stopped jobs to resume" << std::endl;
}

void printJobAlreadyRunningMessage(std::string cmdName, std::string jobId){
  std::cerr << "smash error: " + cmdName + ": job-id " + jobId + " is already running in the background" << std::endl;
}

bool isNumber(char* str){
  if(str == nullptr){
    return false;
  }
  if(*str == '-'){
    str++;
  }
  while(*str != '\0'){
    if(!std::isdigit(*str)){
      return false;
    }
    str++;
  }
  return true;
}

bool isNumberWithDash(char* str) {
    int len = std::strlen(str);
    if (len == 0) {
        return false;
    }
    if (str[0] != '-') {
        return false;
    }
    for (int i = 1; i < len; i++) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

void trimDash(char* str){
  std::string s(str);
    if (!s.empty() && s[0] == '-') {
        s.erase(0, 1);
    }
    std::strcpy(str, s.c_str());
}

//******************************************* BUILT-IN COMMANDS IMPLEMENTATION **************************************
//Command::~Command() {}
Command::Command(const char* cmd_line)
{
  char* args[COMMAND_MAX_ARGS];
  int argsNum = _parseCommandLine(cmd_line, args);
  if(argsNum > 0){
    m_commandName = args[0];
  }else{
    m_commandName = "";
  }
  m_commandLine = cmd_line;
  SmallShell& smashdude = SmallShell::getInstance();
  smashdude.m_isForeGround = false;
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




ChangeDirCommand::ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line)
{
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
}

void ChangeDirCommand::execute()
{
  char* args[COMMAND_MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if (argsNum > 2){
    std::cerr << "smash error: cd: too many arguments" << std::endl;
    return;
  }
  else if(argsNum < 2){
    return;
  } else {
    SmallShell& shell = SmallShell::getInstance();
    char* newPath = args[FIRST_ARGUMENT]; //= {nullptr};
    char currentPath[MAX_LINE_LEN] = {0};
    getcwd(currentPath, sizeof(currentPath));
    const char* oldPath = shell.get_lastDir().c_str();
    string oldPathstr = shell.get_lastDir();
    if(strcmp(newPath, "-") == 0)
    {
      if(strlen(oldPath) == 0)
      {
        std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
      } else{
        if(chdir(oldPath) == SYSCALL_FAILED){
          perror("smash error: chdir failed");
          return;
        }else{
          shell.set_lastDir(currentPath);
        }
      }
    } else {
      if(chdir(newPath) == SYSCALL_FAILED){
        perror("smash error: chdir failed");
        return;
      }
      else { //if change dir successed
        shell.set_lastDir(currentPath);
      }
    }  
  }  
}

ChpromptCommand::ChpromptCommand(const char* cmd_line):  BuiltInCommand(cmd_line)
{
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
}

void ChpromptCommand::execute()
{
  char* args[COMMAND_MAX_ARGS];
  int argsNum = _parseCommandLine(m_cmdLine, args);
  SmallShell& shell = SmallShell::getInstance();
  if(argsNum == 1)
  {
    shell.set_shellPrompt(DEFAULT_SHELL_PROMPT);
  } else {
    shell.set_shellPrompt(args[FIRST_ARGUMENT]);
  }
}

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs):  BuiltInCommand(cmd_line)
{
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
}

void JobsCommand::execute(){
  m_jobs->removeFinishedJobs();
  m_jobs->printJobsList();
}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
}

void ForegroundCommand::execute(){
  char* args[MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if(argsNum > 2 && strcmp(args[2], "&") != 0){
    printInvalidArgumentsMessage("fg");
    return;
  }
  if(args[FIRST_ARGUMENT]){
    _removeBackgroundSign(args[FIRST_ARGUMENT]);
  }
  if(argsNum == 2 && !isNumber(args[1])){
    printInvalidArgumentsMessage("fg");
    return;
  }
  int jobToFgId;
  JobsList::JobEntry* jobToFg;
  if(args[1]){ //we have a jobid to bring to fg
    jobToFgId = std::stoi(args[1]);
    jobToFg = m_jobs->getJobById(jobToFgId);
    if(!jobToFg){
      printInvalidJobId("fg", args[1]);
      return;
    }
  }else{
    if(m_jobs->getJobsListSize() == 0){
      printEmptyJobsListMessage("fg");
      return;
    }else{
      jobToFg = m_jobs->getLastJob(&jobToFgId);
    }
  } 
  std::cout << jobToFg->m_commandLine + " : " + std::to_string(jobToFg->m_pid) << std::endl;
  int result = kill(jobToFg->m_pid, SIGCONT); //wake up child
  if(result == -1){
    perror("smash error: kill failed");
    return;
  }
  SmallShell& smashguy = SmallShell::getInstance();
  smashguy.m_forgroundCmdLine = jobToFg->m_commandLine;
  smashguy.m_forgroundPid = jobToFg->m_pid;
  smashguy.m_isForeGround = true;
  smashguy.m_forgroundJobid = jobToFg->m_jobId;
  jobToFg->m_isStopped = false;
  pid_t waitResult = waitpid(jobToFg->m_pid, nullptr, WUNTRACED); //wait for child to finish
  if(waitResult == -1){
    perror("smash error: waitpid failed");
    return;
  }
}

BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
}

void BackgroundCommand::execute(){
  char* args[MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if(argsNum > 2 && strcmp(args[2], "&") != 0){
    printInvalidArgumentsMessage("bg");
    return;
  }
  if(args[FIRST_ARGUMENT]){
    _removeBackgroundSign(args[FIRST_ARGUMENT]);
  }
  if(argsNum == 2 && !isNumber(args[FIRST_ARGUMENT])){
    printInvalidArgumentsMessage("bg");
    return;
  }
  int jobToBgId = 0;
  JobsList::JobEntry* jobToBg;
  if(argsNum == 1){
    if(m_jobs->getJobsListSize() == 0){
      printNoStoppedJobsMessage("bg");
      return;
    }
    jobToBg = m_jobs->getLastStoppedJob(&jobToBgId);
  }else{
    jobToBgId = std::stoi(args[1]);
    jobToBg = m_jobs->getJobById(jobToBgId);
    if(jobToBg == nullptr){
      printInvalidJobId("bg", args[1]);
      return;
    }
  }
  if(!jobToBg->m_isStopped){
    printJobAlreadyRunningMessage("bg", std::to_string(jobToBgId));
    return;
  }
  std::cout << jobToBg->m_commandLine +" : " + std::to_string(jobToBg->m_pid) << std::endl;
  SmallShell& smashguy = SmallShell::getInstance();
  smashguy.m_forgroundCmdLine = jobToBg->m_commandLine;
  smashguy.m_forgroundPid = jobToBg->m_pid;
  jobToBg->m_isStopped = false;
  int result = kill(jobToBg->m_pid, SIGCONT);
  if (result == -1){
    perror("smash error: kill failed");
    return;
  }
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
  m_isKill = false;
}

void QuitCommand::execute(){
  char* args[MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if((argsNum == 2 && strcmp(args[1], "&") != 0) || (argsNum == 3 && strcmp(args[2], "&") == 0)){
    _removeBackgroundSign(args[1]);
    if(strcmp(args[1], "kill") == 0){
        m_isKill = true;
    }
  }
  
  if(m_isKill){
    std::cout <<"smash: sending SIGKILL signal to " + std::to_string(m_jobs->getJobsListSize()) +
     " jobs:" << std::endl;
    int jobId = 0, jobsListSize = m_jobs->getJobsListSize();
    for(int i=0; i<jobsListSize; i++){
      JobsList::JobEntry* tempJob = m_jobs->getFirstJob(&jobId);
      if(tempJob == nullptr){
        break;
      }
      int result = kill(tempJob->m_pid, SIGKILL);
      if(result == -1){
        perror("smash error: kill failed");
      }else{
        std::cout << std::to_string(tempJob->m_pid) + ": " + tempJob->m_commandLine << std::endl;
        m_jobs->removeJobById(tempJob->m_jobId);
      }
    }
  }
  exit(1);
}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
}

void KillCommand::execute(){
  char* args[MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if(argsNum != 3){
    printInvalidArgumentsMessage("kill");
    return;
  }
  _removeBackgroundSign(args[2]);
  if(args[1][0] != '-' || !isNumberWithDash(args[1]) || !isNumber(args[2])){
    printInvalidArgumentsMessage("kill");
    return;
  }
  trimDash(args[1]);
  int signalNumber = std::stoi(args[1]);
  int jobIdToKill = std::stoi(args[2]);
  JobsList::JobEntry* jobToKill = m_jobs->getJobById(jobIdToKill);
  if(jobToKill == nullptr){
    printInvalidJobId("kill", args[2]);
    return;
  }
  int result = kill(jobToKill->m_pid, signalNumber);
  if(result == -1){
    perror("smash error: kill failed");
    return;
  }
  if(signalNumber == SIGSTOP){
    jobToKill->m_isStopped = true;
  } else if(signalNumber == SIGCONT){
    jobToKill->m_isStopped = false;
  }
  std::cout << "signal number " + std::to_string(signalNumber) + " was sent to pid " + std::to_string(jobToKill->m_pid) << std::endl;
}

//******************************************* EXTERNAL COMMANDS IMPLEMENTATION **************************************//

char** merge_arguments_arrays(char* str1, char* str2) {
  int len3 = 2;
  char** mergedArray = new char*[len3 + 1];
  mergedArray[0] = str1;
  mergedArray[1] = str2;
  mergedArray[2] = nullptr;
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
ExternalCommand::ExternalCommand(const char* cmd_line, JobsList* jobs): Command(cmd_line)
{
  m_timeout = -1;
  m_jobs = jobs;
  m_isBackground = _isBackgroundCommand(cmd_line);
  m_isComplex = is_complex_external_command(cmd_line);
  char* commandDup = (char*)malloc(strlen(cmd_line) + 1); //to remove the & sign if exists
  strcpy(commandDup, cmd_line);
  if(strlen(cmd_line) != 0){
    _removeBackgroundSign(commandDup);
  }
  char* tempArgs[MAX_ARGS] = {nullptr};
  int numArgs = _parseCommandLine(commandDup, tempArgs);
  if(strcmp(tempArgs[0], "timeout") == 0){
    if(!isNumber(tempArgs[FIRST_ARGUMENT])){
      assert(false); 
    } else {
      m_timeout = std::stoi(tempArgs[FIRST_ARGUMENT]);
      for(int i = 2; i < numArgs; i++)
      {
        m_command[i - 2] = tempArgs[i];
      }
    }
  }
  else {
    _parseCommandLine(commandDup, m_command);
  }
}

void ExternalCommand::execute()
{
  SmallShell& smashman = SmallShell::getInstance();
  if(!m_isComplex){
    if(m_isBackground){ //in case of background
      pid_t pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        if(execvp(m_command[0], m_command) == SYSCALL_FAILED){
            perror("smash error: execvp failed");  
        }
        exit(0);
      } else { //parent proccess
        m_jobs->addJob(this, pid);
        if(m_timeout != -1){
          smashman.m_childAlarm[pid] = std::make_tuple(m_commandLine, m_timeout, time(nullptr));
          alarm(m_timeout);
        }
        return;
      }
    } else { // in case of foreground
      int pid = fork();
      smashman.m_forgroundPid = pid;
      smashman.m_forgroundCmdLine = m_commandLine;
      if(pid == 0){ //child proccess
        setpgrp();
        if(execvp(m_command[0], m_command) == SYSCALL_FAILED){
            perror("smash error: execvp failed");  
        }
        exit(0);
      } else { //parent proccess
        if(m_timeout != -1){
          smashman.m_childAlarm[pid] = std::make_tuple(m_commandLine, m_timeout, time(nullptr));
          alarm(m_timeout);
        }
        if(waitpid(pid, nullptr, WUNTRACED) == SYSCALL_FAILED){
          perror("smash error: waitpid failed");  
        }
        smashman.m_forgroundPid = -1;
        smashman.m_forgroundCmdLine = "";
        return;
      }
    }
  } else { //in case of complex
    char* cmdLineCpy = new char[m_commandLine.size() + 1];
    strcpy(cmdLineCpy, m_commandLine.c_str());
    char* merged[] = {"bash", "-c", cmdLineCpy, nullptr};
    if(m_isBackground){ //in case of bachground
      int pid = fork();
      if(pid == 0){ //child proccess
        setpgrp();
        if(execvp("bash", merged) == SYSCALL_FAILED){
          perror("smash error: execvp failed");  
        }
        exit(0);
      } else { //parent proccess
        m_jobs->addJob(this, pid);
        if(m_timeout != -1){
          smashman.m_childAlarm[pid] = std::make_tuple(m_commandLine, m_timeout, time(nullptr));
          alarm(m_timeout);
        }
        return;
      }
    } else { // in case of foreground
      int pid = fork();
      smashman.m_forgroundPid = pid;
      smashman.m_forgroundCmdLine = m_commandLine;
      if(pid == 0){ //child proccess
        setpgrp();
        if(execvp("bash", merged) == SYSCALL_FAILED){
          perror("smash error: execvp failed");  
        }
        exit(0);
      } else { //parent proccess
        if(m_timeout != -1){
          smashman.m_childAlarm[pid] = std::make_tuple(m_commandLine, m_timeout, time(nullptr));
          alarm(m_timeout);
        }
        if(waitpid(pid, nullptr, WUNTRACED) == SYSCALL_FAILED){
          perror("smash error: waitpid failed");  
        }
        smashman.m_forgroundPid = -1;
        smashman.m_forgroundCmdLine = "";
        return;
      }
    }
  }
}

//******************************************* SPECIAL COMMAND IMPLEMENTATION **************************************//


RedirectionCommand::RedirectionCommand(const char* cmd_line): Command(cmd_line)
{
  char* cmdLineCpy = new char[m_commandLine.size() + 1];
  strcpy(cmdLineCpy, m_commandLine.c_str());
  _removeBackgroundSign(cmdLineCpy);
  // m_argsNum = _parseCommandLine(cmdLineCpy, m_args);
  // m_outputPath = m_args[m_argsNum - 1];
  // if(std::strcmp(m_args[m_argsNum - 2], ">") == 0){
  //   m_toOverride = true;
  // } else {
  //   m_toOverride = false;
  //   assert(std::strcmp(m_args[m_argsNum - 2], ">>") == 0);
  // }
  // strcpy(m_innerCommand, cmd_line);
  // cutUpToChar(m_innerCommand, '>');

  char command[MAX_LINE_LEN];
  char file[MAX_LINE_LEN];
  char redirectSymbol[MAX_LINE_LEN];
  if(std::strstr(cmdLineCpy, ">>") != nullptr){
    strcpy(redirectSymbol, ">>");
    m_toOverride = false;
  }else{
    strcpy(redirectSymbol, ">");
    m_toOverride = true;
  }
  splitString(cmdLineCpy, command, file, redirectSymbol);
  strcpy(m_innerCommand, command);
  m_argsNum = _parseCommandLine(command, m_args);
  assert(_parseCommandLine(file, m_outputPath) == 1);
}


/*
gets a string and a char
return a substr, from the original cut up to the first appearance of the char
*/
char* cutUpToChar(char* str, char ch) {
    char* ptr = std::strchr(str, ch); // Find pointer to ch
    if (ptr != nullptr) {
        *ptr = '\0';
    }
    return str;
}

bool is_builtIn_command(char* command)
{
  std::string commandDup = command;
  if (commandDup == "showpid" || commandDup == "chprompt" || commandDup == "pwd" || commandDup == "cd" ||
       commandDup == "jobs" || commandDup == "fg" || commandDup == "bg" || commandDup == "quit" || commandDup == "kill"){
    return true;
  }
  return false;
}

void RedirectionCommand::execute()
{
  SmallShell& smashy = SmallShell::getInstance();
  int stdoutDup = dup(1);
  if (is_builtIn_command(m_args[0]))
  {
    if(m_toOverride){
      close(1);
      int fd = open(m_outputPath[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(fd == SYSCALL_FAILED){
        perror("smash error: open fails");
        return;
      }
      smashy.executeCommand(m_innerCommand);
      close(1);
      int fd1 = dup(stdoutDup);
      close(stdoutDup);
      if (fd1 == -1){ 
        perror("smash error: open fails");
        return;
      }
      return;
    } else {  // not fork and not override
      close(1);
      int fd = open(m_outputPath[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
      if(fd == SYSCALL_FAILED){
        perror("smash error: open fails");
        return;
      }
      smashy.executeCommand(m_innerCommand);
      close(1);
      int fd1 = dup(stdoutDup);
      close(stdoutDup);
      if (fd1 == -1){ 
        perror("smash error: open fails");
      }
      return;
    }
  }
  pid_t pid = fork();
  if(pid == SYSCALL_FAILED){
      perror("smash error: pid failed");
      return;
  }
  if(m_toOverride)
  {
    if(pid == 0){
      setpgrp();
      close(1);
      int fd = open(m_outputPath[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(fd == SYSCALL_FAILED){
        perror("smash error: open fails");
        return;
      }
      smashy.executeCommand(m_innerCommand);
      exit(0);
    }else{
      waitpid(pid, nullptr, 0);
      return;
    }
  } else {
    if(pid == 0){
    close(1);
    int fd = open(m_outputPath[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == SYSCALL_FAILED){
      perror(":smash error: open fails");
      return;
    }
    smashy.executeCommand(m_innerCommand);
    exit(0);
    }else{
      waitpid(pid, nullptr, 0);
      return;
    }
  }
}

void splitString(const char* str1, char* str2, char* str3, const char* symbol) {
    // Find the position of the delimiter "|&"
    const char* delimiter = std::strstr(str1, symbol);
    if (delimiter == nullptr) {
        // The delimiter was not found, set both output strings to empty
        std::strcpy(str2, "");
        std::strcpy(str3, "");
        assert(false);
    } else {
        // Copy the first part of the input string up to the delimiter to str2
        std::size_t delimiterPos = delimiter - str1;
        std::strncpy(str2, str1, delimiterPos);
        str2[delimiterPos] = '\0';

        // Copy the second part of the input string after the delimiter to str3
        std::strcpy(str3, delimiter + strlen(symbol));
    }
}

PipeCommand::PipeCommand(const char* cmd_line): Command(cmd_line)
{
  char* cmdLineCpy = new char[m_commandLine.size() + 1];
  strcpy(cmdLineCpy, m_commandLine.c_str());
  _removeBackgroundSign(cmdLineCpy);
  if (std::strstr(cmdLineCpy, "|&") == nullptr) //then it consist only |
  {
    m_directStdErr = false;
    splitString(cmdLineCpy, m_firstCommand, m_secondCommand, "|");
  } else {
    m_directStdErr = true;
    splitString(cmdLineCpy, m_firstCommand, m_secondCommand, "|&");
  }
}

void PipeCommand::execute()
{
  SmallShell& smashush = SmallShell::getInstance();

  int pipefd[2];
  pid_t pid;

  if (pipe(pipefd) == SYSCALL_FAILED) {
      perror("smash error: pipe failed");
      return;
  }
  pid = fork();
  if (pid == SYSCALL_FAILED) {
      perror("smash error: fork failed");
      return;
  }
  if(pid != 0){
    //setpgrp();
    if(m_directStdErr){// DIRECT THE STD ERROR 
      int stdErrDup = dup(STDERR_FILENO);
      close(pipefd[0]); // close read end of pipe
      dup2(pipefd[1], STDERR_FILENO); // redirect stderr to pipe
      smashush.executeCommand(m_firstCommand); // execute command1
      dup2(stdErrDup, STDERR_FILENO); // redirect stdout to shell
      close(pipefd[1]); 
    } else { // DIRECT THE STD OUT
      int stdoutDup = dup(STDOUT_FILENO);
      close(pipefd[0]); // close read end of pipe
      dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
      smashush.executeCommand(m_firstCommand); // execute command1
      dup2(stdoutDup, STDOUT_FILENO); // redirect stdout to shell
      close(pipefd[1]);
    }
    waitpid(pid, nullptr, 0);
    //exit(0);
  } else {
      setpgrp();
      //waitpid(pid, nullptr, 0);
      close(pipefd[1]); // close write end of pipe
      int stdInDup = dup(STDIN_FILENO);
      dup2(pipefd[0], STDIN_FILENO); // redirect stdin from pipe
      smashush.executeCommand(m_secondCommand); // execute command2
      dup2(stdInDup, STDIN_FILENO);
      close(pipefd[0]);
      exit(0);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////


//get file info implementation
GetFileTypeCommand::GetFileTypeCommand(const char* cmd_line): BuiltInCommand(cmd_line)
{
  m_argsNum = _parseCommandLine(cmd_line, m_args);
}

void GetFileTypeCommand::execute()
{
  struct stat st;
    if (stat(m_args[FIRST_ARGUMENT], &st) == -1 || m_argsNum != 2) {
        perror("smash error: gettype: invalid arguments");
        return;
    }
    std::string type;
    std::string size = to_string(st.st_size);
    std::string path = m_args[FIRST_ARGUMENT];
    mode_t mode = st.st_mode;
    if (S_ISREG(mode)) {
        type =  "regular file";
    } else if (S_ISDIR(mode)) {
        type = "directory";
    } else if (S_ISLNK(mode)) {
        type = "symbolic link";
    } else if (S_ISFIFO(mode)) {
        type = "named pipe (FIFO)";
    } else if (S_ISSOCK(mode)) {
        type = "socket";
    } else if (S_ISCHR(mode)) {
        type = "character device";
    } else if (S_ISBLK(mode)) {
        type = "block device";
    } else {
        type = "unknown";
    }
    std::cout << path + "'s type is \"" + type + "\" and takes up " + size + " bytes" << std::endl;
}


SetcoreCommand::SetcoreCommand(const char* cmd_line, JobsList* jobs) :  BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
  m_jobs = jobs;
}

void SetcoreCommand::execute(){
  char* args[COMMAND_MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if(argsNum != 3 || (argsNum == 3 && (!isNumber(args[1]) || !isNumber(args[2])))){
    printInvalidArgumentsMessage("setcore");
    return;
  }
  int jobId = std::stoi(args[1]);
  int coreNum = std::stoi(args[2]);
  JobsList::JobEntry* job = m_jobs->getJobById(jobId);
  if(job == nullptr){
    printInvalidJobId("setcore", args[1]);
    return;
  }
  int numOfCores = sysconf(_SC_NPROCESSORS_ONLN); //get number of cores in cpu
  if(numOfCores == SYSCALL_FAILED){
    perror("smash error: sysconfig failed");
    return;
  }
  if(coreNum < 0 || coreNum >= numOfCores){
    printInvalidCoreNumMessage("setcore");
    return;
  }

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(coreNum, &cpuset);

  if(sched_setaffinity(job->m_pid, sizeof(cpu_set_t), &cpuset) == SYSCALL_FAILED){
    perror("smash error: sched_setaffinity failed");
    return;
  }
}

ChmodCommand::ChmodCommand(const char* cmd_line) : BuiltInCommand(cmd_line){
  m_cmdLine = new char[strlen(cmd_line) + 1];
  strcpy(m_cmdLine, cmd_line);
}

void ChmodCommand::execute(){
  char* args[COMMAND_MAX_ARGS] = {nullptr};
  int argsNum = _parseCommandLine(m_cmdLine, args);
  if(argsNum != 3 || !isNumber(args[1])){
    printInvalidArgumentsMessage("chmod");
    return;
  }
  if(std::stoi(args[1]) > 9999){
    printInvalidArgumentsMessage("chmod");
    return;
  }
  mode_t mode = std::stoi(args[1], 0, 8); // need the mode number in octal base
  char currentWorkingDir[MAX_LINE_LEN] = {0};
  if(getcwd(currentWorkingDir, sizeof(currentWorkingDir)) == nullptr){
    perror("smash error: getcwd failed");
    return;
  }
  std::string filePath;
  if(strstr(args[2], currentWorkingDir)!= nullptr){
    filePath = args[2];
  }else{
    filePath =  std::string(currentWorkingDir) + "/" + args[2];
  }
  if(chmod(filePath.c_str(), mode) == SYSCALL_FAILED){
    perror("smash error: chmod failed");
    return;
  }
}
