/* Kiwibes Automation Server Unit Tests
  =====================================
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
  Implements the unit tests for the database.  
 */
#include "unit_tests.h"
#include "kiwibes_jobs_manager.h"
#include "kiwibes_database.h"

#include "nlohmann/json.h"

#include <fstream>
#include <chrono>

#include <iostream>

/*----------------------- Public Functions Definitions ------------*/
void test_jobs_manager_start_job(void)
{
  KiwibesDatabase database; 
  KiwibesJobsManager manager(&database);

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
#if defined(__linux__)
    std::ifstream src("../tests/data/databases/linux_jobs.json");
#else 
    #error "OS not supported"
#endif 
    std::ofstream dst("./test_jobs.json");

    dst << src.rdbuf();
  }

  /* load a valid database and start one of its jobs */
  ASSERT(ERROR_NO_ERROR == database.load("./test_jobs.json"));

  /* attempt to start a job that does not exist */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == manager.start_job("my job"));  

  /* start a job which takes two seconds */
  std::time_t    now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  nlohmann::json job; 

  ASSERT(ERROR_NO_ERROR == manager.start_job("sleep 2"));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"sleep 2"));

  /* verify that it is started */
  ASSERT(std::string("running") == job["status"].get<std::string>());
  ASSERT(now                    == job["start-time"].get<std::time_t>());           

  /* cannot start it again */
  ASSERT(ERROR_JOB_IS_RUNNING == manager.start_job("sleep 2"));

  /* wait until it finishes */
  std::this_thread::sleep_for(std::chrono::seconds(job["max-runtime"].get<std::time_t>()));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"sleep 2"));

  /* verify that it stopped. We only verify that the average runtime is greater
     than zero because the actual value depends on how the OS manages its threads.
     The variance must be zero, though, because there is only one sample yet.
  */
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());   
  ASSERT(1                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(0.0                    <  job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
}

void test_jobs_manager_stop_job(void)
{
  KiwibesDatabase database; 
  KiwibesJobsManager manager(&database);

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
#if defined(__linux__)
    std::ifstream src("../tests/data/databases/linux_jobs.json");
#else 
    #error "OS not supported"
#endif 
    std::ofstream dst("./test_jobs.json");

    dst << src.rdbuf();
  }

  /* load a valid database and start one of its jobs */
  ASSERT(ERROR_NO_ERROR == database.load("./test_jobs.json"));

  /* attempt to stop a job that does not exist */
  ASSERT(ERROR_JOB_IS_NOT_RUNNING == manager.stop_job("my job"));  

  /* attempt to stop a job that is not running */
  ASSERT(ERROR_JOB_IS_NOT_RUNNING == manager.stop_job("sleep 2"));  

  /* start a job which takes 20 seconds */
  std::time_t    now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  nlohmann::json job; 

  ASSERT(ERROR_NO_ERROR == manager.start_job("sleep 20"));

  /* wait a little, verify it is still running and then force it to stop */
  std::this_thread::sleep_for(std::chrono::seconds(1));

  /* verify it is still running */
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"sleep 20"));
  ASSERT(std::string("running") == job["status"].get<std::string>());
  ASSERT(now                    == job["start-time"].get<std::time_t>());           

  /* force it to stop */
  ASSERT(ERROR_NO_ERROR == manager.stop_job("sleep 20"));

  /* wait a little, verify it stopped */
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"sleep 20"));
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());   
  ASSERT(1                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(0.0                    <  job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
}

void test_jobs_manager_stop_all_jobs(void)
{
  KiwibesDatabase database; 
  KiwibesJobsManager manager(&database);

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
#if defined(__linux__)
    std::ifstream src("../tests/data/databases/linux_jobs.json");
#else 
    #error "OS not supported"
#endif 
    std::ofstream dst("./test_jobs.json");

    dst << src.rdbuf();
  }

  /* load a valid database and start one of its jobs */
  ASSERT(ERROR_NO_ERROR == database.load("./test_jobs.json"));

  /* start all jobs, then stop them all */
  const char *names[] = {
    "sleep 2", "sleep 20", "sleep 10",
  };

  for(unsigned int t = 0; t < sizeof(names)/sizeof(const char *); t++)
  {
    nlohmann::json job;

    ASSERT(ERROR_NO_ERROR == manager.start_job(names[t]));  
    ASSERT(ERROR_NO_ERROR == database.get_job_description(job,names[t]));
    ASSERT(std::string("running") == job["status"].get<std::string>());
  }

  manager.stop_all_jobs();

  /* wait a little, verify all jobs efectively stopped */
  std::this_thread::sleep_for(std::chrono::seconds(1));

  for(unsigned int t = 0; t < sizeof(names)/sizeof(const char *); t++)
  {
    nlohmann::json job;

    ASSERT(ERROR_NO_ERROR == database.get_job_description(job,names[t]));
    ASSERT(std::string("stopped") == job["status"].get<std::string>());
    ASSERT(0                      == job["start-time"].get<std::time_t>());   
    ASSERT(1                      == job["nbr-runs"].get<unsigned long int>()); 
    ASSERT(0.0                    <  job["avg-runtime"].get<double>()); 
    ASSERT(0.0                    == job["var-runtime"].get<double>()); 
  }
}