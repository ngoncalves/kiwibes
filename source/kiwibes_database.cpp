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
#include <iostream>

#include "kiwibes_database.h"

#include "NanoLog/NanoLog.hpp"

KiwibesDatabase::KiwibesDatabase(const char *home_folder)
{
  db_fname.reset(new std::string(*(home_folder) + 
                                 std::string("/") +
                                 std::string("kiwibes.json")
                                )
                );
  db.reset(nullptr);  
}

KiwibesDatabase::~KiwibesDatabase()
{

}

bool KiwibesDatabase::load(void)
{ 
  bool success = true;

  /* create the 'database' and set the initial values */
  db.reset(new nlohmann::json);
  (*db)["save_counter"] = 0;
  (*db)["jobs"] = nullptr;

  std::ifstream fname(*db_fname);

  if(true == fname.is_open())
  {
    try
    {
      fname >> *db;
    }
    catch(nlohmann::detail::parse_error &e)
    {
      LOG_CRIT << "failed to parse JSON file: " << *db_fname;
      LOG_CRIT << "JSON error: " << e.what();
      success = false; 
    }
  }

  return success; 
}

bool KiwibesDatabase::save(void)
{
  bool success = true;

  /* backup the current file, if there is one */
  if(0 != std::rename(db_fname->c_str(),(*db_fname + std::to_string((int)(*db)["save_counter"])).c_str()))
  {
    LOG_CRIT << "failed to backup the database: ";
  }
  else
  {
    std::ofstream fname(*db_fname);
    fname << std::setw(4) << (*db) << std::endl;
    (*db)["save_counter"] += 1;  
  }
  
  return success; 
} 
