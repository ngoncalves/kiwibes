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

  This module implements the thread on which scheduling events
  are handled.
*/
#ifndef __KIWIBES_SCHEDULER_THREAD_H__
#define __KIWIBES_SCHEDULER_THEAD_H__

#include "kiwibes_database.h"
#include "kiwibes_scheduler_event.h"
#include <queue>
#include <mutex>

/** Scheduler Thread

  This function implements the scheduler thread, which manages
  the execution of the jobs. The function will run in a non-stop
  loop until the exit event is received

  @param database   pointer to the database object 
  @param events     events queue
  @param qlock      the events queue lock 
 */
void scheduler_thread_main(KiwibesDatabase *database, std::priority_queue<const KiwibesSchedulerEvent *> *events, std::mutex *qlock);

#endif
