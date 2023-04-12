#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define SIGKILL 9
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

std::vector<std::string> get_args_in_vec(const char *cmd_line)
{
    std::vector<std::string> res;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;)
        res.push_back(s);
    return res;
}

bool _isSimpleExternal(std::string str)
{
    return str.find_first_of("*?") == std::string::npos;
}

//<---------------------------C'tors and D'tors--------------------------->

// ChangeDirCommand
ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand::BuiltInCommand(cmd_line)
{
    if (args_vec.size() != 2)
    {
        // לזרוק שגיאה על כמות לא נכונה של ארגומנטים או שלא נוספו ארגומנטים
        // throw error that "smash error: cd: too many arguments"
    }

    // העברתי את התנאי לתוך execute כי הוא מתקיים רק עם ארגומנט "-"
    //    if ((*plastPwd) == nullptr)
    //    {
    //        // לזרוק חריגה שתיקיה אחרונה חוקית לא קיימת עדיין
    //        // throw error that "smash error: cd: OLDPWD not set"
    //    }
};

// Small Shell
SmallShell::SmallShell() : prompt("smash"), last_wd(""), current_process(nullptr), jobs_list()
{
}

// Command

Command::Command(const char *cmd_line) : job_id(-1), process_id(getpid()), cmd_l(cmd_line), is_finished(false)
{
    args_vec = get_args_in_vec(cmd_l);
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

int JobsList::JobEntry::getJobId() const
{
    return command->getJobId();
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
            setpgrp();
            cmd->execute();
        }
        else
        {
            current_process = cmd;
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
    SmallShell &smash = SmallShell::getInstance();
    std::string new_dir;
    if (args_vec.size() != 2)
    {
        // לזרוק שגיאה שאין ארגונטים
    }
    if (args_vec[1] == "-")
    {
        // if the last working directory doesn't exist
        if (smash.get_last_wd().empty())
        {
            // לזרוק שגיאה שתיקיה אחרונה חוקית עוד לא הייתה
        }
        else
        {
            new_dir = smash.get_last_wd();
        }
    }
    else
    {
        new_dir = args_vec[1];
    }

    char buffer[256];
    if (getcwd(buffer, sizeof(buffer)) == NULL)
    {
        // perror("smash error: getcwd failed");
        // לזרוק חריגה שקריאת מערכת נכשלה
    }

    int res = chdir(new_dir.c_str());
    // if change fails
    if (res == -1)
    {
        // perror("smash error: chdir failed");
        // לזרוק חריגה שקריאת מערכת נכשלה
    }
    // if change succeeds
    else
    {
        smash.set_last_wd(string(buffer));
    }
    is_finished = true;
}

void ExternalCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();

    this->process_id = getpid();
    if (_isBackgroundCommand(cmd_l))
    {
        smash.addJob(this);
    }

    if (_isSimpleExternal(cmd_l))
    {
        char **args = new char *[args_vec.size()];
        _parseCommandLine(cmd_l, args);
        if (execv(args[0], args) == -1)
        {
            smash.removeJob(this->job_id);
            // ולזרוק שגיאה שפעולת מערכת לא הצליחה
        }
    }
    else
    {
        // char **args= new char*[args_vec.size()];
        char *args[args_vec.size()];
        // creating an appropriate **args format for execv()
        _parseCommandLine(("-c " + string(cmd_l)).c_str(), args);

        if (execv("/bin/bash", args) == -1)
        {
            smash.removeJob(this->job_id);
            // ולזרוק שגיאה שפעולת מערכת לא הצליחה
        }
    }
    is_finished = true;
}

void JobsCommand::execute()
{
    jobs->removeFinishedJobs();
    jobs->printJobsList();
    is_finished = true;
}

void BackgroundCommand::execute()
{
    std::vector<std::string> args = get_args_in_vec(this->cmd_l);
    if (args.size() > 2 || args.size() < 1)
    {
        // לזרוק שגיאה שinvalid arguments
    }
    else if (args.size() == 2)
    {
        if (this->jobs != nullptr)
        {
            JobsList::JobEntry *job = this->jobs->getJobById(args[1]);
            if (job != nullptr)
            {
                if (job->getStopped())
                {
                    int pid = job->getCommand()->getProcessId();
                    job->setStopped(false);
                    std::cout << job->getCommand()->getCmdL() << std::endl;
                    kill(pid, SIGCONT);
                }
                else
                {
                    // להדפיס שגיאה שהעבודה הזאת כבר רצה ברקע
                    // smash error: bg: job-id <job-id> is already running in the background
                }
            }
            else
            {
                // לזרוק שגיאה שלא קיימת כזאת עבודה:
                //  smash error: bg: job-id <job-id> does not exist
            }
        }
    }
    else
    {
        if (this->jobs != nullptr)
        {
            JobsList::JobEntry *job = this->jobs->getLastStoppedJob();
            if (job != nullptr)
            {
                int pid = job->getCommand()->getProcessId();
                job->setStopped(false);
                kill(pid, SIGCONT);
            }
            else
            {
                // לזרוק שגיאה שלא קיימת עבודה שנעצרה:
                //  smash error: bg: there is no stopped jobs to resume
            }
        }
    }
}

void ForegroundCommand::execute()
{
}

void QuitCommand::execute()
{
    bool flag_kill = false;
    std::vector<std::string> args = get_args_in_vec(this->cmd_l);
    for (int i = 1; i < args.size(); i++)
    {
        if (args[i] == "kill")
        {
            flag_kill = true;
            break;
        }
    }
    if (flag_kill)
    {
        jobs->killAllJobs();
    }
    exit(0);
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
    for (int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i]->getJobId() > max_id)
        {
            max_id = jobs[i]->getJobId();
        }
    }
    return max_id;
}

void JobsList::addJob(Command *cmd, bool isStopped = false)
{
    if (cmd->getJobId() == -1)
    {
        int job_id = getMaxId() + 1;
        command->setJobId(job_id);
        std::shared_ptr<JobEntry> new_job(new JobEntry(cmd, isStopped));
        jobs.push(new_job);
    }
    else
    {
        // was in the list before
        std::shared_ptr<JobEntry> new_job(new JobEntry(cmd, isStopped));
        for (int i = 0; i < jobs.size(); i++)
        {
            if (jobs[i]->getJobId() > cmd->getJobId())
            {
                jobs.insert(jobs.begin() + i, new_job);
                break;
            }
        }
    }
}

void JobsList::removeJobById(int jobId)
{
    for (int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i]->getJobId() == jobId)
        {
            jobs.erase(jobs.begin() + i);
            break;
        }
    }
}

JobEntry *JobsList::getJobById(int jobId)
{
    for (int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i]->getJobId() == jobId)
        {
            return jobs[i].get();
        }
    }
    return nullptr;
}

JobEntry *JobsList::getLastJob(int *lastJobId)
{
    return jobs.back().get();
}

JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
    int i = jobs.size() - 1;
    while (i >= 0)
    {
        if (jobs[i]->getStopped())
        {
            return jobs[i].get();
        }
        i++;
    }
    return nullptr;
}

void JobsList::printJobsList()
{
    for (int i = 0; i < jobs.size(); i++)
    {
        jobs[i]->printInfo();
    }
}
void JobsList::killAllJobs()
{
    this->removeFinishedJobs();
    std::cout << " sending SIGKILL signal to " << jobs.size() << " jobs:" << std::endl;
    for (int i = 0 li < jobs.size(); i++)
    {
        std::cout << jobs[i]->getCommand()->getProcessId() << ": " << jobs[i]->getCommand()->getCmdL() << std::endl;
    }
    for (int i = jobs.size() - 1; i >= 0; i--)
    {
        int pid = jobs[i]->getCommand()->getProcessId();
        int job_id = jobs[i]->getJobId();
        kill(pid, SIGKILL);
        this->removeJobById(job_id);
    }
}
void JobsList::removeFinishedJobs()
{
    std::vector<int> jobs_to_delete;
    for (int i = 0; jobs.size(); i++)
    {
        if (waitpid(jobs[i]->getCommand()->getProcessId(), nullptr, WNOHANG)) // האם זה גם משחרר את התהליך?
        {
            jobs_to_delete.push_back(jobs[i]->getJobId());
        }
    }
    for (int i = 0; i < jobs_to_delete.size(); i++)
    {
        int job_id_to_delete = jobs_to_delete.pop_back();

        this->removeJobById(job_id_to_delete);
    }
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
    else if (firstWord.compare("bg") == 0)
    {
        return new BackgroundCommand(cmd_line, &this->jobs_list);
    }
    else if (firstWord.compare("fg") == 0)
    {
        return new ForegroundCommand(cmd_line, &this->jobs_list);
    }
    else if (firstWord.compare("quit") == 0)
    {
        return new QuitCommand(cmd_line, &this->jobs_list);
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
    vector<string> args = get_args_in_vec(cmd_line); // להוסיף את הפונקציה שמחזירה וקטור מהcmd_line
    string new_prompt = args.size() == 1 ? "smash> " : (args[1] + "> ");
    prompt = new_prompt;
}

SmallShell::printPrompt()
{
    std::cout << prompt;
}

void SmallShell::addJob(Command *cmd, bool is_stopped)
{
    jobs_list->addJob(cmd, is_stopped);
};

void SmallShell::removeJob(int job_id)
{
    jobs_list->removeJobById(job_id);
};

//<--------------------------- Smash functions - end--------------------------->

//<--------------------------- Aux functions--------------------------->

bool isBuildInCommand(string firstWord)
{
    if (firstWord.compare("pwd") == 0 ||
        firstWord.compare("showpid") == 0 ||
        firstWord.compare("cd") == 0 ||
        firstWord.compare("jobs") == 0 ||
        firstWord.compare("bg") == 0 ||
        firstWord.compare("fg") == 0 ||
        firstWord.compare("quit") == 0) // להוסיף עוד
    {
        return true;
    }
    return false;
}

//<--------------------------- Aux functions - end--------------------------->
