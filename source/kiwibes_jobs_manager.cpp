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

#include <vector>
#include <string>
#include <chrono>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#if defined(__linux__)
  #include <signal.h>
  #include <wait.h>
#endif 

/*----------------- Private Functions Declarations -----------------------------*/
/** Launch the job in a separate process

  @param job    the job description
  @return the new process handle
 */
static T_PROCESS_HANDLER launch_job_process(nlohmann::json &job);

/** Watcher Thread 

  This function waits for the processes in the map of active jobs to finish.

  @param database     pointer to the database object
  @param active_jobs  map of active jobs
  @param jobs_lock    access lock for the map of active jobs
  @param exitFlag     set to true when the thread should exit 
 */
static void watcher_thread(KiwibesDatabase *database,
                           std::map<std::string, T_PROCESS_HANDLER> *active_jobs,
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
  /* stop all jobs and also the watcher thread */
  stop_all_jobs();

  watcherExit = true;
  LOG_INFO << "waiting for the watcher thread to finish"; 
  watcher->join();
  LOG_INFO << "the watcher thread has finished";
}

T_KIWIBES_ERROR KiwibesJobsManager::start_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;
  
  if(1 == active_jobs.count(name))
  {
    LOG_INFO << "Job '" << name << "' is already running, queueing it";
    error = database->job_incr_start_requests(name);
  }
  else
  {
    nlohmann::json job;
    error = database->get_job_description(job,name);

    if(ERROR_NO_ERROR != error)
    {
      LOG_WARN << "No job with name '" << name << "' was found in the database";  
    }
    else
    {
      T_PROCESS_HANDLER handle = launch_job_process(job);

      if(INVALID_PROCESS_HANDLE != handle)
      {
        active_jobs.insert(std::pair<std::string,T_PROCESS_HANDLER>(name,handle));
        database->job_started(name);
        LOG_INFO << "Started job '" << name << "'";
      }    
      else
      {
        LOG_CRIT << "Failed to launch process for job '" << name << "'";  
        error = ERROR_PROCESS_LAUNCH_FAILED;
      }
    }
  }

  return error;
}

T_KIWIBES_ERROR KiwibesJobsManager::stop_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;
  nlohmann::json  job;

  error = database->get_job_description(job,name);

  if(ERROR_NO_ERROR != error)
  {
      LOG_WARN << "No job with name '" << name << "' was found in the database";  
  }
  else
  {
    std::map<std::string,T_PROCESS_HANDLER>::iterator iter = active_jobs.find(name);

    if(active_jobs.end() == iter)
    {
      LOG_WARN << "Job '" << name << "' is not running, not stopping it";
      error = ERROR_JOB_IS_NOT_RUNNING;
    }
    else
    {
#if defined(__linux__)
      /* kill the child process and let the watcher thread to handle its exit */
      LOG_INFO << "Killing process for job '" << name << "'";
      kill((*iter).second,SIGKILL);
#endif    
    }
  }

  return error;
}

void KiwibesJobsManager::stop_all_jobs(void)
{
  std::lock_guard<std::mutex> lock(jobs_lock);

  for(std::map<std::string,T_PROCESS_HANDLER>::iterator iter = active_jobs.begin(); iter != active_jobs.end(); iter++)
  {
#if defined(__linux__)
    /* kill the child process and let the watcher thread to handle its exit */
    LOG_INFO << "Killing process for job '" << (*iter).first << "'";
    kill((*iter).second,SIGKILL);
#endif        
  }
}

/*------------------ Private Functions Definitions ----------------------*/
static T_PROCESS_HANDLER launch_job_process(nlohmann::json &job)
{
  T_PROCESS_HANDLER handle = INVALID_PROCESS_HANDLE;

#if defined(__linux__)
  handle = fork();
  
  if(0 == handle)
  {
    /* child process, start the process with the given command line */
    std::vector<std::string> program(job["program"].get<std::vector<std::string> >());

    char **arguments = (char **)malloc(sizeof(char *)*(1 + program.size()));
    for(unsigned int a = 0; a < program.size(); a++)
    {
      arguments[a] = (char *)program[a].c_str();
    }
    arguments[program.size()] = NULL;

    execv(program[0].c_str(),(char *const *)arguments);

    /* should not reach here */
    return INVALID_PROCESS_HANDLE;
  }
  else if(0 > handle)    
  {
    /* an error occurred */
    LOG_CRIT << "Failed to fork new process(" << errno << "): "<< strerror(errno);
  }
#endif 

  return handle; 
}

static void watcher_thread(KiwibesDatabase *database, std::map<std::string, T_PROCESS_HANDLER> *active_jobs, std::mutex *jobs_lock, bool *exitFlag)
{
  while(false == *exitFlag)
  {
    /* wait a little before attempting to get the lock */
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

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
        for(std::map<std::string, T_PROCESS_HANDLER>::iterator iter = active_jobs->begin(); iter != active_jobs->end(); iter++)
        {
          if(pid == (*iter).second)
          {
            /* notify the database that the job has finished and then remove 
               the job from the map of active jobs
             */
            std::string name = std::string((*iter).first);

            database->job_stopped(name);
            active_jobs->erase(iter);

            /* if there are queued start requests for this job, run it again */
            if(0 <= database->job_decr_start_requests(name))
            {
              LOG_INFO << "Job '" << name << "' has pending start requests, starting it again";

              nlohmann::json job;
              if(ERROR_NO_ERROR == database->get_job_description(job,name))
              {
                T_PROCESS_HANDLER handle = launch_job_process(job);

                if(INVALID_PROCESS_HANDLE != handle)
                {
                  active_jobs->insert(std::pair<std::string,T_PROCESS_HANDLER>(name,handle));
                  database->job_started(name);
                  LOG_INFO << "Started job '" << name << "'";
                }    
                else
                {
                  LOG_CRIT << "Failed to launch process for job '" << name << "'";  
                }
              }
            }
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
