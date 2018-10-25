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
#include "NanoLog/NanoLog.hpp"

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

  dbhome.reset(new std::string(home));
  bool success = true;

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

      /* reset the state of each job, to be initially stopped */
      for(auto &job : (*db)["jobs"])
      {
        job["state"] = "stopped";       
      }
    }
    catch(nlohmann::detail::parse_error &e)
    {
      LOG_CRIT << "failed to parse JSON file: " << dbfname;
      LOG_CRIT << "JSON error: " << e.what();
      success = false; 
    }
  }

  /* add and empty object */
  (*db)["empty"] = nullptr;

  return success; 
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

const nlohmann::json & KiwibesDatabase::get_all_jobs(void)
{
  std::lock_guard<std::mutex> lock(dblock);

  return (*db)["jobs"];  
}

const nlohmann::json & KiwibesDatabase::get_job(const std::string &name)
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
