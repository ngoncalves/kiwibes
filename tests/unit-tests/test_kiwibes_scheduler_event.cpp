/* Kiwibes Automation Server Unit Tests
  =====================================
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
  Implements the unit tests for the events.  
 */
#include "unit_tests.h"
#include "kiwibes_scheduler_event.h"

#include <chrono>

/*----------------------- Public Functions Definitions ------------*/
void test_scheduler_event_order(void)
{
  std::time_t now = std::time_t(nullptr);

  KiwibesSchedulerEvent first(EVENT_START_JOB,now,std::string("first"));
  KiwibesSchedulerEvent second(EVENT_START_JOB,now + 1,std::string("second"));
  KiwibesSchedulerEvent third(EVENT_START_JOB,now + 2,std::string("third"));

  /* this order should be correct */
  ASSERT(first < second);
  ASSERT(second < third);
  ASSERT(first < third);
  
  /* this order is not correct */
  ASSERT(false == (third < first));
  ASSERT(false == (third < third));
}