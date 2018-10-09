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

  This class implements the HTTP server for the Kiwibes Automation Server.
*/
#ifndef __KIWIBES_HTTP_H__
#define __KIWIBES_HTTP_H__

#include <memory>
#include <string>

#include "kiwibes_scheduler.h"

class KiwibesHTTP {

public:
  /** Class constructor

    @scheduler  pointer to the scheduler object
   */
  KiwibesHTTP(KiwibesScheduler *scheduler);

  /** Class destructor
   */
  ~KiwibesHTTP();

  /* Run the server main loop in the current thread
   */
  void run();

private:
  KiwibesScheduler *scheduler;
};

#endif
