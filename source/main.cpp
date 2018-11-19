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

  Application entry point.
*/
#include <iostream>
#include <memory>
#include "kiwibes.h"
#include "kiwibes_errors.h"
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
#define KIWIBES_VERSION         "0.9.0"
#define KIWIBES_COPYRIGHT_YEARS "2018"

/** The Kiwibes servers:
  - the automation server
  - the REST HTTP  server
 */
static std::unique_ptr<Kiwibes>         kwbServer;
static std::unique_ptr<httplib::Server> restHTTP;

/*--------------------------Private Function Declarations -------------------------------*/
/** Show the copyright information
 */
static void show_copyright(void);

/** Handler for the CTRL-C signal

  @param sig  the signal that was received
 */
static void signal_handler(int sig);

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
static void restHTTPLogger(const httplib::Request& req, const httplib::Response& res);

/** REST: Error handler

  @param req  the incoming HTTP request
  @param res  the outgoing HTTP response
 */
static void restHTTPErrorHandler(const httplib::Request& req, httplib::Response& res);

/*--------------------------Public Function Definitions -------------------------------*/

int main(int argc, char **argv)
{
  int error = ERROR_NO_ERROR;

  /* main program steps:
    - setup the signal handlers (platform specific) 
    - show the copyright information
    - parse the command line 
    - run the server main loop
   */
#if defined(__linux__)
  struct sigaction sigHandler;

  sigHandler.sa_handler = signal_handler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;

  sigaction(SIGINT,&sigHandler,NULL);
#endif

	show_copyright();

  restHTTP.reset(new httplib::Server);
	kwbServer.reset(new Kiwibes);
  error = kwbServer->init(argc,argv);

	if(ERROR_NO_ERROR == error)
  {
    /* setup the HTTP REST server and run it */
    restHTTP->Post("/start_job/([a-zA-Z_0-9]+)",post_start_job);
    restHTTP->Post("/stop_job/([a-zA-Z_0-9]+)",post_stop_job);
    restHTTP->Post("/create_job/([a-zA-Z_0-9]+)",post_create_job);    
    restHTTP->Post("/edit_job/([a-zA-Z_0-9]+)",post_edit_job);    
    restHTTP->Post("/delete_job/([a-zA-Z_0-9]+)",post_delete_job);    
    restHTTP->Get("/jobs_list",get_list_jobs);
    restHTTP->Get("/job/([a-zA-Z_0-9]+)",get_get_job);    

    /* setup the logger and the error handler */
    restHTTP->set_logger(restHTTPLogger);
    restHTTP->set_error_handler(restHTTPErrorHandler);

    /* start listening for incoming requests */
    std::cout << "Listening on the REST interface" << std::endl;
    LOG_INFO << "Listening on the REST interface";
    
    restHTTP->listen("localhost",kwbServer->get_listening_port());   
  }
  
  return error;
}

/*--------------------------Private Function Definitions -------------------------------*/
static void show_copyright(void)
{
  std::cout << "Kiwibes Automation Server v" << KIWIBES_VERSION << std::endl;
  std::cout << "Copyright (c) " << KIWIBES_COPYRIGHT_YEARS;
  std::cout <<  "by Nelson Filipe Ferreira Gonçalves." << std::endl;
  std::cout << "All rights reserved." << std::endl << std::endl;
}

static void signal_handler(int sig)
{
  restHTTP->stop();
  exit(ERROR_MAIN_INTERRUPTED);
}

static void post_start_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->post_start_job(req,res);
}

static void post_stop_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->post_stop_job(req,res);
}

static void post_create_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->post_create_job(req,res);
}

static void post_edit_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->post_edit_job(req,res);
}

static void post_delete_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->post_edit_job(req,res);
}

static void get_get_job(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->get_get_job(req,res);
}

static void get_list_jobs(const httplib::Request& req, httplib::Response& res)
{
  kwbServer->get_list_jobs(req,res);
}

static void restHTTPLogger(const httplib::Request& req, const httplib::Response& res)
{
  LOG_INFO << req.method.c_str() << " - " << req.path.c_str();
}

static void restHTTPErrorHandler(const httplib::Request& req, httplib::Response& res)
{
  res.set_content("<p>ERROR</p>","text/html");
}