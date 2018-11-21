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
static void get_list_jobs(const httplib::Request& req, httplib::Response& res);

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
  http->Get("/jobs_list",get_list_jobs);
  http->Get("/job/([a-zA-Z_0-9]+)",get_get_job);    

  /* setup the logger and the error handler */
  http->set_logger(rest_logger);
  http->set_error_handler(rest_error_handler);
}

/*--------------------------Private Function Definitions -------------------------------*/
void post_start_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = pManager->start_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

void post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  result["error"] = pManager->stop_job(req.matches[1]);

  res.set_content(result.dump(),"application/json");
}

void post_create_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  if(0 == req.params.size())
  {
    result["error"] = ERROR_EMPTY_REST_REQUEST;
  }
  else
  {
    nlohmann::json description((*(req.params.begin())).second);
    result["error"] = pDatabase->create_job(req.matches[1],description);
  
    if(ERROR_NO_ERROR == result["error"].get<T_KIWIBES_ERROR>())
    {
      /* if the job was created and can be scheduled, then scheduled it */
      KiwibesCron cron(description["schedule"]);

      if(cron.is_valid())
      {
        pScheduler->schedule_job(req.matches[1]);
      }
    }
  } 

  res.set_content(result.dump(),"application/json");
}

void post_edit_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json result;

  if(0 == req.params.size())
  {
    result["error"] = ERROR_EMPTY_REST_REQUEST;
  }
  else
  {
    nlohmann::json description((*(req.params.begin())).second);
    result["error"] = pDatabase->edit_job(req.matches[1],description);
  
    if(ERROR_NO_ERROR == result["error"].get<T_KIWIBES_ERROR>())
    {
      /* if the job was edited and can be scheduled, then scheduled it */
      KiwibesCron cron(description["schedule"]);

      if(cron.is_valid())
      {
        pScheduler->schedule_job(req.matches[1]);
      }
      else
      {
        pScheduler->unschedule_job(req.matches[1]);  
      }
    }
  }
    
  res.set_content(result.dump(),"application/json");
}

void post_delete_job(const httplib::Request& req, httplib::Response& res)
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

void get_get_job(const httplib::Request& req, httplib::Response& res)
{
  nlohmann::json job; 
  T_KIWIBES_ERROR error = pDatabase->get_job_description(job,req.matches[1]);

  job["error"] = error;

  res.set_content(job.dump(),"application/json");
}

void get_list_jobs(const httplib::Request& req, httplib::Response& res)
{
  std::vector<std::string> jobs;

  pDatabase->get_all_job_names(jobs);
  
  nlohmann::json names(jobs); 
  res.set_content(names.dump(),"application/json");    
}

static void rest_logger(const httplib::Request& req, const httplib::Response& res)
{
  LOG_INFO << req.method.c_str() << " - " << req.path.c_str();
}

static void rest_error_handler(const httplib::Request& req, httplib::Response& res)
{
  res.set_content("<p>ERROR</p>","text/html");
}