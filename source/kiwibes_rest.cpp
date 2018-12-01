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
#include "kiwibes_rest.h"
#include "kiwibes_cron.h"

#include "NanoLog/NanoLog.hpp"
#include "nlohmann/json.h"

/*--------------------------Private Data Definitions -------------------------------*/
/** Private pointers to the Kiwibes components
 */
static KiwibesDatabase    *pDatabase;
static KiwibesJobsManager *pManager;
static KiwibesScheduler   *pScheduler;

/*--------------------------Private Function Declarations -------------------------------*/

/** REST: Start job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_start_job(const httplib::Request& req, httplib::Response& res);

/** REST: Stop job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_stop_job(const httplib::Request& req, httplib::Response& res);

/** REST: Create job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_create_job(const httplib::Request& req, httplib::Response& res);

/** REST: Edit job description

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_edit_job(const httplib::Request& req, httplib::Response& res);

/** REST: Delete a job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_delete_job(const httplib::Request& req, httplib::Response& res);

/** REST: Get job description

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void get_get_job(const httplib::Request& req, httplib::Response& res);

/** REST: List the name of all jobs

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void get_jobs_list(const httplib::Request& req, httplib::Response& res);

/** REST: List the name of all jobs currently scheduled to run

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void get_scheduled_jobs(const httplib::Request& req, httplib::Response& res);

/** REST: Requests logger

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void rest_logger(const httplib::Request& req, const httplib::Response& res);

/** REST: Error handler

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void rest_error_handler(const httplib::Request& req, httplib::Response& res);

/** Read the job parameters from the POST request

  @param params   on return, contains the POST job parameters
  @param req      the incomming HTTP request
  @return true if successfull, false otherwise
 */
static bool read_job_parameters(nlohmann::json &params, const httplib::Request &req);

/*--------------------------Public Function Definitions -------------------------------*/
void setup_rest_interface(httplib::Server *http, KiwibesJobsManager *manager, KiwibesScheduler *scheduler, KiwibesDatabase *database)
{
  /* setup the private pointers */
  pDatabase  = database; 
  pScheduler = scheduler;
  pManager   = manager;  

  /* setup the HTTP REST route handlers */
  http->Post("/start_job/([a-zA-Z_0-9]+)",post_start_job);
  http->Post("/stop_job/([a-zA-Z_0-9]+)",post_stop_job);
  http->Post("/create_job/([a-zA-Z_0-9]+)",post_create_job);    
  http->Post("/edit_job/([a-zA-Z_0-9]+)",post_edit_job);    
  http->Post("/delete_job/([a-zA-Z_0-9]+)",post_delete_job);    
  http->Get("/jobs_list",get_jobs_list);
  http->Get("/scheduled_jobs",get_scheduled_jobs);
  http->Get("/job/([a-zA-Z_0-9]+)",get_get_job);    

  /* setup the logger and the error handler */
  http->set_logger(rest_logger);
  http->set_error_handler(rest_error_handler);
}

/*--------------------------Private Function Definitions -------------------------------*/
static void post_start_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = pManager->start_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

static void post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = pManager->stop_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

static void post_create_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;
  nlohmann::json params; 

  if(false == read_job_parameters(params,req))
  {
    result["error"] = ERROR_JOB_DESCRIPTION_INVALID;
  }
  else
  {
    result["error"] = pDatabase->create_job(req.matches[1],params);
  
    if(ERROR_NO_ERROR == result["error"].get<T_KIWIBES_ERROR>())
    {
      /* if the job was created and can be scheduled, then scheduled it */
      std::string schedule = params["schedule"].get<std::string>();
      KiwibesCron cron(schedule);

      LOG_CRIT << "create job with schedule: |" << schedule << "|";

      if((0 < schedule.size()) && (true == cron.is_valid()))
      {
        pScheduler->schedule_job(req.matches[1]);
      }
      else if((0 < schedule.size()) && (false == cron.is_valid()))
      {
        result["error"] = ERROR_JOB_SCHEDULE_INVALID;       
      }
    }
  } 

  res.set_content(result.dump(),"application/json");
}

static void post_edit_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;
  nlohmann::json params; 

  if(false == read_job_parameters(params,req))
  {
    result["error"] = ERROR_EMPTY_REST_REQUEST;
  }
  else
  {
    result["error"] = pDatabase->edit_job(req.matches[1],params);
  
    if(ERROR_NO_ERROR == result["error"].get<T_KIWIBES_ERROR>())
    {
      /* if the job was edited and can be scheduled, then scheduled it */
      std::string schedule = params["schedule"].get<std::string>();
      KiwibesCron cron(schedule);

      if(cron.is_valid())
      {
        pScheduler->unschedule_job(req.matches[1]);
        pScheduler->schedule_job(req.matches[1]);
      }
      else if(0 == schedule.size())
      {
        pScheduler->unschedule_job(req.matches[1]);  
      }
      else if((0 < schedule.size()) && (false == cron.is_valid()))
      {
        result["error"] = ERROR_JOB_SCHEDULE_INVALID;  
      }
    }
  }
    
  res.set_content(result.dump(),"application/json");
}

static void post_delete_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = pDatabase->delete_job(req.matches[1]);

  if(ERROR_NO_ERROR == result["error"].get<T_KIWIBES_ERROR>())
  {
    /* unschedule the job, if it was previously scheduled */
    pScheduler->unschedule_job(req.matches[1]);  
  }

  res.set_content(result.dump(),"application/json");
}

static void get_get_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json job; 
  T_KIWIBES_ERROR error = pDatabase->get_job_description(job,req.matches[1]);

  job["error"] = error;

  res.set_content(job.dump(),"application/json");
}

static void get_jobs_list(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  pDatabase->get_all_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.set_content(names.dump(),"application/json");    
}

static void get_scheduled_jobs(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  pScheduler->get_all_scheduled_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.set_content(names.dump(),"application/json");    
}

static void rest_logger(const httplib::Request& req, const httplib::Response& res)
{
  std::string params("");

  for(auto it = req.params.begin(); it != req.params.end(); ++it)
  {
    params += (it == req.params.begin() ? '?' : '&') + (*it).first + (*it).second;
  }

  LOG_INFO << req.method.c_str() << " - " << req.path.c_str() << params ; 
}

static void rest_error_handler(const httplib::Request& req, httplib::Response& res)
{
  res.set_content("<p>ERROR</p>","text/html");
}

static bool read_job_parameters(nlohmann::json &params, const httplib::Request &req)
{
  bool success = true; 
  
  /* the expected parameters are: 
    - program     : a string array 
    - schedule    : a string 
    - max-runtime : an unsigned long integer 
   */
  if(true == req.has_param("max-runtime"))
  {
    params["max-runtime"] = (unsigned long int)std::stol(req.get_param_value("max-runtime"));
  } 
  else
  {
    success = false;
  }

  if(true == req.has_param("schedule"))
  {
    params["schedule"] = std::string(req.get_param_value("schedule"));
  } 
  else
  {
    success = false;
  }

  if(true == req.has_param("program"))
  {
    std::vector<std::string> program;
    req.get_param_value_as_vector(program,"program");
    params["program"] = program;
  } 
  else
  {
    success = false;
  }

  return success; 
}