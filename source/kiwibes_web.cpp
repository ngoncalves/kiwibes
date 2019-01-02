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
#include "kiwibes_web.h"
#include "kiwibes_cron.h"

#include "NanoLog/NanoLog.hpp"
#include "inja/inja.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <ctime>
#include <locale>
#include <sstream>

/*--------------------------Private Data Definitions -------------------------------*/
/** Private pointers to the Kiwibes components
 */
static KiwibesDatabase       *pDatabase       = nullptr;
static KiwibesJobsManager    *pManager        = nullptr;
static KiwibesAuthentication *pAuthentication = nullptr;
static std::string           *pTemplates      = nullptr;

static const char *no_web_interface =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
" <meta charset=\"utf-8\">"
" <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">"
" <title>Kiwibes</title>"
"  <meta name=\"author\" content=\"Nelson Gonçalves\">"
"  <meta name=\"description\" content=\"Kiwibes Web Interface\">"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"</head><body>"
"  <p>This Kiwibes server does not provide a web interface.</p>"
"</body></html>";

/*--------------------------Private Function Declarations -------------------------------*/

/** Start job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void web_post_start_job(const httplib::Request& req, httplib::Response& res);

/** Stop job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void web_post_stop_job(const httplib::Request& req, httplib::Response& res);

/** Clear all pending start requests for this job

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void web_post_clear_pending_job(const httplib::Request& req, httplib::Response& res);

/** List the name of all jobs

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void web_get_jobs_list(const httplib::Request& req, httplib::Response& res);

/** Authenticate with a token

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void web_post_authenticate(const httplib::Request& req, httplib::Response& res);

/** Set response to POST methods
  
  @param res    the outgoing HTTP response
  @param cookie the authentication cookie, if set
 */
static void web_response(T_KIWIBES_ERROR error, httplib::Response& res, const std::string &cookie);

/*--------------------------Public Function Definitions -------------------------------*/
void setup_web_interface(httplib::SSLServer *https,
                         KiwibesJobsManager *manager,
                         KiwibesDatabase *database,
                         KiwibesAuthentication *authentication,
                         std::string &templates)
{
  /* setup the private pointers */
  pDatabase       = database; 
  pManager        = manager;  
  pAuthentication = authentication;
  pTemplates      = new std::string(templates); 
  
  /* setup the Web route handlers */
  https->Post("/job/start/([a-zA-Z_0-9]+)",web_post_start_job);
  https->Post("/job/stop/([a-zA-Z_0-9]+)",web_post_stop_job);
  https->Post("/job/clear_pending/([a-zA-Z_0-9]+)",web_post_clear_pending_job);    
  https->Post("/authenticate",web_post_authenticate);    
  https->Get("/",web_get_jobs_list);
}

void cleanup_web_interface(void)
{
  if(nullptr != pTemplates)
  {
    delete pTemplates;
  }
}
/*--------------------------Private Function Definitions -------------------------------*/
static void web_post_start_job(const httplib::Request& req, httplib::Response& res)
{
  if((true != req.has_header("auth-token")) ||
     (true != pAuthentication->verify_web_cookie(req.get_header_value("auth-token",0)))
    )
  {
    web_response(ERROR_AUTHENTICATION_FAIL,res,nullptr);
  }
  else
  {
    web_response(pManager->start_job(req.matches[1]),res,req.get_header_value("auth-token",0));
  }
}

static void web_post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  if((true != req.has_header("auth-token")) ||
     (true != pAuthentication->verify_web_cookie(req.get_header_value("auth-token",0)))
    )
  {
    web_response(ERROR_AUTHENTICATION_FAIL,res,nullptr);
  }
  else
  {
    web_response(pManager->stop_job(req.matches[1]),res,req.get_header_value("auth-token",0));
  }
}

static void web_post_clear_pending_job(const httplib::Request& req, httplib::Response& res)
{
  if((true != req.has_header("auth-token")) ||
     (true != pAuthentication->verify_web_cookie(req.get_header_value("auth-token",0)))
    )
  {
    web_response(ERROR_AUTHENTICATION_FAIL,res,nullptr);
  }
  else
  {
    web_response(pDatabase->job_clear_start_requests(req.matches[1]),res,req.get_header_value("auth-token",0));
  }
}

static void web_post_authenticate(const httplib::Request& req, httplib::Response& res)
{
  if((true != req.has_header("auth-token")) ||
     (true != pAuthentication->verify_web_cookie(req.get_header_value("auth-token",0)))
    )
  {
    web_response(ERROR_AUTHENTICATION_FAIL,res,nullptr);
  }
  else
  {
    web_response(pManager->stop_job(req.matches[1]),res,req.get_header_value("auth-token",0));
  }
}

static void web_get_jobs_list(const httplib::Request& req, httplib::Response& res)
{
  if((true != req.has_header("auth-token")) ||
     (true != pAuthentication->verify_web_cookie(req.get_header_value("auth-token",0)))
    )
  {
    std::ifstream authentication_page(*pTemplates + std::string("authentication.html"));
    if(true == authentication_page.is_open())
    {
      inja::Environment env  = inja::Environment(*pTemplates);
      inja::Template    temp = env.parse_template("authentication.html");
      nlohmann::json    data; 

      res.set_content(env.render(temp, data),"text/html");
    }
    else
    {
      res.set_content(no_web_interface,"text/html");
    }
  }
  else
  {
    std::ifstream index_page(*pTemplates + std::string("index.html"));
    if(true == index_page.is_open())
    {
      inja::Environment env  = inja::Environment(*pTemplates);
      inja::Template    temp = env.parse_template("index.html");
      nlohmann::json    data; 
      unsigned int      page = 0; 
      std::vector<std::string> job_names; 

      pDatabase->get_all_job_names(job_names);
      std::sort(job_names.begin(),job_names.end());

      if(true == req.has_param("page"))
      {
        page = (unsigned int)std::stol(req.get_param_value("page"));
        if(page > job_names.size()/20)
        {
          page = (job_names.size() < 20 ? 0 : job_names.size()/20);
        }
      }

      for(unsigned int j = (page*20); j < 20*(page + 1); j++)
      {
        nlohmann::json job; 
        if(ERROR_NO_ERROR == pDatabase->get_job_description(job,job_names[j]))
        {
          data[job_names[j]] = job;
        }
      }

      res.set_content(env.render(temp,data),"text/html");
    
      std::string cookie = std::string("auth-token=") + cookie + std::string("; Secure");
      res.set_header("Set-Cookie",cookie.c_str());  
    }
    else
    {
      res.set_content(no_web_interface,"text/html");
    } 
  }
}

static void web_response(T_KIWIBES_ERROR error, httplib::Response& res, const std::string &cookie)
{
  switch(error)
  {
    case ERROR_NO_ERROR:
      /* all good, redirect to the home page */
      res.status = 303;
      res.set_content("Location: /","text/html");
      {
        std::string cookie = std::string("auth-token=") + cookie + std::string("; Secure");
        res.set_header("Set-Cookie",cookie.c_str());  
      }
      
      break;

    case ERROR_AUTHENTICATION_FAIL:
      res.status = 403;
      res.set_content("<p>Authentication failed !<p>","text/html");
      break;

    case ERROR_JOB_NAME_UNKNOWN:
      break; 

    default:
      break;
  }    
}
