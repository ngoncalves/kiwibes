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

  This class implements the Kiwibes Automation Server.
*/
#ifndef __KIWIBES_H__
#define __KIWIBES_H__

#include <memory>
#include "kiwibes_database.h"
#include "kiwibes_scheduler.h"

class Kiwibes {

public:
  /** Class constructor
   */
  Kiwibes();

  /** Class destructor
   */
  ~Kiwibes();

  /** Initialize the server

    @param argc   number of command line input arguments
    @param argv   array of command line input arguments

    This function parses the command line arguments, starts
    the logger and loads the jobs descriptions. It then schedules
    the jobs that must run periodically.

    In case of error, it forces the application to exit.
  */
  void init(int argc,char **argv);

  /** Run server main loop

    This method runs the HTTP server main loop until it is either
    stopped by the user or CTRL-C is received.

    @returns 0 if successfull, non-null value otherwise
  */
  int run(void);

private:
  /** Show the command line help
   */
  void show_help(void);

  /** Setup the home folder and start the logging system

    It creates the home folder:
      $HOME$/.kiwibes       if running in Linux
      $APPDATA$/kiwibes     if running in Windows

    where $HOME$ is the user home folder in Linux and
    $APPDATA$ is the user AppData folder in Windows.

    The Kiwibes server activity logs are written in the
    application home folder.
    In case of faillure, this method forces the application to exit.
   */
  void setup_home(void);
  
  /** Parse the command line arguments

    @param argc   number of command line input arguments
    @param argv   array of command line input arguments

    In case of faillure, this method forces the application to exit.
   */
  void parse_cmd_line(int argc, char **argv);

private:
  std::unique_ptr<KiwibesDatabase>  database;   /* contains all information about jobs and the server */
  std::unique_ptr<KiwibesScheduler> scheduler;  /* schedules jobs to run */
  std::unique_ptr<std::string>      home;       /* the home folder */
};

#endif
