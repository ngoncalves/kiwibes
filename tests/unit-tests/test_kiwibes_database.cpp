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
#include "kiwibes_database.h"

#include "nlohmann/json.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

/*----------------------- Public Functions Definitions ------------*/
void test_database_constructor(void)
{
  KiwibesDatabase database; 
  std::vector<std::string> job_names;

  /* start with an empty list of jobs */
  database.get_all_job_names(job_names);
  ASSERT(0 == job_names.size());
}

void test_database_load_errors(void)
{
  KiwibesDatabase database; 
  std::vector<std::string> job_names;

  /* attempt to load from a location where there is no database */
  ASSERT(ERROR_NO_DATABASE_FILE == database.load("/nowhere/noplace/does/no/exist_db.json"));

  database.get_all_job_names(job_names);
  ASSERT(0 == job_names.size());

  /* attempt to load from a database with JSON syntax errrors */
  ASSERT(ERROR_JSON_PARSE_FAIL == database.load("../tests/data/databases/syntax_error.json"));

  database.get_all_job_names(job_names);
  ASSERT(0 == job_names.size());

  /* attempt to load from a database with where a job has not all data */
  ASSERT(ERROR_JOB_DESCRIPTION_INVALID == database.load("../tests/data/databases/job_incomplete_data.json"));

  database.get_all_job_names(job_names);
  ASSERT(0 == job_names.size());
}

void test_database_load(void)
{
  KiwibesDatabase database; 
  std::vector<std::string> job_names;

  /* load from an empty database is OK */
  ASSERT(ERROR_NO_ERROR == database.load("../tests/data/databases/empty_db.json"));

  database.get_all_job_names(job_names);
  ASSERT(0 == job_names.size());

  /* load two jobs */
  ASSERT(ERROR_NO_ERROR == database.load("../tests/data/databases/two_jobs.json"));

  database.get_all_job_names(job_names);

  std::vector<std::string> expected_names = { "job_1", "job_2"};

  ASSERT(expected_names == job_names);
}

void test_database_get_all_schedulable_jobs(void)
{
  KiwibesDatabase database; 
  std::vector<std::string> all_jobs;
  std::vector<std::string> schedulable_jobs;

  /* load two jobs */
  ASSERT(ERROR_NO_ERROR == database.load("../tests/data/databases/two_jobs.json"));

  database.get_all_job_names(all_jobs);
  database.get_all_schedulable_jobs(schedulable_jobs);

  std::vector<std::string> expected_all_jobs = { "job_1", "job_2"};
  std::vector<std::string> expected_schedulable_jobs = { "job_2"};

  ASSERT(expected_all_jobs == all_jobs);
  ASSERT(expected_schedulable_jobs == schedulable_jobs);
}

void test_database_get_job_description(void)
{
  KiwibesDatabase database; 
  nlohmann::json job;

  /* in case of an empty database, or an error while loading 
     the database, there should be no jobs 
   */
  const char *bad_database[] = {
    "../tests/data/databases/empty_db.json",
    "../tests/data/databases/syntax_error.json",
    "../tests/data/databases/job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)
  {
    database.load(bad_database[d]);  
    ASSERT(ERROR_JOB_NAME_UNKNOWN == database.get_job_description(job,"my job"));
  }

  /* load a valid database and retrieve:
      - a job with an unknown name 
      - the two jobs in the database 
   */
  ASSERT(ERROR_NO_ERROR == database.load("../tests/data/databases/two_jobs.json"));

  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.get_job_description(job,"my job"));

  /* "job 1" exists, verify it is correctly read out */
  std::vector<std::string> expected_program = { "/usr/bin/ls", "-hal"};

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(10                     == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(23.3456                == job["avg-runtime"].get<double>()); 
  ASSERT(0.123234               == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")        == job["schedule"].get<std::string>()); 
  ASSERT(1                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program       == job["program"].get<std::vector<std::string> >()); 

  /* these fields are reseted when first loading a database, for all jobs */ 
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>()); 
  ASSERT(0                      == job["pending-start"].get<signed int>()); 

  /* "job 2" exists, verify it is correctly read out */
  expected_program = { "/usr/bin/ls", "-h","-a","-l"};

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_2"));

  ASSERT(10                         == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(2.45                       == job["avg-runtime"].get<double>()); 
  ASSERT(0.123                      == job["var-runtime"].get<double>()); 
  ASSERT(std::string("* * 5 * 5 *") == job["schedule"].get<std::string>()); 
  ASSERT(50                         == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program           == job["program"].get<std::vector<std::string> >()); 

  /* these two fields are reseted when first loading a database, for all jobs */ 
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>()); 
  ASSERT(0                      == job["pending-start"].get<signed int>());
}

void test_database_job_started(void)
{
  KiwibesDatabase database; 
  
  /* in case of an empty database, or an error while loading 
     the database, cannot start jobs 
   */
  const char *bad_database[] = {
    "../tests/data/databases/empty_db.json",
    "../tests/data/databases/syntax_error.json",
    "../tests/data/databases/job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)
  {
    database.load(bad_database[d]);  
    ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_started("my job"));
  }

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database and attempt to change the status of jobs */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* job does not exist in a valid database */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_started("my job")); 

  /* get the job properties before it started, and compare them 
     after it was started 
   */
  nlohmann::json job;
  std::vector<std::string> expected_program = { "/usr/bin/ls", "-hal"};

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(10                     == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                    == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")        == job["schedule"].get<std::string>()); 
  ASSERT(0                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program       == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["pending-start"].get<signed int>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());   

  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  ASSERT(ERROR_NO_ERROR == database.job_started("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  /* only these parameters change */
  ASSERT(std::string("running") == job["status"].get<std::string>());
  ASSERT(now                    == job["start-time"].get<std::time_t>());   

  /* these parameters do not change */
  ASSERT(10               == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0              == job["avg-runtime"].get<double>()); 
  ASSERT(0.0              == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")  == job["schedule"].get<std::string>()); 
  ASSERT(0                == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program == job["program"].get<std::vector<std::string> >()); 
  ASSERT(0                == job["pending-start"].get<signed int>());

  /* cannot change again the job status to "running */
  ASSERT(ERROR_JOB_IS_RUNNING == database.job_started("job_1"));
}

void test_database_job_stopped(void)
{
  KiwibesDatabase database; 
  nlohmann::json  job;
  std::time_t     now;
  std::vector<std::string> expected_program = { "/usr/bin/ls", "-hal"};
  
  /* in case of an empty database, or an error while loading 
     the database, cannot stop jobs 
   */
  const char *bad_database[] = {
    "../tests/data/databases/empty_db.json",
    "../tests/data/databases/syntax_error.json",
    "../tests/data/databases/job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)
  {
    database.load(bad_database[d]);  
    ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_stopped("my job"));
  }

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database and attempt to change the status of jobs */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* job does not exist in a valid database ,cannot be stopped */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_stopped("my job")); 

  /* start the job, get its properties and compare them to when the
     job stops
   */
  now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  ASSERT(ERROR_NO_ERROR == database.job_started("job_1"));
  
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(10                     == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                    == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")        == job["schedule"].get<std::string>()); 
  ASSERT(0                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program       == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("running") == job["status"].get<std::string>());
  ASSERT(0                      == job["pending-start"].get<signed int>());
  ASSERT(now                    == job["start-time"].get<std::time_t>());   

  std::this_thread::sleep_for(std::chrono::seconds(2));

  ASSERT(ERROR_NO_ERROR == database.job_stopped("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  /* these parameters do not change when starting/stopping jobs */
  ASSERT(10               == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(std::string("")  == job["schedule"].get<std::string>()); 
  ASSERT(expected_program == job["program"].get<std::vector<std::string> >()); 
  ASSERT(0                == job["pending-start"].get<signed int>());

  /* these properties change after stopping the job. The average runtime is equal to
     the sleep time and the variance is zero, because there is only one sample
   */
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());   
  ASSERT(1                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(2.0                    == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 

  /* cannot stop the job twice */
  ASSERT(ERROR_JOB_IS_NOT_RUNNING == database.job_stopped("job_1"));
}

void test_database_delete_job(void)
{
  KiwibesDatabase database; 
  nlohmann::json job; 
  std::vector<std::string> names;

  /* in case of an empty database, or an error while loading 
     the database, cannot delete jobs 
   */
  const char *bad_database[] = {
    "../tests/data/databases/empty_db.json",
    "../tests/data/databases/syntax_error.json",
    "../tests/data/databases/job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)
  {
    database.load(bad_database[d]);  
    ASSERT(ERROR_JOB_NAME_UNKNOWN == database.delete_job("my job"));
  }

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* cannot delete a non-existing job */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.delete_job("my job"));

  /* cannot delete a job that is running */
  ASSERT(ERROR_NO_ERROR == database.job_started("job_1"));  
  ASSERT(ERROR_JOB_IS_RUNNING == database.delete_job("job_1"));

  /* delete the job and verify it cannot be retrieved afterwards */
  ASSERT(ERROR_NO_ERROR == database.job_stopped("job_1"));
  ASSERT(ERROR_NO_ERROR == database.delete_job("job_1"));  
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.get_job_description(job,"job_1"));  

  /* loading the database again, it is empty */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));
   
  database.get_all_job_names(names);
  ASSERT(0 == names.size());
}

void test_database_create_job(void)
{
  KiwibesDatabase database; 
  std::vector<std::string> expected_program = { "/usr/bin/ls"};
  nlohmann::json job;

  job["program"]     = expected_program;
  job["schedule"]    = "1 2 3 4 5 6";
  job["max-runtime"] = 9;

  /* in case of an empty database, or an error while loading 
     the database, it is possible to create jobs because the
     database is empty. First the databases are copied because
     changes to them are immediately saved
   */
  const char *bad_database[] = {
    "empty_db.json",
    "syntax_error.json",
    "job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)  
  {
    nlohmann::json created_job;
    std::vector<std::string> job_names; 

    /* copy the original database */
    {
      std::ifstream src(std::string("../tests/data/databases/") + std::string(bad_database[d]));
      std::ofstream dst(std::string("./") + std::string(bad_database[d]));

      dst << src.rdbuf();
    } 

    /* load it, then create the job */
    database.load(std::string("./") + std::string(bad_database[d]));  
    ASSERT(ERROR_NO_ERROR == database.create_job("my job",job));

    /* verify that the new job details are correct */
    ASSERT(ERROR_NO_ERROR == database.get_job_description(created_job,"my job"));

    ASSERT(9                          == created_job["max-runtime"].get<unsigned long int>()); 
    ASSERT(0.0                        == created_job["avg-runtime"].get<double>()); 
    ASSERT(0.0                        == created_job["var-runtime"].get<double>()); 
    ASSERT(std::string("1 2 3 4 5 6") == created_job["schedule"].get<std::string>()); 
    ASSERT(0                          == created_job["nbr-runs"].get<unsigned long int>()); 
    ASSERT(expected_program           == created_job["program"].get<std::vector<std::string> >()); 
    ASSERT(std::string("stopped")     == created_job["status"].get<std::string>());
    ASSERT(0                          == created_job["start-time"].get<std::time_t>());
    ASSERT(0                          == created_job["pending-start"].get<signed int>());

    database.get_all_job_names(job_names);
    ASSERT(1 == job_names.size());

    /* load it again and verify that it:
      - loads correctly
      - has one single job 
      - the job details are correct
     */
    ASSERT(ERROR_NO_ERROR == database.load(std::string("./") + std::string(bad_database[d])));  
    
    database.get_all_job_names(job_names);
    ASSERT(1 == job_names.size());

    ASSERT(ERROR_NO_ERROR == database.get_job_description(created_job,"my job"));

    ASSERT(9                          == created_job["max-runtime"].get<unsigned long int>()); 
    ASSERT(0.0                        == created_job["avg-runtime"].get<double>()); 
    ASSERT(0.0                        == created_job["var-runtime"].get<double>()); 
    ASSERT(std::string("1 2 3 4 5 6") == created_job["schedule"].get<std::string>()); 
    ASSERT(0                          == created_job["nbr-runs"].get<unsigned long int>()); 
    ASSERT(expected_program           == created_job["program"].get<std::vector<std::string> >()); 
    ASSERT(std::string("stopped")     == created_job["status"].get<std::string>());
    ASSERT(0                          == created_job["start-time"].get<std::time_t>());
    ASSERT(0                          == created_job["pending-start"].get<signed int>());
  }

  /* cannot create a job with an already existing name */
  ASSERT(ERROR_NO_ERROR == database.load("./empty_db.json"));
  ASSERT(ERROR_JOB_NAME_TAKEN == database.create_job("my job",job));

  /* cannot create a job without the following properties:
    - program
    - schedule
    - max-runtime
  */
  nlohmann::json invalid_job; 

  invalid_job["avg-runtime"]   = 21.9;
  invalid_job["var-runtime"]   = 8.76;
  invalid_job["nbr-runs"]      = 123;
  invalid_job["status"]        = "fubar";
  invalid_job["start-time"]    = 12346;
  invalid_job["pending-start"] = -46;

  ASSERT(ERROR_JOB_DESCRIPTION_INVALID == database.create_job("my other job",invalid_job));

  invalid_job["program"] = expected_program;  
  ASSERT(ERROR_JOB_DESCRIPTION_INVALID == database.create_job("my other job",invalid_job));
  
  invalid_job["schedule"] = "* * * * 3 4";  
  ASSERT(ERROR_JOB_DESCRIPTION_INVALID == database.create_job("my other job",invalid_job));

  invalid_job["max-runtime"] = 42;  
  ASSERT(ERROR_NO_ERROR == database.create_job("my other job",invalid_job));

  /* verify that only the mandatory properties are set */
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"my other job"));

  ASSERT(42                         == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                        == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                        == job["var-runtime"].get<double>()); 
  ASSERT(std::string("* * * * 3 4") == job["schedule"].get<std::string>()); 
  ASSERT(0                          == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program           == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped")     == job["status"].get<std::string>());
  ASSERT(0                          == job["start-time"].get<std::time_t>());
  ASSERT(0                          == job["pending-start"].get<signed int>());
}

void test_database_edit_job(void)
{
  KiwibesDatabase database; 
  nlohmann::json update;

  update["max-runtime"] = 1234;

  /* in case of an empty database, or an error while loading 
     the database, cannot edit jobs because they don't exist 
   */
  const char *bad_database[] = {
    "../tests/data/databases/empty_db.json",
    "../tests/data/databases/syntax_error.json",
    "../tests/data/databases/job_incomplete_data.json",
  };

  for(unsigned int d = 0; d < sizeof(bad_database)/sizeof(const char *); d++)
  {
    database.load(bad_database[d]);  
    ASSERT(ERROR_JOB_NAME_UNKNOWN == database.edit_job("my job",update));
  }

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* cannot edit a non-existing job */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.edit_job("my job",update));

  /* can only edit: 
    - program 
    - max-runtime
    - schedule

    all other properties are ignored
   */
  nlohmann::json job; 
  std::vector<std::string> expected_program = { "/usr/bin/ls", "-hal"} ;

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(10                     == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                    == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")        == job["schedule"].get<std::string>()); 
  ASSERT(0                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program       == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());     
  ASSERT(0                      == job["pending-start"].get<signed int>());

  /* update the max-runtime property of a job */
  update["max-runtime"] = 69;
  ASSERT(ERROR_NO_ERROR == database.edit_job("job_1",update));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(69                     == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                    == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                    == job["var-runtime"].get<double>()); 
  ASSERT(std::string("")        == job["schedule"].get<std::string>()); 
  ASSERT(0                      == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program       == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped") == job["status"].get<std::string>());
  ASSERT(0                      == job["start-time"].get<std::time_t>());    
  ASSERT(0                      == job["pending-start"].get<signed int>());

  /* update the schedule property of a job */
  update = { {"schedule", "* * * * 5 6"} };

  ASSERT(ERROR_NO_ERROR == database.edit_job("job_1",update));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(69                         == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                        == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                        == job["var-runtime"].get<double>()); 
  ASSERT(std::string("* * * * 5 6") == job["schedule"].get<std::string>()); 
  ASSERT(0                          == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program           == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped")     == job["status"].get<std::string>());
  ASSERT(0                          == job["start-time"].get<std::time_t>());        
  ASSERT(0                          == job["pending-start"].get<signed int>());

  /* update the program property of a job */
  update = { {"program", {"/usr/bin/echo"}} };
  expected_program = {"/usr/bin/echo"};

  ASSERT(ERROR_NO_ERROR == database.edit_job("job_1",update));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(69                         == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                        == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                        == job["var-runtime"].get<double>()); 
  ASSERT(std::string("* * * * 5 6") == job["schedule"].get<std::string>()); 
  ASSERT(0                          == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program           == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped")     == job["status"].get<std::string>());
  ASSERT(0                          == job["start-time"].get<std::time_t>());        
  ASSERT(0                          == job["pending-start"].get<signed int>());

  /* all other properties are ignored */
  update = { {"avg-runtime", 12,34}, {"var-runtime", 78.90}, {"status", "fubar"} };
  
  ASSERT(ERROR_NO_ERROR == database.edit_job("job_1",update));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));

  ASSERT(69                         == job["max-runtime"].get<unsigned long int>()); 
  ASSERT(0.0                        == job["avg-runtime"].get<double>()); 
  ASSERT(0.0                        == job["var-runtime"].get<double>()); 
  ASSERT(std::string("* * * * 5 6") == job["schedule"].get<std::string>()); 
  ASSERT(0                          == job["nbr-runs"].get<unsigned long int>()); 
  ASSERT(expected_program           == job["program"].get<std::vector<std::string> >()); 
  ASSERT(std::string("stopped")     == job["status"].get<std::string>());
  ASSERT(0                          == job["start-time"].get<std::time_t>());          
  ASSERT(0                          == job["pending-start"].get<signed int>());

  /* cannot edit a job that is running */
  ASSERT(ERROR_NO_ERROR == database.job_started("job_1"));

  update = { {"program", {"/usr/bin/echo"}} };

  ASSERT(ERROR_JOB_IS_RUNNING == database.edit_job("job_1",update));
}

void test_database_job_incr_start_requests(void)
{
  KiwibesDatabase database; 
  nlohmann::json job;

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* cannot increment the pending start requests for a non-existing job */
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_incr_start_requests("does_not_exist"));

  /* can increment the pending start requests for an existing job */
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));
  
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(1 == job["pending-start"].get<signed int>());
}

void test_database_job_decr_start_requests(void)
{
  KiwibesDatabase database; 
  nlohmann::json job;

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* decrementing the pending start requests for a non-existing job returns -1*/
  ASSERT(-1 == database.job_decr_start_requests("does_not_exist"));

  /* decrementing the pending start requests for a existing job returns the number
     of stil pending requests. When there no requests pending, it returns -1
   */
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  ASSERT(-1 == database.job_decr_start_requests("job_1"));
  ASSERT(-1 == database.job_decr_start_requests("job_1"));
  ASSERT(-1 == database.job_decr_start_requests("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  /* one request pending, decrementing the requests returns 0 */
  ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));
  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(1 == job["pending-start"].get<signed int>());

  ASSERT(0 == database.job_decr_start_requests("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  /* decrementing it again returns -1, because there are no more pending requests */
  ASSERT(-1 == database.job_decr_start_requests("job_1"));  

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  /* verify that decrement returns the correct pending count */
  ASSERT(ERROR_NO_ERROR == database.job_clear_start_requests("job_1"));

  for(unsigned int i = 0; i < 100; i++)
  {
    ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));
  }

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(100 == job["pending-start"].get<signed int>());

  for(signed int i = 100; i > 0; i--)
  {
    ASSERT((i - 1) == database.job_decr_start_requests("job_1"));

    ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
    ASSERT((i - 1) == job["pending-start"].get<signed int>());    
  }

  /* no more pending requests */
  ASSERT(-1 == database.job_decr_start_requests("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());
}

void test_database_job_clear_start_requests(void)
{
  KiwibesDatabase database; 
  nlohmann::json job;

  /* because all job changes are written to the database, we need to use
     a copy of the original database 
   */
  {
    std::ifstream src("../tests/data/databases/single_job.json");
    std::ofstream dst("./single_job.json");

    dst << src.rdbuf();
  }

  /* load a valid database */
  ASSERT(ERROR_NO_ERROR == database.load("./single_job.json"));

  /* cannot clear all pending requests for a job that does not exist*/
  ASSERT(ERROR_JOB_NAME_UNKNOWN == database.job_clear_start_requests("does_not_exist"));

  /* increment the number of pending requests */
  ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));
  ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));
  ASSERT(ERROR_NO_ERROR == database.job_incr_start_requests("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(3 == job["pending-start"].get<signed int>());

  ASSERT(ERROR_NO_ERROR == database.job_clear_start_requests("job_1"));

  ASSERT(ERROR_NO_ERROR == database.get_job_description(job,"job_1"));
  ASSERT(0 == job["pending-start"].get<signed int>());

  /* cannot decrement the pending requests */
  ASSERT(-1 == database.job_decr_start_requests("job_1"));  
}
