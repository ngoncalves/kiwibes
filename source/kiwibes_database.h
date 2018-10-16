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

#include <memory>
#include <mutex>
#include "nlohmann/json.h"

class KiwibesDatabase {

public:
  /** Class constructor

    @param home_folder  full path to the database folder
   */
  KiwibesDatabase(const char *home_folder);

  /** Class constructor

    @param home_folder  pointer to the home folder
   */
  ~KiwibesDatabase();

  /** Load the database to memory

    @return true if successfull, false otherwise

    The database is the JSON file "kiwibes.json" located in
    the home folder. 
  */
  bool load(void);

  /** Save the database to file

    @return true if successfull, false otherwise

    It creates a backup of the current filde, and then
    saves the database information to file.
  */ 
  bool save(void);

private:
  std::unique_ptr<std::string>    db_fname;   /* full path to the database file */
  std::unique_ptr<nlohmann::json> db;         /* the database */
  std::mutex                      db_lock;    /* synchronize access to the database */
};

#endif
