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

  This class implements the jobs manager, responsible for starting
  and stopping jobs.
*/
#ifndef __KIWIBES_JOBS_MANAGER_H__
#define __KIWIBES_JOBS_MANAGER_H__

#include "kiwibes_database.h"
#include "kiwibes_errors.h"

#include "nlohmann/json.h"

#include <map>
#include <string>
#include <mutex>
#include <thread>

#if defined(__linux__)
  #include <sys/types.h>
  #include <unistd.h>

  typedef pid_t T_PROCESS_HANDLER;
  #define INVALID_PROCESS_HANDLE (-1)
#else
  #error "OS not supported"
#endif 

class KiwibesJobsManager {

public:
  /** Class constructor

    @param database   pointer to the database object
   */
  KiwibesJobsManager(KiwibesDatabase *database);

  /** Class destructor
   */
  ~KiwibesJobsManager();

  /** Start the job with the given name

    @param name   name of the job to start
    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR start_job(const std::string &name);
  
  /** Stop the job with the given name

    @param name   name of the job to stop
    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR stop_job(const std::string &name);

  /** Stop all of the currently running jobs
   */
  void stop_all_jobs(void);

private:
  KiwibesDatabase                          *database;    /* private pointer to the database */
  std::map<std::string, T_PROCESS_HANDLER> active_jobs;  /* active jobs */
  std::mutex                               jobs_lock;    /* exclusive access to the list of running jobs */
  std::unique_ptr<std::thread>             watcher;      /* thread that waits for child processes to exit */
  bool                                     watcherExit;  /* flag to indicate when the watcher thread should exit */
};  

#endif
