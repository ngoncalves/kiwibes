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
#include "kiwibes_scheduler_thread.h"
#include "NanoLog/NanoLog.hpp"

KiwibesScheduler::KiwibesScheduler()
{
  scheduler.reset(nullptr);
  is_running = false;
}

KiwibesScheduler::~KiwibesScheduler()
{
  while(!events.empty())
  {
    delete events.top();
    events.pop();
  }
}

void KiwibesScheduler::start(KiwibesDatabase *database)
{
  scheduler.reset(new std::thread(scheduler_thread_main,database,&events,&qlock));
  is_running = true;
}

void KiwibesScheduler::stop(void)
{
  /* sent the exit event, then wait for the thread to exit */
  this->push(new KiwibesSchedulerEvent(EXIT_SCHEDULER,
                                       TIME_NOW,
                                       nullptr)
            );  

  LOG_INFO << "waiting for the scheduler thread to exit";

  scheduler->join();
  is_running = false;
}

void KiwibesScheduler::push(const KiwibesSchedulerEvent *event)
{
  std::lock_guard<std::mutex> lock(qlock);
  events.push(event);
}

bool KiwibesScheduler::is_thread_running(void)
{
  return is_running;
}