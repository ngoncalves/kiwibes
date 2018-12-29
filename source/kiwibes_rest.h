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
  
  This class implements the REST interface.
*/
#ifndef __KIWIBES_REST_H__
#define __KIWIBES_REST_H__

#include "kiwibes_database.h"
#include "kiwibes_data_store.h"
#include "kiwibes_jobs_manager.h"
#include "kiwibes_scheduler.h"
#include "kiwibes_authentication.h"

#include "cpp-httplib/httplib.h"

/*-------------------------- Public Function Declarations -------------------------------*/

/** Setup the REST HTTP route handlers
 
  @param httpS          pointer to the HTTPS server
  @param manager        pointer to the Kiwibes jobs manager
  @param scheduler      pointer to the Kiwibes jobs scheduler
  @param database       pointer to the Kiwibes database interface 
  @param database       pointer to the Kiwibes data store interface 
  @param authentication pointer to the Kiwibes authentication
*/
void setup_rest_interface(httplib::SSLServer *https,
                          KiwibesJobsManager *manager,
                          KiwibesScheduler *scheduler,
                          KiwibesDatabase *database,
                          KiwibesDataStore *store,
                          KiwibesAuthentication *authentication);
#endif
