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

  This class implements the automation jobs.
*/
#ifndef __KIWIBES_JOB_H__
#define __KIWIBES_JOB_H__

#include <string>
#include <memory>

class KiwibesJob {

public:
  /** Class constructor
   */
  KiwibesJob();

  /** Class destructor
   */
  ~KiwibesJob();

  /** Load the job description from file

    @param fname  full path to the file name
    @returns true if successfull, false otherwise

    The file is assumed to have the following info, one per line:
    name: <string>
    schedule: <string>
    executable: <string>
    max runtime: <unsigned integer>
    ---
    [optional program script]

    where:
      name          is the job name, must fit in a single line
      schedule      (optional) is a cron-like expression for executing the job
      max_runtime   is the maximum allowed runtime for the job, in seconds
      executable    full path to the program to run

    The order in which the key-value pairs appear is not relevant.
    After these arguments, an optional script can be given.
    All lines begining with '#' are ignored. 
   */ 
  bool load(const char *fname);

private:
  std::unique_ptr<std::string> name;        /* name of the job */
  std::unique_ptr<std::string> schedule;    /* cron like schedule for executing the job */
  std::unique_ptr<std::string> executable;  /* full path to the program to run */
  std::unique_ptr<std::string> fname;       /* full path to the file with te job description */
  unsigned int maxRuntime;                  /* maximum runtime allowed for the job, in seconds */  
};

#endif
