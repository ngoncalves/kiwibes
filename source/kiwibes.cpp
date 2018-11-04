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
#include "kiwibes.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#include "NanoLog/NanoLog.hpp"

#if defined(__linux__)
  #include <unistd.h>
  #include <pwd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
#else
  #error "OS not supported"
#endif

/*------------------------- Private Data Definitions ---------------------*/
/** Logging levels
 */
#define KWB_OPT_LOG_LEVEL 0
#define KWB_OPT_LOG_SIZE  1
#define KWB_OPT_HTTP_PORT 2

/** Expected command line options 
 */
typedef struct{
  const char *option;
  const char *description;
  unsigned int value;
} OPTIONS_T;

static OPTIONS_T cmd_line[] = {
  { "-l","Log level. Defaults to 0 (critical messages only)",0},
  { "-s","Maximum size of the log, in MB. Defaults to 1 MB",1},
  { "-p","HTTP server listening port. Defaults to 4242",4242},
};

Kiwibes::Kiwibes()
{
  /* create:
    - the database interface object
    - the scheduler for periodic jobs
    - the jobs manager
   */
  database.reset(new KiwibesDatabase);
  manager.reset(new KiwibesJobsManager(database.get()));  
  scheduler.reset(new KiwibesScheduler(database.get(),manager.get()));
  home.reset(nullptr);

  scheduler->start();
}

Kiwibes::~Kiwibes()
{
  /* stop the jobs scheduler, as well as any job still running */
  scheduler->stop();
  manager->stop_all();
}

void Kiwibes::init(int argc,char **argv)
{
  std::cout << "[INFO] initialization of the Kiwibes server" << std::endl;

  /* parse the command line arguments */
  parse_cmd_line(argc,argv);

  /* setup the home folder and start logging */
  setup_home();

  /* initialize the database and the scheduler */
  LOG_INFO << "loading the jobs database";
  if(false == database->load(*(home.get())))
  {
    LOG_CRIT << "failed to load the database, exiting";
    exit(EXIT_ERROR_FAIL_LOAD_DATABASE);    
  }

  /* schedule jobs that run periodically */
  LOG_INFO << "scheduling periodic jobs";
  
  for(auto &job : database->get_all_jobs())
  {
    if(1 == job.count("schedule"))
    {
      scheduler->schedule_job(job["name"].get<std::string>());  
    }
  }

  /* initialization is complete, when reaching here */
  std::cout << "[INFO] the Kiwibes server is initialized" << std::endl;
  LOG_INFO << "the Kiwibes server is initialized";
}

void Kiwibes::parse_cmd_line(int argc,char **argv)  
{
  for(int a = 1; a < argc; a++)
  {
    bool found = false;
      
    for(unsigned int opt = 0; opt < sizeof(cmd_line)/sizeof(OPTIONS_T); opt++)
    {
      if((0 == strcmp(cmd_line[opt].option,argv[a])) && (a + 1) < argc) 
      {
        a++;
        cmd_line[opt].value = strtol(argv[a],NULL,10);
        found = true;
        break;  
      }
    }

    if(!found)
    {
      show_help();
      exit(EXIT_ERROR_FAIL_CMD_LINE_PARSE);
    }
  }
}

void Kiwibes::setup_home(void)
{
  /* create the home folder, if it does not exist */
#if defined(__linux__)
  const char *user_home = getenv("HOME");
  if(NULL == user_home)
  {
    home.reset(new std::string(std::string(getpwuid(getuid())->pw_dir) + std::string("/.kiwibes/")));
  }
  else
  {
    home.reset(new std::string(std::string(user_home) + std::string("/.kiwibes/")));
  }
#endif
  
  struct stat path;
    
  if(0 != stat(home->c_str(),&path))
  {
    /* folder does not exist, create it with the permissions:
      - read, write and execute by the owner
      - read and execute by the group
      - read and execute by others 
    */
    if(0 != mkdir(home->c_str(),S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
    {
      std::cout << "[ERROR] failed to create folder: " << *home << std::endl;
      exit(EXIT_ERROR_FAIL_HOME_SETUP);
    }
    else
    {
      std::cout << "[INFO] created folder: " << *home << std::endl;  
    }
  }

  /* start logging */
  nanolog::initialize(nanolog::GuaranteedLogger(),*home,"kiwibes.log",cmd_line[KWB_OPT_LOG_SIZE].value);  
   
  if(0 == cmd_line[KWB_OPT_LOG_LEVEL].value)
  {
    nanolog::set_log_level(nanolog::LogLevel::CRIT);
  }
  else if(1 == cmd_line[KWB_OPT_LOG_LEVEL].value)
  {
    nanolog::set_log_level(nanolog::LogLevel::WARN);  
  }
  else
  {
    nanolog::set_log_level(nanolog::LogLevel::INFO);    
  }
}

void Kiwibes::show_help(void)
{
  std::cout << "Usage: kiwibes [OPTIONS]" << std::endl << std::endl;
  std::cout << "All arguments are optional and set different working parameters:" << std::endl;
  
  for(unsigned int opt = 0; opt < sizeof(cmd_line)/sizeof(OPTIONS_T); opt++)
  {
    std::cout << "  " << cmd_line[opt].option << " UINT\t" << cmd_line[opt].description << std::endl;
  } 
  std::cout << std::endl;
}

int Kiwibes::run(void)
{
  /* TODO */

  return EXIT_ERROR_NO_ERROR;
}