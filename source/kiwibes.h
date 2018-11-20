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

  This class implements the Kiwibes Automation Server,
  with its REST interface
*/
#ifndef __KIWIBES_H__
#define __KIWIBES_H__

#include <memory>
#include "cpp-httplib/httplib.h"
#include "kiwibes_database.h"
#include "kiwibes_scheduler.h"
#include "kiwibes_jobs_manager.h"

/*--------- Kiwibes Server Class -------------------------------- */

class Kiwibes : public httplib::Server {

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
  
    @return ERROR_NO_ERROR if successfull, error code otherwise

    This function parses the command line arguments, starts
    the logger and loads the jobs descriptions. It then schedules
    the jobs that must run periodically.
  */
  T_KIWIBES_ERROR init(int argc,char **argv);

  /** Return the listening port for the REST interface
    */
  unsigned int get_listening_port(void);

  /** REST: Start job

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void post_start_job(const httplib::Request& req, httplib::Response& res);

  /** REST: Stop job

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void post_stop_job(const httplib::Request& req, httplib::Response& res);

  /** REST: Create job

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void post_create_job(const httplib::Request& req, httplib::Response& res);

  /** REST: Edit job description

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void post_edit_job(const httplib::Request& req, httplib::Response& res);

  /** REST: Delete job

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void post_delete_job(const httplib::Request& req, httplib::Response& res);

  /** REST: Get job description

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void get_get_job(const httplib::Request& req, httplib::Response& res);

  /** REST: List the name of all jobs

    @param req  the incoming HTTP request
    @param res  the outgoing HTTP response
  */
  void get_list_jobs(const httplib::Request& req, httplib::Response& res);

private:
  /** Show the command line help
   */
  void show_help(void);

  /** Verify the home folder exists and start logging

    @returns ERROR_NO_ERROR if successfull, error code otherwise
   */
  T_KIWIBES_ERROR setup_home(void);
  
  /** Parse the command line arguments

    @returns ERROR_NO_ERROR if successfull, error code otherwise

    @param argc   number of command line input arguments
    @param argv   array of command line input arguments
   */
  T_KIWIBES_ERROR parse_cmd_line(int argc, char **argv);

private:
  std::unique_ptr<KiwibesDatabase>    database;   /* contains all information about jobs and the server */
  std::unique_ptr<KiwibesScheduler>   scheduler;  /* schedules jobs to run */
  std::unique_ptr<KiwibesJobsManager> manager;    /* job management */
  std::unique_ptr<std::string>        home;       /* the home folder */
};

#endif
