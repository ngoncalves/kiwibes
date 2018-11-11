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

  This class implements the interface layer for the database.
  In practice the "database" is a simple JSON file, located in
  the home folder of the Kiwibes server.
*/
#ifndef __KIWIBES_DATABASE_H__
#define __KIWIBES_DATABASE_H__

#include <mutex>
#include <string>
#include <map>
#include <memory>
#include <chrono>
#include "nlohmann/json.h"

class KiwibesDatabase {

public:
  /** Class constructor
   */
  KiwibesDatabase();

  /** Class destructor
   */
  ~KiwibesDatabase();

  /** Load the job descriptions to memory

    The database is the JSON file "kiwibes.json" located in
    the home folder. 

    @param home  full path to the folder containing the database
    @return true if successfull, false otherwise
  */
  bool load(const std::string &home);

  /** Save the database to file

    @return true if successfull, false otherwise

    It creates a backup of the current filde, and then
    saves the database information to file.
  */ 
  bool save(void);

  /** Return a JSON list with all jobs 
   */
  const nlohmann::json &get_all_jobs(void);

  /** Return the JSON description of a job 

    @param name   the name of the job
    @return the JSON description of the job,  nullptr if the name was not found 
   */
  const nlohmann::json &get_job(const std::string &name);

  /** Change the job state to 'running'

   @param name  the name of the job
  */
  void job_started(const std::string &name);

  /** Change the job state to 'stopped'

   @param name      the name of the job
   @param runtime   the number of seconds during which the job ran
  */
  void job_stopped(const std::string &name, std::time_t runtime);

private:
  std::mutex                      dblock;   /* synchronize access to the database */
  std::unique_ptr<std::string>    dbhome;   /* full path to the folder containing the database */
  std::unique_ptr<nlohmann::json> db;       /* the database */
};

#endif
