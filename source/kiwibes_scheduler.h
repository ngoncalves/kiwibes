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

#include "nlohmann/json.h"
#include "kiwibes_database.h"
#include "kiwibes_scheduler_event.h"
#include <queue>
#include <memory>
#include <thread>

class KiwibesScheduler {

public:
  /** Class constructor
   */
  KiwibesScheduler();

  /** Class destructor
   */
  ~KiwibesScheduler();

  /** Start the scheduler thread

    @param database   pointer to the database object
   */
  void start(KiwibesDatabase *database);

  /** Stop the scheduler task 
   */
  void stop(void);

  /** Return true if the scheduler thread is running
   */
  bool is_thread_running(void);
  
  /** Push an event onto the event queue

    @param event  the event to push
   */
  void push(const KiwibesSchedulerEvent *event);

private:
  bool                         is_running;                    /* set to true if the scheduler thread is running */
  std::mutex                   qlock;                         /* synchronize access to the event queue */
  std::unique_ptr<std::thread> scheduler;                     /* the scheduler thread */
  std::priority_queue<const KiwibesSchedulerEvent *> events;  /* event queue */
};

#endif
