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

  This class implements the job scheduler, wich runs jobs periodically.
  It can also run jobs upon request, as well as stopping them at any
  point in time.
*/
#ifndef __KIWIBES_SCHEDULER_H__
#define __KIWIBES_SCHEDULER_H__

#include "kiwibes_database.h"
#include "kiwibes_jobs_manager.h"
#include "kiwibes_scheduler_event.h"
#include <queue>
#include <memory>
#include <thread>
#include <map>
#include <vector>

class KiwibesScheduler {

public:
  /** Class constructor

    @param  database    pointer to the database object
    @param  manager     pointer to the jobs manager
   */
  KiwibesScheduler(KiwibesDatabase *database, KiwibesJobsManager *manager);

  /** Class destructor
   */
  ~KiwibesScheduler();

  /** Start the scheduler thread
   */
  void start(void);

  /** Stop the scheduler task 
   */
  void stop(void);
  
  /** Schedule a job to run periodically

    @param name   name of the job
    @return ERROR_NO_ERROR if successfull, error code otherwise
   */
  T_KIWIBES_ERROR schedule_job(const std::string &name);

  /** Stop a job from running periodically.

    @param name   name of the job

    If the job is not scheduled to run, nothing is done.
   */
  void unschedule_job(const std::string &name);

  /** Return the list with all schedulled jobs
   */
  void get_all_scheduled_job_names(std::vector<std::string> &jobs); 

private:
  KiwibesDatabase              *database;                 /* private pointer to the database */
  KiwibesJobsManager           *manager;                  /* private pointer to the jobs manager */
  bool                         is_running;                /* set to true if the scheduler thread is running */
  std::mutex                   qlock;                     /* synchronize access to the event queue */
  std::unique_ptr<std::thread> scheduler;                 /* the scheduler thread */
  std::priority_queue<KiwibesSchedulerEvent *> events;    /* event queue */
};

#endif
