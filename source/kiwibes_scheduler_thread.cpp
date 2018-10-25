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
#include "kiwibes_scheduler_thread.h"
#include "NanoLog/NanoLog.hpp"

/*----------------- Private Functions ----------------------------*/

/** Schedule a job for periodic execution

  @param database   the database containing the job description
  @param event      the event and its payload
 */
static void schedule_job_event(KiwibesDatabase *database,const KiwibesSchedulerEvent *event)
{
  /* TODO */
}

/** Start the execution of a job

  @param database   the database containing the job description
  @param event      the event and its payload
 */
static void start_job_event(KiwibesDatabase *database,const KiwibesSchedulerEvent *event)
{
  /* TODO */
}

/** Stop the execution of a job

  @param database   the database containing the job description
  @param event      the event and its payload
 */
static void stop_job_event(KiwibesDatabase *database,const KiwibesSchedulerEvent *event)
{
  /* TODO */
}

/*----------------- Public Functions -----------------------------*/
void scheduler_thread_main(KiwibesDatabase *database, std::priority_queue<const KiwibesSchedulerEvent *> *events, std::mutex *qlock)
{
  /* run until the exit event is received */
  bool exit_received = false;

  while(!exit_received)
  {
    /* execute all events for which the occurrence time has passed */
    if(qlock->try_lock())
    {
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

      while(!exit_received     && 
            !(events->empty()) && 
            (now >= *(events->top()->t0))
           )
      {
        switch(events->top()->type)
        {
          case SCHEDULE_JOB:
            schedule_job_event(database,events->top());
            break ;

          case START_JOB:
            start_job_event(database,events->top());
            break ;

          case STOP_JOB:
            stop_job_event(database,events->top());
            break ;

          case EXIT_SCHEDULER:
            exit_received = true;
            break ;

          default:
            LOG_CRIT << "unknow event type: " << events->top()->type;
            break;
        }
        /* remove the event, since it is processed */
        delete events->top();
        events->pop();
      }
      /* unlock the access to the queue */
      qlock->unlock();
    }
  }
}