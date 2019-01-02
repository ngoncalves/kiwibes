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
  
  This class implements the Web interface.
*/
#ifndef __KIWIBES_WEB_H__
#define __KIWIBES_WEB_H__

#include "kiwibes_database.h"
#include "kiwibes_jobs_manager.h"
#include "kiwibes_authentication.h"

#include "cpp-httplib/httplib.h"

#include <string>

/*-------------------------- Public Function Declarations -------------------------------*/

/** Setup the HTML page route handlers
 
  @param https          pointer to the HTTPS server
  @param manager        pointer to the Kiwibes jobs manager
  @param database       pointer to the Kiwibes database interface 
  @param authentication pointer to the Kiwibes authentication
  @param templates      path to the Web templates folder
*/
void setup_web_interface(httplib::SSLServer *https,
                         KiwibesJobsManager *manager,
                         KiwibesDatabase *database,
                         KiwibesAuthentication *authentication,
                         std::string &templates);

/** Cleanup private data
 */
void cleanup_web_interface(void);

#endif
