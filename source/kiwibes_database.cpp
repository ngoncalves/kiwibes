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
#include "kiwibes_database.h"
#include "kiwibes_errors.h"
#include "kiwibes_cron.h"

#include "NanoLog/NanoLog.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>

KiwibesDatabase::KiwibesDatabase()
{
  dbpath.reset(new std::string(""));
  dbjobs.reset(new nlohmann::json);
}

T_KIWIBES_ERROR KiwibesDatabase::load(const std::string &fname)
{ 
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  dbpath.reset(new std::string(fname));
  dbjobs.reset(new nlohmann::json);

  std::ifstream dbfile((*dbpath));

  if(true == dbfile.is_open())
  {
    try
    {
      /* load the database and then validate its contents */
      nlohmann::json database;
      dbfile >> *dbjobs;

      for(nlohmann::json::iterator job = dbjobs->begin() ; job != dbjobs->end(); job++)
      {
        const char *expected[] = { 
          "program","max-runtime","avg-runtime","var-runtime","schedule","status",
          "start-time","nbr-runs",
        };

        for(unsigned int f = 0; f < sizeof(expected)/sizeof(const char *); f++)
        {
          if(0 == job.value().count(expected[f]))
          {
            LOG_CRIT << "job '" << job.key() << "' missing filed '" << expected[f] << "'";
            error = ERROR_JOB_DESCRIPTION_INVALID;
            break;  
          }
        }

        if(ERROR_NO_ERROR == error)
        {
          /* valid job description, reset some of the fields */
          job.value()["status"]     = "stopped";
          job.value()["start-time"] = 0;
        }     
      }
    }
    catch(nlohmann::detail::parse_error &e)
    {
      LOG_CRIT << "failed to parse JSON file: " << (*dbpath);
      LOG_CRIT << "JSON error: " << e.what();
      error = ERROR_JSON_PARSE_FAIL;
    }
  }
  else
  {
    LOG_WARN << "could not open the JSON file: " << (*dbpath);
    LOG_WARN << "database of jobs is empty";
    error = ERROR_NO_DATABASE_FILE;
  }

  /* in case of errors, reset the database contents */
  if(ERROR_NO_ERROR != error)
  {
    dbjobs.reset(new nlohmann::json);
  }

  return error; 
}

T_KIWIBES_ERROR KiwibesDatabase::save(void)
{
  std::lock_guard<std::mutex> lock(dblock);

  unsafe_save();

  return ERROR_NO_ERROR; 
} 

void KiwibesDatabase::unsafe_save(void)
{
  std::ofstream dbfile((*dbpath));
  dbfile << std::setw(4) << (*dbjobs) << std::endl;
}

T_KIWIBES_ERROR KiwibesDatabase::job_started(const std::string &name)
{
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(0 == (*dbjobs).count(name))
  {
    LOG_CRIT << "could not find job '" << name << "'";
    error = ERROR_JOB_NAME_UNKNOWN;
  }
  else if(std::string("running") == (*dbjobs)[name]["status"].get<std::string>())
  {
    LOG_WARN << "job '" << name << "' is already running, cannot start it again";
    error = ERROR_JOB_IS_RUNNING;    
  }
  else
  {
    LOG_INFO << "has started, job '" << name << "'";

    (*dbjobs)[name]["status"]     = "running";
    (*dbjobs)[name]["start-time"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  }

  return error; 
}

T_KIWIBES_ERROR KiwibesDatabase::job_stopped(const std::string &name)
{
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(0 == (*dbjobs).count(name))
  {
    LOG_CRIT << "could not find job '" << name << "'";
    error = ERROR_JOB_NAME_UNKNOWN;
  }
  else if(std::string("stopped") == (*dbjobs)[name]["status"].get<std::string>())
  {
    LOG_WARN << "job '" << name << "' is already stopped, cannot stop it again";
    error = ERROR_JOB_IS_NOT_RUNNING;    
  }
  else  
  {
    LOG_INFO << "has stopped, job '" << name << "'";
    std::time_t       now     = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::time_t       runtime = now - (*dbjobs)[name]["start-time"].get<std::time_t>();
    unsigned long int runs    = (*dbjobs)[name]["nbr-runs"].get<unsigned long int>() + 1;
    double            avg     = (*dbjobs)[name]["avg-runtime"].get<double>();
    double            var     = (*dbjobs)[name]["var-runtime"].get<double>();
    double            delta   = runtime - avg;
    
    avg += delta/runs; 
    var += delta*(runtime - avg);

    /* update the job status and its runtime statistics */
    (*dbjobs)[name]["status"]      = "stopped";
    (*dbjobs)[name]["start-time"]  = 0;
    (*dbjobs)[name]["avg-runtime"] = avg;
    (*dbjobs)[name]["var-runtime"] = var;
    (*dbjobs)[name]["nbr-runs"]    = runs;

    /* save the changes to the job description */
    unsafe_save();
  }

  return error;
}

void KiwibesDatabase::get_all_schedulable_jobs(std::vector<std::string> &jobs)
{
  std::lock_guard<std::mutex> lock(dblock);

  for(nlohmann::json::iterator job = dbjobs->begin() ; job != dbjobs->end(); job++)
  {
    KiwibesCron cron(job.value()["schedule"].get<std::string>());
    
    if(true == cron.is_valid())
    {
      jobs.push_back(job.key());
    }
  }
}

void KiwibesDatabase::get_all_job_names(std::vector<std::string> &jobs)
{
  std::lock_guard<std::mutex> lock(dblock);

  jobs.clear();

  for(nlohmann::json::iterator job = dbjobs->begin() ; job != dbjobs->end(); job++)
  {
    jobs.push_back(job.key());
  }
}

T_KIWIBES_ERROR KiwibesDatabase::get_job_description(nlohmann::json &job, const std::string &name)
{
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR          error = ERROR_NO_ERROR;
  nlohmann::json::iterator iter  = dbjobs->find(name);

  if(dbjobs->end() == iter)
  {
    error = ERROR_JOB_NAME_UNKNOWN;
  }
  else
  {
    job = iter.value();
  }

  return error;
}

T_KIWIBES_ERROR KiwibesDatabase::delete_job(const std::string &name)
{
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR          error = ERROR_NO_ERROR;
  nlohmann::json::iterator iter  = dbjobs->find(name);

  if(dbjobs->end() == iter)
  {
    error = ERROR_JOB_NAME_UNKNOWN;
  }
  else if(std::string("running") == iter.value()["status"].get<std::string>())
  {
    error = ERROR_JOB_IS_RUNNING; 
  }
  else
  {
    /* delete the job by patching the database */
    nlohmann::json remove = R"([ { "op": "remove", "path": ""} ])"_json;

    remove[0]["path"] = std::string("/") + name;

    nlohmann::json *new_db = new nlohmann::json(dbjobs->patch(remove));
    dbjobs.reset(new_db);

    unsafe_save();
  }

  return error;  
}

T_KIWIBES_ERROR KiwibesDatabase::create_job(const std::string &name, const nlohmann::json &details)
{
  /* verify the details contain the necessary information */
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if((0 == details.count("program")) || 
     (0 == details.count("schedule")) || 
     (0 == details.count("max-runtime")))
  {
    error = ERROR_JOB_DESCRIPTION_INVALID;
  }

  if(ERROR_NO_ERROR == error)
  {
    std::lock_guard<std::mutex> lock(dblock);

    nlohmann::json::iterator iter  = dbjobs->find(name);

    if(dbjobs->end() != iter)
    {
      error = ERROR_JOB_NAME_TAKEN;
    }
    else
    {
      /* set the job details */
      (*dbjobs)[name]["program"]     = details["program"].get<std::vector<std::string> >(); 
      (*dbjobs)[name]["schedule"]    = details["schedule"].get<std::string>(); 
      (*dbjobs)[name]["max-runtime"] = details["max-runtime"].get<std::time_t>(); 

      /* reset the job parameters */
      (*dbjobs)[name]["avg-runtime"] = 0.0; 
      (*dbjobs)[name]["var-runtime"] = 0.0; 
      (*dbjobs)[name]["status"]      = "stopped"; 
      (*dbjobs)[name]["start-time"]  = 0; 
      (*dbjobs)[name]["nbr-runs"]    = 0; 

      unsafe_save();
    }  
  }

  return error;
}

T_KIWIBES_ERROR KiwibesDatabase::edit_job(const std::string &name, const nlohmann::json &details)
{
  std::lock_guard<std::mutex> lock(dblock);

  T_KIWIBES_ERROR          error = ERROR_NO_ERROR;
  nlohmann::json::iterator iter  = dbjobs->find(name);

  if(dbjobs->end() == iter)
  {
    error = ERROR_JOB_NAME_UNKNOWN;
  }
  else if(std::string("running") == iter.value()["status"].get<std::string>())
  {
    error = ERROR_JOB_IS_RUNNING; 
  }  
  else
  {
    /* set the job details */
    if(1 == details.count("program"))
    {
      (*dbjobs)[name]["program"] = details["program"].get<std::vector<std::string> >();   
    }
    
    if(1 == details.count("schedule"))
    {
      (*dbjobs)[name]["schedule"] = details["schedule"].get<std::string>();   
    }
    
    if(1 == details.count("max-runtime"))
    {
      (*dbjobs)[name]["max-runtime"] = details["max-runtime"].get<unsigned long int>();   
    }

    unsafe_save();
  }  
  
  return error;
}
