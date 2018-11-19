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
#include "kiwibes_scheduler.h"
#include "kiwibes_cron.h"
#include "NanoLog/NanoLog.hpp"
#include <chrono>
#include <vector>

/*----------------- Private Functions Declarations -----------------------------*/
/** Scheduler Thread 

  This function implements the scheduler thread, which manages
  the execution of scheduled jobs. The function will run in a non-stop
  loop until the exit event is received

  @param scheduler  pointer to the scheduler
  @param manager    pointer to the jobs manager
  @param qlock      the events queue lock   
  @param events     events queue
 */
static void scheduler_thread(KiwibesScheduler   *scheduler,
                             KiwibesJobsManager *manager,
                             std::mutex         *qlock,
                             std::priority_queue<KiwibesSchedulerEvent *> *events);

/*--------------------- Modified Piority Queue -------------------------------*/
/** Returns the underlying container of the priority queue
    Based on the solution from here: https://stackoverflow.com/a/12886393
 */    
template <class T, class S, class C> S& Container(std::priority_queue<T, S, C>& q)
{
  struct HackedQueue : private std::priority_queue<T, S, C>
  {
    static S& Container(std::priority_queue<T, S, C>& q)
    {
      return q.*&HackedQueue::c;
    }
  };
  
  return HackedQueue::Container(q);
}


/*--------------- Class Implemementation --------------------------------------*/  
KiwibesScheduler::KiwibesScheduler(KiwibesDatabase *database, KiwibesJobsManager *manager)
{
  this->database = database;
  this->manager  = manager;
  scheduler.reset(nullptr);
  is_running = false;
}

KiwibesScheduler::~KiwibesScheduler()
{
  stop();
  
  while(!events.empty())
  {
    delete events.top();
    events.pop();
  }
}

void KiwibesScheduler::start(void)
{
  LOG_INFO << "starting the scheduler thread";
  scheduler.reset(new std::thread(scheduler_thread,this,manager,&qlock,&events));
  is_running = true;
}

void KiwibesScheduler::stop(void)
{
  if(true == is_running)
  {
    /* push the exit event to the queue, then wait for the thread to stop */
    {
      std::lock_guard<std::mutex> lock(qlock);
    
      std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      
      events.push(new KiwibesSchedulerEvent(EVENT_EXIT_SCHEDULER,now,std::string("")));
    }

    LOG_INFO << "waiting for the scheduler thread to finish";
    scheduler->join();
    is_running = false;
    LOG_INFO << "scheduler thread has finished";    
  }
  else
  {
    LOG_INFO << "scheduler thread is not running, no need to stop it";     
  }
}

T_KIWIBES_ERROR KiwibesScheduler::schedule_job(const std::string &name)
{
  nlohmann::json  job;
  T_KIWIBES_ERROR error = database->get_job_description(job,name);

  if(ERROR_NO_ERROR != error)
  {
    LOG_CRIT << "cannot find a job with name '" << name << "'";
  } 
  else
  {
    KiwibesCron cron(job["schedule"].get<std::string>());

    if(false == cron.is_valid())
    {
      LOG_CRIT << "job '" << name << "' has an invalid schedule";
      error = ERROR_JOB_SCHEDULE_INVALID;
    }
    else
    {
      std::lock_guard<std::mutex> lock(qlock);

      events.push(new KiwibesSchedulerEvent(EVENT_START_JOB,cron.next(),name));

      LOG_INFO << "scheduled job '" << name << "'";      
    }
  }

  return error; 
}

void KiwibesScheduler::unschedule_job(const std::string &name)
{
  /* go through the scheduled jobs and change the event type of
     the events which match the job name. We do not change the
     occurrence time because that would require a re-ordering
     of the events in the priory queue
   */
  std::lock_guard<std::mutex> lock(qlock);

  std::vector<KiwibesSchedulerEvent *> &vEvents = Container(events);

  for(unsigned int e = 0; e < vEvents.size(); e++)
  {
    if(0 == name.compare(*(vEvents[e]->job_name)))
    {
      vEvents[e]->type = EVENT_STOP_JOB;
    }
  }

  LOG_INFO << "unscheduled job '" << name << "'";        
}

/*--------------------- Private Functions Definitions ------------------------------*/
static void scheduler_thread(KiwibesScheduler *scheduler, KiwibesJobsManager *manager,std::mutex *qlock,std::priority_queue<KiwibesSchedulerEvent *> *events)
{
  /* run in an infinite loop until the exit event is received */
  bool exit_event_received = false;

  while(false == exit_event_received)
  {
    /* verify if the first waiting event has occurred */
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    qlock->lock();

    while((false == exit_event_received) && !(events->empty()) && (now >= events->top()->t0))
    {
      KiwibesSchedulerEvent *event = events->top();

      switch(event->type)
      {
        case EVENT_START_JOB:
          /* start the job, then re-schedule it again */
          {
            manager->start_job(*(event->job_name));
            scheduler->schedule_job(*(event->job_name));
          }
          break;

        case EVENT_STOP_JOB:
          /* nothing to do, simply ignore it */
          LOG_INFO << "not re-scheduling job '" << *(event->job_name) << "'";
          break;

        case EVENT_EXIT_SCHEDULER:
          exit_event_received = true;
          break;

        default:
          LOG_CRIT << "ignoring unknown event type: " << event->type;
          break;
      }

      events->pop();
      delete event;
    }

    qlock->unlock();

    /* snooze a little to give the main thread a change at inserting events in the queue */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}