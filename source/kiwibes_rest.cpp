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
static KiwibesDataStore   *pDataStore;
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

/** REST: Clear all pending start requests for this job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_clear_pending_job(const httplib::Request& req, httplib::Response& res);

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

/** Read the job parameters from the POST request

  @param params   on return, contains the POST job parameters
  @param req      the incomming HTTP request
  @return true if successfull, false otherwise
 */
static bool read_job_parameters(nlohmann::json &params, const httplib::Request &req);

/** REST: Write a piece of data

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_write_data(const httplib::Request& req, httplib::Response& res);

/** REST: Clear a piece of data

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_clear_data(const httplib::Request& req, httplib::Response& res);

/** REST: Clear all stored data

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void post_clear_all_data(const httplib::Request& req, httplib::Response& res);

/** REST: Get data

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void get_read_data(const httplib::Request& req, httplib::Response& res);

/** Set the return error code
 */
static void set_return_code(httplib::Response& res, T_KIWIBES_ERROR error);

/*--------------------------Public Function Definitions -------------------------------*/
void setup_rest_interface(httplib::Server *http, KiwibesJobsManager *manager, KiwibesScheduler *scheduler, KiwibesDatabase *database, KiwibesDataStore *store)
{
  /* setup the private pointers */
  pDatabase  = database; 
  pDataStore = store; 
  pScheduler = scheduler;
  pManager   = manager;  

  /* setup the HTTP REST route handlers */
  http->Post("/rest/job/start/([a-zA-Z_0-9]+)",post_start_job);
  http->Post("/rest/job/stop/([a-zA-Z_0-9]+)",post_stop_job);
  http->Post("/rest/job/create/([a-zA-Z_0-9]+)",post_create_job);    
  http->Post("/rest/job/edit/([a-zA-Z_0-9]+)",post_edit_job);    
  http->Post("/rest/job/delete/([a-zA-Z_0-9]+)",post_delete_job);    
  http->Post("/rest/job/clear_pending/([a-zA-Z_0-9]+)",post_clear_pending_job);    
  http->Get( "/rest/job/details/([a-zA-Z_0-9]+)",get_get_job);

  http->Post("/rest/data/write/([a-zA-Z_0-9]+)",post_write_data);    
  http->Post("/rest/data/clear/([a-zA-Z_0-9]+)",post_clear_data);    
  http->Post("/rest/data/clear_all",post_clear_all_data);    
  http->Get( "/rest/data/read/([a-zA-Z_0-9]+)",get_read_data);    
  
  http->Get("/rest/jobs/list",get_jobs_list);
  http->Get("/rest/jobs/scheduled",get_scheduled_jobs);
      
  /* setup the logger */
  http->set_logger(rest_logger);
}

/*--------------------------Private Function Definitions -------------------------------*/
static void post_start_job(const httplib::Request& req, httplib::Response& res)
{
  set_return_code(res,pManager->start_job(req.matches[1]));
}

static void post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  set_return_code(res,pManager->stop_job(req.matches[1]));
}

static void post_create_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json params; 
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(false == read_job_parameters(params,req))
  {
    error = ERROR_JOB_DESCRIPTION_INVALID;
    LOG_INFO << "job description invalid";
  }
  else
  {
    error = pDatabase->create_job(req.matches[1],params);
  
    if(ERROR_NO_ERROR == error)
    {
      /* if the job was created and can be scheduled, then scheduled it */
      std::string schedule = params["schedule"].get<std::string>();
      KiwibesCron cron(schedule);

      if((0 < schedule.size()) && (true == cron.is_valid()))
      {
        pScheduler->schedule_job(req.matches[1]);
      }
      else if((0 < schedule.size()) && (false == cron.is_valid()))
      {
        /* invalid schedule, delete the job */
        pDatabase->delete_job(req.matches[1]);
        error = ERROR_JOB_SCHEDULE_INVALID;
      }
    }
  }
  
  set_return_code(res,error);
}

static void post_edit_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json params; 
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(false == read_job_parameters(params,req))
  {
    error = ERROR_JOB_DESCRIPTION_INVALID;
  }
  else
  {
    error = pDatabase->edit_job(req.matches[1],params);
  
    if(ERROR_NO_ERROR == error)
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
        /* invalid schedule, delete the job */
        pDatabase->delete_job(req.matches[1]);
        error = ERROR_JOB_SCHEDULE_INVALID;
      }
    }
  }

  set_return_code(res,error);
}

static void post_delete_job(const httplib::Request& req, httplib::Response& res)
{
  T_KIWIBES_ERROR error = pDatabase->delete_job(req.matches[1]);

  if(ERROR_NO_ERROR == error)
  {
    /* unschedule the job, if it was previously scheduled */
    pScheduler->unschedule_job(req.matches[1]);  
  }

  set_return_code(res,error);
}

static void post_clear_pending_job(const httplib::Request& req, httplib::Response& res)
{
  set_return_code(res,pDatabase->job_clear_start_requests(req.matches[1]));
}

static void get_get_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json job; 
  
  if(ERROR_JOB_NAME_UNKNOWN == pDatabase->get_job_description(job,req.matches[1]))
  {
    set_return_code(res,ERROR_JOB_NAME_UNKNOWN);
  }
  else
  {
    res.status = 200;
    res.set_content(job.dump(),"application/json");
  }
}

static void get_jobs_list(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  pDatabase->get_all_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.status = 200;
  res.set_content(names.dump(),"application/json");    
}

static void get_scheduled_jobs(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  pScheduler->get_all_scheduled_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.status = 200;
  res.set_content(names.dump(),"application/json");    
}

static void post_write_data(const httplib::Request& req, httplib::Response& res)
{
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(true == req.has_param("value"))
  {
    error = pDataStore->write(req.matches[1],req.get_param_value("value"));
  }
  else
  {
    error = ERROR_EMPTY_REST_REQUEST;
  }

  set_return_code(res,error);
}

static void post_clear_data(const httplib::Request& req, httplib::Response& res)
{
  set_return_code(res,pDataStore->clear(req.matches[1]));
}

static void post_clear_all_data(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;
  result["cleared-count"] = pDataStore->clear_all();

  res.set_content(result.dump(),"application/json");
}

static void get_read_data(const httplib::Request& req, httplib::Response& res)
{
  std::string value; 
  T_KIWIBES_ERROR error = pDataStore->read(value,req.matches[1]);

  if(ERROR_NO_ERROR == error)
  {
    res.status = 200; /* ok */
    res.set_content(value,"text/plain");   
  }
  else
  {
    set_return_code(res,error);
  } 
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

static void set_return_code(httplib::Response& res, T_KIWIBES_ERROR error)
{
  nlohmann::json description; 

  description["code"]    = error;
  description["message"] = "";

  switch(error)
  {
    case ERROR_NO_ERROR: 
      res.status = 200;
      break; 

    case ERROR_JOB_NAME_UNKNOWN:
      res.status = 404;   /* Not found */
      description["message"] = "Job not found";
      break;

    case ERROR_DATA_KEY_UNKNOWN:
      res.status = 404;   /* Not found */
      description["message"] = "Data key not found";
      break;

    case ERROR_DATA_STORE_FULL:
      res.status = 507;   /* Not enough space */
      description["message"] = "Not enough space in the data storage";
      break;

    case ERROR_JOB_NAME_TAKEN:
      res.status = 409;   /* Conflict */
      description["message"] = "Job name already exists";
      break;

    case ERROR_DATA_KEY_TAKEN:
      res.status = 409;   /* Conflict */
      description["message"] = "Data key already exists";
      break;

    case ERROR_PROCESS_LAUNCH_FAILED:
      res.status = 500;   /* Generic server error */
      description["message"] = "Failed to start job";  
      break;

    case ERROR_JOB_DESCRIPTION_INVALID:
    case ERROR_EMPTY_REST_REQUEST:
      res.status = 400;   /* Bad request */
      description["message"] = "Bad request";
      break; 

    case ERROR_JOB_SCHEDULE_INVALID:
      res.status = 400;   /* Bad request */
      description["message"] = "Invalid job schedule";
      break; 

    case ERROR_JOB_IS_NOT_RUNNING:
      res.status = 403;   /* Not allowed */
      description["message"] = "Job is not running";
      break; 

    case ERROR_JOB_IS_RUNNING:
      res.status = 403;   /* Not allowed */
      description["message"] = "Job is running";
      break;
      
    default:
      res.status = 500;   /* Generic server error */
      description["message"] = "Generic server error";         
      break;      
  }

  res.set_content(description.dump(),"application/json");
}