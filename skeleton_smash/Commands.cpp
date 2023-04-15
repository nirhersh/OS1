#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#include <assert.h>

using namespace std;
#define MAX_LINE_LEN 80
#define SYSCALL_FAILED -1
#define DEFAULT_SHELL_PROMPT "smash> "
#define FIRST_ARGUMENT 1

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

bool _isBackgroundComamnd(const char* cmd_line) {
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

//******************************************* SMALL SHELL IMPLEMENTATION **************************************//


SmallShell::SmallShell() {
// TODO: add your implementation
  set_lastDir("");
  m_shellPrompt = "smash> ";
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
  //add ifs for every command
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

void SmallShell::set_lastDir(const std::string newDir)
{
  m_lastDir = newDir;
  //std::cout << "entered setter, new path is " << m_lastDir << std::endl;
}

void SmallShell::set_shellPrompt(const std::string newPrompt)
{
  m_shellPrompt = newPrompt;
}


//******************************************* HELPFULL FUNCTIONS **************************************//



//******************************************* BUILT-IN COMMANDS IMPLEMENTATION **************************************
//Command::~Command() {}
Command::Command(const char* cmd_line){}
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
  char** args;
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
  char** example1;
  int argsNum = _parseCommandLine(m_cmdLine, example1);
  std::cout << "1111111111" << std::endl;
  SmallShell& shell = SmallShell::getInstance();
  if(argsNum == 1)
  {
    shell.set_shellPrompt(DEFAULT_SHELL_PROMPT);
  } else {
    shell.set_shellPrompt(example1[FIRST_ARGUMENT]);
  }
}

