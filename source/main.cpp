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
  Application setup and startup .
*/
#include <iostream>
#include <vector>
#include <string>

#include "kiwibes_database.h"
#include "kiwibes_data_store.h"
#include "kiwibes_jobs_manager.h"
#include "kiwibes_scheduler.h"
#include "kiwibes_errors.h"
#include "kiwibes_cmd_line.h"
#include "kiwibes_rest.h"

#include "cpp-httplib/httplib.h"
#include "NanoLog/NanoLog.hpp"

/* Headers necessary for catching CTRL-C.
 * This is platform specific.  
 */
#if defined(__linux__)
  #include <signal.h>
  #include <unistd.h>
#else
  #error "OS not supported !"
#endif 

/*--------------------------Private Data Definitions -------------------------------*/
/** Copyright and version information
 */
#define KIWIBES_VERSION         "1.0.0"
#define KIWIBES_COPYRIGHT_YEARS "2018"

/** The Kiwibes components
 */
static KiwibesDatabase    *database       = nullptr;    /* database interface */
static KiwibesDataStore   *data_store     = nullptr;    /* data store interface */
static KiwibesJobsManager *jobs_manager   = nullptr;    /* jobs execution manager */
static KiwibesScheduler   *jobs_scheduler = nullptr;    /* jobs scheduler */
static httplib::Server    *http           = nullptr;    /* HTTP server, for the REST interface */  

/*--------------------------Private Function Declarations -------------------------------*/
/** Clean up the server on exit
 */
static void cleanup(void);

/** Show the copyright information
 */
static void show_copyright(void);

/** Handler for the CTRL-C signal

  @param sig  the signal that was received
 */
static void signal_handler(int sig);

/** Setup logging

  @param options  reference to the parsed command line options 
 */
static void start_logging(T_CMD_LINE_OPTIONS &options);

/** Initialize the Kiwibes Automation Server

  @param options  reference to the parsed command line options   
  @returns ERROR_NO_ERROR if successfull, error code otherwise
*/
static T_KIWIBES_ERROR initialize_kiwibes(T_CMD_LINE_OPTIONS &options);

/*--------------------------Public Function Definitions -------------------------------*/
int main(int argc, char **argv)
{
  int                error = ERROR_NO_ERROR;
  T_CMD_LINE_OPTIONS options;

  /* main program steps:
    - set the cleanup routine
    - setup the signal handlers (platform specific) 
    - show the copyright information
    - parse the command line 
    - start the logging server
    - instantiate the Kiwibes components
    - run the HTTP server main loop
   */
  atexit(cleanup);

#if defined(__linux__)
  struct sigaction sigHandler;

  sigHandler.sa_handler = signal_handler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;

  sigaction(SIGINT,&sigHandler,NULL);
#endif

	show_copyright();

  error = parse_and_validate_command_line(options,argc,argv);

  if(ERROR_NO_ERROR == error)
  {
    /* start logging */
    start_logging(options);

    /* instantiate the Kiwibes components */
    error = initialize_kiwibes(options);
  }
  else
  {
    show_cmd_line_help();
  }

  if(ERROR_NO_ERROR == error)
  {
    /* run the HTTP server */
    std::cout << "Listening on the HTTP REST interface on port " << options.http_port << std::endl;
    LOG_INFO << "Listening on the HTTP REST interface on port " << options.http_port;
    
    http->listen("localhost",options.http_port);
  }

  return error;
}

/*--------------------------Private Function Definitions -------------------------------*/
static void cleanup(void)
{
  if(nullptr != http)
  {
    http->stop();
    delete http; 
  }

  if(nullptr != jobs_scheduler)
  {
    jobs_scheduler->stop();
    delete jobs_scheduler;
  }

  if(nullptr != jobs_manager)
  {
    jobs_manager->stop_all_jobs();
    delete jobs_manager;
  }

  if(nullptr != data_store)
  {
    delete data_store;
  }

  if(nullptr != database)
  {
    database->save();
    delete database;
  }
}

static void show_copyright(void)
{
  std::cout << "Kiwibes Automation Server v" << KIWIBES_VERSION << std::endl;
  std::cout << "Copyright (c) " << KIWIBES_COPYRIGHT_YEARS;
  std::cout <<  "by Nelson Filipe Ferreira Gonçalves." << std::endl;
  std::cout << "All rights reserved." << std::endl << std::endl;
}

static void signal_handler(int sig)
{
  LOG_CRIT << "caught CTRL-C, exiting";
  exit(ERROR_MAIN_INTERRUPTED);
}

static void start_logging(T_CMD_LINE_OPTIONS &options)
{
  nanolog::initialize(nanolog::GuaranteedLogger(),*(options.home),"kiwibes.log",options.log_max_size);  
   
  if(0 == options.log_level)
  {
    nanolog::set_log_level(nanolog::LogLevel::CRIT);
  }
  else if(1 == options.log_level)
  {
    nanolog::set_log_level(nanolog::LogLevel::WARN);  
  }
  else
  {
    nanolog::set_log_level(nanolog::LogLevel::INFO);    
  }
}

static T_KIWIBES_ERROR initialize_kiwibes(T_CMD_LINE_OPTIONS &options)
{
  T_KIWIBES_ERROR error        = ERROR_NO_ERROR;
  std::string     jobs_db_file = *(options.home) + std::string("kiwibes.json"); 

  std::cout << "[INFO] loading the Kiwibes jobs database from: " << jobs_db_file << std::endl;
  LOG_INFO << "loading the Kiwibes jobs database from: " << jobs_db_file;

  database = new KiwibesDatabase;
  error = database->load(jobs_db_file);

  if(ERROR_NO_ERROR != error)
  {
    LOG_CRIT << "failed to load the database from: " << jobs_db_file;
    std::cout << "[ERROR] failed to load the database" << std::endl;
  }
  else
  {
    /* create the other components */
    data_store     = new KiwibesDataStore(options.data_store_size);
    jobs_manager   = new KiwibesJobsManager(database);
    jobs_scheduler = new KiwibesScheduler(database,jobs_manager);
    http           = new httplib::Server;

    /* schedule all jobs that have a valid schedule */
    jobs_scheduler->start();
  
    std::cout << "[INFO] scheduling all jobs with a valid schedule" << std::endl;
    LOG_INFO << "scheduling all jobs with a valid schedule";

    std::vector<std::string> schedulable_jobs; 
    database->get_all_schedulable_jobs(schedulable_jobs);

    for(std::string &name : schedulable_jobs)
    {
      jobs_scheduler->schedule_job(name);  
    }
  
    /* setup the REST interface */
    setup_rest_interface(http,jobs_manager,jobs_scheduler,database,data_store);

    /* initialization is complete */
    std::cout << "[INFO] the Kiwibes server is initialized" << std::endl;
    LOG_INFO << "the Kiwibes server is initialized";
  }

  return error;
}