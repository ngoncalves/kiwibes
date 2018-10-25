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

  This class represents an event of the scheduler.
*/
#ifndef __KIWIBES_SCHEDULER_EVENT_H__
#define __KIWIBES_SCHEDULER_EVENT_H__

#include <memory>
#include <chrono>

/** Syntatic sugar for a time instant 
 */
typedef std::chrono::system_clock::time_point TimePoint_T;

/** Special macro that returns the current time
 */
#define TIME_NOW (new TimePoint_T(std::chrono::system_clock::now()))

/** Type of events possible 
 */
typedef enum { 
  START_JOB,        /* start a job */
  STOP_JOB,         /* stop a job */
  EXIT_SCHEDULER,   /* exit from the scheduler thread */
  SCHEDULE_JOB,     /* schedule the job with the given name */
} EventType_T;


class KiwibesSchedulerEvent {

public:
  /** Class constructor

    @param type     the type of event 
    @param t0       the instant when the event occurs
    @param payload  the event payload, can be nullptr
   */
  KiwibesSchedulerEvent(EventType_T type, TimePoint_T *t0, std::string *payload);    
  
  /** Class destructor
   */
  ~KiwibesSchedulerEvent();

 /** Ordering for events

  This operator is used to order events in the event queue.

  @param rhs  the event to compare against
  @return true if this event occurs first, false otherwise
  */
  bool operator<(const KiwibesSchedulerEvent &rhs);

public:
 EventType_T                  type;      /* type of event */
 std::unique_ptr<TimePoint_T> t0;        /* instant in the future when the event occurs */   
 std::unique_ptr<std::string> payload;   /* name of the job */   
};

#endif
