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
#include "kiwibes_scheduler_event.h"
#include "NanoLog/NanoLog.hpp"

KiwibesSchedulerEvent::KiwibesSchedulerEvent(T_EVENT_TYPE type, std::time_t t0, const std::string &job_name)
{
  this->type = type;
  this->t0   = t0;
  this->job_name = new std::string(job_name);
}

KiwibesSchedulerEvent::~KiwibesSchedulerEvent()
{
  delete job_name;
}

bool KiwibesSchedulerEvent::operator<(const KiwibesSchedulerEvent &rhs)
{
  return (t0 < rhs.t0); 
}
