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
  On disk, the database is stored in a JSON file. Which is then
  loaded and modified in memory.
*/
#ifndef __KIWIBES_DATABASE_H__
#define __KIWIBES_DATABASE_H__

#include "kiwibes_errors.h"

#include "nlohmann/json.h"

#include <mutex>
#include <string>
#include <vector>
#include <memory>

class KiwibesDatabase {

public:
  /** Class constructor
   */
  KiwibesDatabase();

  /** Load the job descriptions to memory

    The database is the JSON file "kiwibes.json" located in
    the home folder. 

    @param home  full path to the folder containing the database
    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR load(const std::string &home);

  /** Save the job descriptions to file

    The database is the JSON file "kiwibes.json" located in
    the home folder. 

    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR save(void);

  /** Update the job status to running

    @param name   the name of the job
   */
  void job_started(const std::string &name);

  /** Update the job status to stopped

    @param name   the name of the job
   */
  void job_stopped(const std::string &name);

  /** Return the names of the jobs that can be scheduled

    @param jobs   on return contains the names of the jobs that can be scheduled
   */
  void get_all_schedulable_jobs(std::vector<std::string> &jobs);

  /** Return the names of all jobs

    @param jobs   on return contains the names of all jobs  
   */
  void get_all_job_names(std::vector<std::string> &jobs);  

  /** Return the description of the given job

   @param job   on return contains the JSON description of the job
   @param name  the name of the job 
   @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR get_job_description(nlohmann::json &job, const std::string &name);

  /** Delete the job with the given name 

   @param name  the name of the job
   @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR delete_job(const std::string &name);

  /** Create a new job with the given details

   @param name      the name of the job 
   @param details   the description of the job
   @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR create_job(const std::string &name, const nlohmann::json &details);

  /** Update the job with the new details

   @param name      the name of the job 
   @param details   the description of the job
   @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR edit_job(const std::string &name, const nlohmann::json &details);

private:
  /** Save the database to file, without locking it first
   */
  void unsafe_save(void);
  
private:
  std::unique_ptr<std::string>    dbpath;   /* path to the Kiwibes database file */                     
  std::mutex                      dblock;   /* synchronize access to the database */
  std::unique_ptr<nlohmann::json> dbjobs;   /* the jobs database, kept in memory */ 
};

#endif
