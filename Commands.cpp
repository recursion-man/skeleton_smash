#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

//<---------------------------stuff functions--------------------------->

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

//<---------------------------stuff functions - end --------------------------->

//<---------------------------C'tors and D'tors--------------------------->

// ChangeDirCommand
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand::BuiltInCommand(cmd_line), args(func()),
                                                                            last_wd(plastPwd)
{
  if (args.size() != 2)
  {
    // לזרוק שגיאה על כמות לא נכונה של ארגומנטים או שלא נוספו ארגומנטים
    // throw error that "smash error: cd: too many arguments"
  }
  else if (*plastPwd == "")
  {
    // לזרוק חריגה שתיקיה אחרונה חוקית לא קיימת עדיין
    // throw error that "smash error: cd: OLDPWD not set"
  }
  else
  {
  }
};

// Small Shell
SmallShell::SmallShell() : prompt("smash"), last_wd(new char[256]), current_process_id(nullptr), jobs_list()
{
  last_wd = nullptr;
};

SmallShell::~SmallShell()
{
  delete last_wd;
};

//<---------------------------C'tors and D'tors - end--------------------------->

//<---------------------------getters--------------------------->
const char *Command::getCmdL() const
{
  return cmd_l;
}
bool Command::getStatus() const
{
  return is_finished;
}
int Command::getJobId() const
{
  return is_finished;
}
int Command::getProcessId() const
{
  return is_finished;
}

bool JobsList::JobEntry::isJobFinished() const
{
  return command->getStatus();
};
bool JobsList::JobEntry::getStopped() const
{
  return is_stopped;
};

Command *JobsList::JobEntry::getCommand() const
{
  return command;
};

//<---------------------------getters - end--------------------------->

//<---------------------------setters--------------------------->
void Command::setJobId(int id)
{
  job_id = id;
}
void Command::setProcessId(int id)
{
  process_id = id;
}

void JobsList::JobEntry::setTime()
{
  init_time = time(NULL);
};

void JobsList::JobEntry::setStopped(bool is_stopped)
{
  is_stopped = is_stopped;
};

//<---------------------------setters - end--------------------------->

//<---------------------------execute functions--------------------------->

void SmallShell::executeCommand(const char *cmd_line)
{
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  if (firstWord.compare("chprompt") == 0)
  {
    changeChprompt(cmd_line);
  }
  else if (isBuildInCommand(firstWord))
  {
    Command *cmd = CreateCommand(cmd_line);
    cmd->execute();
  }
  else
  {
    Command *cmd = CreateCommand(cmd_line);
    int pid = fork();
    if (pid == 0)
    {
      cmd->execute();
    }
    else
    {
      current_process_id = cmd;
      // חרא של האבא
    }
  }
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void ShowPidCommand::execute()
{
  int process_id = getpid();
  std::cout << "smash pid is " << process_id << std::endl;
  is_finished = true;
}

void GetCurrDirCommand::execute()
{
  char cwd[256];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    std::cout << cwd << std::endl;
  }
  else
  {
    // perror("smash error: getcwd failed");
    // לזרוק חריגה שקראת מערכת נכשלה
  }
  is_finished = true;
}

void ChangeDirCommand::execute()
{
  char *new_dir;
  if (args.size() != 2)
  {
    // לזרוק שגיאה שאין ארגונטים
  }
  if (args[1] == "-")
  {
    if (p_last_wd == nullptr)
    {
      // לזרוק שגיאה שתיקיה אחרונה חוקית עוד לא הייתה
    }
    else
    {
      new_dir = *p_last_wd;
    }
  }
  else
  {
    new_dir = args[1];
  }

  char buffer[256];
  if (getcwd(buffer, sizeof(buffer)) == NULL)
  {
    // perror("smash error: getcwd failed");
    // לזרוק חריגה שקריאת מערכת נכשלה
  }

  int res = chdir(new_dir);
  // change failed
  if (res == -1)
  {
    // perror("smash error: chdir failed");
    // לזרוק חריגה שקריאת מערכת נכשלה
  }
  // change succeeded
  else
  {
    strcpy(*p_last_wd, buffer);
  }
  is_finished = true;
}

void ExternalCommand::execute() override
{
  SmallShell &smash = SmallShell::getInstance();

  this->process_id = getpit();
  if (_isBackgroundCommand(cmd_line))
  {
    smash.addJob(this);
  }

  if (_isSimpleExternal(cmd)) // לממש פונקציה
  {
    char **args;
    _parseCommandLine(cmd, args);
    if (exec(args[0], args) == -1)
    {
      smash.removeJob(this->job_id);
      // ולזרוק שגיאה שפעולת מערכת לא הצליחה
    }
  }
  else
  {
    char *args = {"-c", cmd_line};
    if (exec("/bin/bash", args) == -1)
    {
      smash.removeJob(this->job_id);
      // ולזרוק שגיאה שפעולת מערכת לא הצליחה
    }
  }
  is_finishied = true;
}

void JobsCommand::execute()
{
  jobs->removeFinishedJobs();
  jobs->printJobsList();
  is_finished = true;
}

//<--------------------------- execute functions - end--------------------------->

//<--------------------------- Jobs List functions--------------------------->

void JobsList::JobEntry::printInfo() const
{
  int current_time = time(NULL);
  int time_diff = difftime(current_time, init_time);
  string stopped_str = is_stopped ? "(stopped)" : "";
  std::cout << "[" << command->getJobId() << "]" << command->getCmdL() << " : " << command->getProcessId() << " " << time_diff << " secs " << stopped_str << std::endl;
};

int JobsList::getMaxId() const
{
  int max_id = 0;
  for (iterator it = jobs.begin(); it != jobs.end(); ++it)
  {
    if (it->getCommand->getJobId() > max_id)
    {
      max_id = it->getCommand->getJobId();
    }
  }
  return max_id;
}

void JobsList::addJob(Command *cmd, bool isStopped = false)
{
  int job_id = getMaxId() + 1;
  command->setJobId(job_id);
  JobEntry new_job(cmd, isStopped);
  jobs.push(new_job);
}

void JobsList::removeJobById(int jobId)
{
}
//<--------------------------- Jobs List functions - end--------------------------->

//<--------------------------- Smash List functions--------------------------->

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  // For example:

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0)
  {
    return new ChangeDirCommand(cmd_line, &this->last_wd);
  }
  // להוסיף עוד
  else
  {
    return new ExternalCommand(cmd_line);
  }

  return nullptr;
}

void SmallShell::changeChprompt(const char *cmd_line)
{
  vector<string> args = func(); // להוסיף את הפונקציה שמחזירה וקטור מהcmd_line
  string new_prompt = args.size() == 1 ? "smash> " : (args[1] + "> ");
  prompt = new_prompt;
}

SmallShell::printPrompt()
{
  std::cout << prompt;
}

void SmallShell::addJob(Command *cmd, bool is_stopped = false)
{
  jobs_list->addJob(cmd, is_stopped);
};

void SmallShell::removeJob(int job_id)
{
  jobs_list->removeJobById(job_id);
};

//<--------------------------- Smash functions - end--------------------------->

//<--------------------------- Aux functions--------------------------->

bool _isSimpleExternal(const char *cmd_line)
{
  // לבצע פונקציה שבודקת האם יש כוכבית או סימן שאלה בשורת קוד
}

bool isBuildInCommand(string firstWord)
{
  if (firstWord.compare("pwd") == 0 ||
      firstWord.compare("showpid") == 0 ||
      firstWord.compare("cd") == 0 ||
      firstWord.compare("jobs") == 0 ||
      firstWord.compare("bg") == 0 ||
      firstWord.compare("fg") == 0) // להוסיף עוד
  {
    return true;
  }
  return false;
}

//<--------------------------- Aux functions - end--------------------------->
