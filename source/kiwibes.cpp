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
#include "kiwibes_errors.h"

#include <string>
#include <iostream>
#include <set>

#include "NanoLog/NanoLog.hpp"

#if defined(__linux__)
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
}

Kiwibes::~Kiwibes()
{
  scheduler->stop();
  manager->stop_all_jobs();
  database->save();
}

unsigned int Kiwibes::get_listening_port(void)
{
  return cmd_line[KWB_OPT_HTTP_PORT].value;
}

int Kiwibes::init(int argc,char **argv)
{
  int error = ERROR_NO_ERROR;

  std::cout << "[INFO] initialization of the Kiwibes server" << std::endl;

  /* parse the command line arguments */
  error = parse_cmd_line(argc,argv);

  if(ERROR_NO_ERROR == error)
  {
    /* setup the home folder and start logging */
    setup_home();
  }

  if(ERROR_NO_ERROR == error)
  {
    /* initialize the database and the scheduler */
    LOG_INFO << "loading the jobs database";
    error = database->load(*(home) + std::string("/kiwibes.json"));

    if(ERROR_NO_ERROR != error)
    {
      LOG_CRIT << "failed to load the database, exiting";
    }
  }

  if(ERROR_NO_ERROR == error)
  {  
    /* schedule jobs that run periodically */
    scheduler->start();
  
    LOG_INFO << "scheduling periodic jobs";
    std::vector<std::string> schedulable_jobs; 
    database->get_all_schedulable_jobs(schedulable_jobs);

    for(std::string &name : schedulable_jobs)
    {
      scheduler->schedule_job(name);  
    }
  
    /* initialization is complete, when reaching here */
    std::cout << "[INFO] the Kiwibes server is initialized" << std::endl;
    LOG_INFO << "the Kiwibes server is initialized";
  }

  return error;
}

int Kiwibes::parse_cmd_line(int argc,char **argv)  
{
  int error = ERROR_NO_ERROR;

  if(2 > argc)
  {
    show_help();
    error = ERROR_CMD_LINE_PARSE;  
  }
  else
  {
    home.reset(new std::string(argv[1]));
    
    for(int a = 2; a < argc; a++)
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
        error = ERROR_CMD_LINE_PARSE;
      }
    }
  }

  return error;
}

int Kiwibes::setup_home(void)
{
  int error = ERROR_NO_ERROR;

  /* create the home folder, if it does not exist */  
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
      error = ERROR_HOME_SETUP;
    }
    else
    {
      std::cout << "[INFO] created folder: " << *home << std::endl;  
    }
  }

  if(ERROR_NO_ERROR == error)
  {
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

  return error;
}

void Kiwibes::show_help(void)
{
  std::cout << "Usage: kiwibes HOME [OPTIONS]" << std::endl << std::endl;
  std::cout << "HOME is the location of the kiwibes folder, it will created if it does not exist." << std::endl;
  std::cout << "The options set different working parameters:" << std::endl;
  
  for(unsigned int opt = 0; opt < sizeof(cmd_line)/sizeof(OPTIONS_T); opt++)
  {
    std::cout << "  " << cmd_line[opt].option << " UINT\t" << cmd_line[opt].description << std::endl;
  } 
  std::cout << std::endl;
}

void Kiwibes::post_start_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = manager->start_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

void Kiwibes::post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = manager->stop_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

void Kiwibes::post_create_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  if(0 == req.params.size())
  {
    result["error"] = ERROR_EMPTY_REST_REQUEST;
  }
  else
  {
    nlohmann::json description((*(req.params.begin())).second);
    result["error"] = database->create_job(req.matches[1],description);
  }

  res.set_content(result.dump(),"application/json");
}

void Kiwibes::post_edit_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  if(0 == req.params.size())
  {
    result["error"] = ERROR_EMPTY_REST_REQUEST;
  }
  else
  {
    nlohmann::json description((*(req.params.begin())).second);
    result["error"] = database->edit_job(req.matches[1],description);
  }

  res.set_content(result.dump(),"application/json");
}

void Kiwibes::post_delete_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = database->delete_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

void Kiwibes::get_get_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json job; 
  T_KIWIBES_ERROR error = database->get_job_description(job,req.matches[1]);

  job["error"] = error;

  res.set_content(job.dump(),"application/json");
}

void Kiwibes::get_list_jobs(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  database->get_all_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.set_content(names.dump(),"application/json");    
}