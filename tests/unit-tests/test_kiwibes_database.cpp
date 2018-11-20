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

#include <vector>
#include <string>

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

  std::vector<std::string> expected_names = { "job 1", "job 2"};

  ASSERT(expected_names == job_names);
}