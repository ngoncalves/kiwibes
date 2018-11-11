/**
  Kiwibes Automation Server
  =========================
  Copyright 2018, Nelson Filipe Ferreira Goncalves
  nelsongoncalves@patois.eu

  License
  -------

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. You should have received
  a copy of the GNU General Public License along with this program.
  If not, see <http://www.gnu.org/licenses/>.

  Summary
  -------

  See the respective header file for details.
*/
#include "kiwibes_jobs_manager.h"
#include "NanoLog/NanoLog.hpp"

#include <cerrno>
#include <cstring>
#include <cstdlib>

#if defined(__linux__)
  #include <signal.h>
  #include <wait.h>
#endif 

/*----------------- Private Functions Declarations -----------------------------*/
/** Watcher Thread 

  This function waits for the processes in the map of active jobs to finish.

  @param database     pointer to the database object
  @param active_jobs  map of active jobs
  @param jobs_lock    access lock for the map of active jobs
  @param exitFlag     set to true when the thread should exit 
 */
static void watcher_thread(KiwibesDatabase *database,
                           std::map<std::string, T_JOB *> *active_jobs,
                           std::mutex *jobs_lock,
                           bool *exitFlag);

/*--------------- Class Implemementation --------------------------------------*/  
KiwibesJobsManager::KiwibesJobsManager(KiwibesDatabase *database)
{
  this->database = database;
  watcherExit    = false;

  /* start the watcher thread */
  watcher.reset(new std::thread(watcher_thread,database,&active_jobs,&jobs_lock,&watcherExit));
}

KiwibesJobsManager::~KiwibesJobsManager()
{
  /* stop all jobs and als othe watcher thread */
  stop_all_jobs();
  watcherExit = true; 
  watcher->join();
}

bool KiwibesJobsManager::start_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  bool fail = false;

  if(1 == active_jobs.count(name))
  {
    LOG_INFO << "Job '" << name << "' is already running, not starting it";
    fail = true;
  }
  else
  {   
    nlohmann::json description = database->get_job(name);

    if(nullptr == description)
    {
      LOG_INFO << "No job with name '" << name << "' was found in the database";  
      fail = true;
    }
    else
    {
      T_JOB *job = launch_job(description);

      if(NULL != job)
      {
        active_jobs.insert(std::pair<std::string,T_JOB* >(name,job));
        database->job_started(name);
      }    
      else
      {
        fail = true;
      }
    } 
  }

  return !fail;
}
  
bool KiwibesJobsManager::stop_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  bool fail = false;
  std::map<std::string,T_JOB *>::iterator iter = active_jobs.find(name);

  if(active_jobs.end() == iter)
  {
    LOG_INFO << "Job '" << name << "' is not running, not stopping it";
    fail = true;
  }
  else
  {
#if defined(__linux__)
    /* kill the child process and let the watcher thread to handle its exit */
    kill((*iter).second->handle,SIGKILL);
#endif    
  }

  return !fail;
}

void KiwibesJobsManager::stop_all_jobs(void)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  for(std::map<std::string,T_JOB *>::iterator iter = active_jobs.begin(); iter != active_jobs.end(); iter++)
  {
#if defined(__linux__)
    /* kill the child process and let the watcher thread to handle its exit */
    kill((*iter).second->handle,SIGKILL);
#endif        
  }
}

T_JOB *KiwibesJobsManager::launch_job(const nlohmann::json &description)
{
  T_JOB *job = new T_JOB;

#if defined(__linux__)
  job->handle = fork();
  if(0 == job->handle)
  {
    /* child process, start the process with the given command line */
    std::vector<std::string> command = description["command"].get<std::vector<std::string> >();

    char **arguments = (char **)malloc(sizeof(char *)*(1 + command.size()));
    for(unsigned int a = 0; a < command.size(); a++)
    {
      arguments[a] = (char *)command[a].c_str();
    }
    arguments[command.size()] = NULL;

    execv(command[0].c_str(),(char *const *)arguments);

    /* should not reach here */
    return NULL;
  }
  else if(0 < job->handle)
  {
    /* parent process, begin monitoring the job */
    job->started = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  }
  else    
  {
    /* an error occurred */
    LOG_CRIT << "Failed to fork new process(" << errno << "): "<< strerror(errno);
    delete job; 
    job = NULL;
  }
#endif 

  return job; 
}

/*------------------ Private Functions Definitions ----------------------*/
static void watcher_thread(KiwibesDatabase *database, std::map<std::string, T_JOB *> *active_jobs, std::mutex *jobs_lock, bool *exitFlag)
{
  while(false == *exitFlag)
  {
    /* wait a little before attempting to get the lock */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    /* check if any of the processes has exited, an if so update the database
       information and remove it from the map of active jobs 
     */
    jobs_lock->lock();

#if defined(__linux__)
    int               wstatus = 0;
    T_PROCESS_HANDLER pid     = waitpid(-1,&wstatus,WNOHANG);

    while(0 < pid)
    {
      if(WIFEXITED(wstatus) || WIFSIGNALED(wstatus))
      {
        for(std::map<std::string, T_JOB *>::iterator iter = active_jobs->begin(); iter != active_jobs->end(); iter++)
        {
          if(pid == (*iter).second->handle)
          {
            std::time_t runtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (*iter).second->started;

            /* notify the database that the job has finished and then remove 
              the job from the map of active jobs
             */
            database->job_stopped((*iter).first,runtime);
            active_jobs->erase(iter);
            break;
          }
        }
      }

      /* next job */
      pid = waitpid(-1,&wstatus,WNOHANG);
    }
#endif 

    jobs_lock->unlock();    
  }
}
