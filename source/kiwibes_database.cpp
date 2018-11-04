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
#include <fstream>
#include <iomanip>
#include "kiwibes_database.h"
#include "kiwibes_cron_parser.h"
#include "NanoLog/NanoLog.hpp"
#include "ccronexpr/ccronexpr.h"

KiwibesDatabase::KiwibesDatabase()
{
  dbhome.reset(nullptr);
  db.reset(nullptr);
}

KiwibesDatabase::~KiwibesDatabase()
{

}

bool KiwibesDatabase::load(const std::string &home)
{ 
  std::lock_guard<std::mutex> lock(dblock);

  bool fail = false;

  dbhome.reset(new std::string(home));

  /* clear the current database */
  db.reset(new nlohmann::json());
  (*db)["backup_counter"] = 0;
  (*db)["jobs"]           = nullptr;

  /* read the database file */
  std::string dbfname = std::string(*dbhome + std::string("/kiwibes.json"));
  std::ifstream dbfile(dbfname);

  if(true == dbfile.is_open())
  {
    try
    {
      /* load the database */
      dbfile >> *db;

      /* make sure that each job has:
        - a name 
        - a command 
        - a maximum runtime in seconds        
        - a state: running or stopped, initially at stopped
        - average run time
        - standard deviation for the run time     
        - optionally a schedule
      */
      for(auto &job : (*db)["jobs"])
      {
        /* the name, command and max-runtime fields are mandatory. And if the
           schedule field is present, it must be a valid CRON expression
         */
        if(0 == job.count("name"))
        {
          LOG_CRIT << "found a job without field 'name'";
          fail = true;
          break;
        }
        else if(0 == job.count("command"))
        {
          LOG_CRIT << "job '" << job["name"].get<std::string>() << "' has no field 'command'";
          fail = true;
          break;
        }
        else if(0 == job.count("max-runtime"))
        {
          LOG_CRIT << "job '" << job["name"].get<std::string>() << "' has no field 'max-runtime'";
          fail = true;
          break;
        }
        else if(1 == job.count("schedule"))
        {
          /* verify that the CRON expression is valid */
          KiwibesCronParser cron(job["schedule"].get<std::string>());

          if(false == cron.is_valid())
          {
            LOG_CRIT << "job '" << job["name"].get<std::string>() << "' has an invalid schedule";
            fail = true;
            break;    
          }
        }

        /* if the runtime statistics are not preset, create them */
        if(0 == job["avg-runtime"])
        {
          job["avg-runtime"] = 0.0;
        }

        if(0 == job["stdev-runtime"])
        {
          job["stdev-runtime"] = 0.0;
        }

        /* reset the job state to 'stopped' */
        job["state"] = "stopped";
      }
    }
    catch(nlohmann::detail::parse_error &e)
    {
      LOG_CRIT << "failed to parse JSON file: " << dbfname;
      LOG_CRIT << "JSON error: " << e.what();
      fail = true; 
    }
  }

  /* add and empty object */
  (*db)["empty"] = nullptr;

  return !fail; 
}

bool KiwibesDatabase::save(void)
{
  std::lock_guard<std::mutex> lock(dblock);

  bool              success        = true;
  unsigned int long backup_counter = (*db)["backup_counter"];  
  std::string       dbfname        = std::string(*dbhome + std::string("/kiwibes.json"));

  /* backup the current file, if there is one */
  if(0 != std::rename(dbfname.c_str(),(dbfname + std::to_string(backup_counter)).c_str()))
  {
    LOG_CRIT << "failed to backup the database: ";
  }
  else
  {
    std::ofstream dbfile(dbfname);
    dbfile << std::setw(4) << (*db) << std::endl;
    (*db)["backup_counter"] += 1;  
  }
  
  return success; 
} 

const nlohmann::json &KiwibesDatabase::get_all_jobs(void)
{
  std::lock_guard<std::mutex> lock(dblock);

  return (*db)["jobs"];  
}

const nlohmann::json &KiwibesDatabase::get_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(dblock);

  for(auto &job : (*db)["jobs"])
  {
    if(0 == name.compare(job["name"]))
    {
      return job;
    }
  }

  return (*db)["empty"];  
}
