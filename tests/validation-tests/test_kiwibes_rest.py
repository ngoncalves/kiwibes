# -*- coding: utf-8 -*-
"""
Kiwibes Validation Tests
========================
Copyright 2018, Nelson Filipe Ferreira Gonçalves
nelsongoncalves@patois.eu

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
Test the REST interface calls. 
"""
import util
import requests
import json
import pytest 
import time 

@pytest.fixture(autouse=True)
def setup_cleanup():
	"""
	Setup the Kiwibes server to start with a fresh database,
	then run the test and finally kill the server
	"""
	# setup the home folder and launch the Kiwibes server
	util.clean_home_folder()
	util.copy_database('rest_test_db.json')
	kiwibes = util.launch_non_blocking([util.KIWIBES_HOME,'-l','2'])

	# run the test case
	yield

	# cleanup and backup the db
	kiwibes.terminate()

def test_get_invalid_url(): 
	"""
	Attempt to request an invalid URL
	"""
	# asking for an invalid URL returns an error
	result = requests.get('http://127.0.0.1:4242/does/not/exist')

	assert 404 == result.status_code
	assert u"<p>ERROR</p>" == result.text

def test_get_all_job_names(): 
	"""
	Retrieve a list with all job names
	"""
	# verify that the list of all job names can be retrieved 
	result = requests.get('http://127.0.0.1:4242/jobs_list')

	assert 200 == result.status_code
	assert sorted(['sleep_10','list_home']) == sorted(result.json())

def test_get_job_details():
	"""
	Retrieve the details of a job
	"""
	# cannot retrieve the details of a non-existing job
	result = requests.get('http://127.0.0.1:4242/job/does_not_exist')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# verify that it is possible to get all information for an existing job
	result = requests.get('http://127.0.0.1:4242/job/sleep_10')

	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']
	
	assert result.json()["program"]     == [ "/bin/sleep", "10" ]
	assert result.json()["max-runtime"] == 12
	assert result.json()["avg-runtime"] == 0.0
	assert result.json()["var-runtime"] == 0.0
	assert result.json()["schedule"]    == ""
	assert result.json()["status"]      == "stopped"
	assert result.json()["start-time"]  == 0
	assert result.json()["nbr-runs"]    == 0				

def test_create_job():
	"""
	Create a job 
	"""
	# cannot create a job without a description
	result = requests.post('http://127.0.0.1:4242/create_job/my_shiny_job',data={})
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_DESCRIPTION_INVALID']

	# cannot create a job using an existing name
	job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "",
		"max-runtime" : 1,
	}

	result = requests.post('http://127.0.0.1:4242/create_job/sleep_10',data=job)
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_TAKEN']

	# cannot create a job without the mandatory parameters
	job1 =  {
		"not-required" : 1.0,
	}

	job2 =  {
		"program" : ["/bin/ls"],
	}

	job3 =  {
		"program"  : ["/bin/ls"],
		"schedule" : "",
	}

	for job in [job1, job2, job3]:
		result = requests.post('http://127.0.0.1:4242/create_job/new_job',data=job)
		assert 200 == result.status_code
		assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_DESCRIPTION_INVALID']

	# can create a job with unnecessary parameters
	job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "* * * * 5 1",
		"max-runtime" : 1,
		"avg-runtime" : 234.5,
		"nbr-runs"    : 23,
	}

	result = requests.post('http://127.0.0.1:4242/create_job/jobby_job_job',data=job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']	

	# verify that the job was created and that the details are correct
	result = requests.get('http://127.0.0.1:4242/jobs_list')
	assert 200 == result.status_code
	assert 'jobby_job_job' in result.json()

	result = requests.get('http://127.0.0.1:4242/job/jobby_job_job')

	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']
	
	assert result.json()["program"]     == [ "/bin/ls",'-l','-a','-h']
	assert result.json()["max-runtime"] == 1
	assert result.json()["avg-runtime"] == 0.0
	assert result.json()["var-runtime"] == 0.0
	assert result.json()["schedule"]    == "* * * * 5 1"
	assert result.json()["status"]      == "stopped"
	assert result.json()["start-time"]  == 0
	assert result.json()["nbr-runs"]    == 0

def test_edit_job(): 
	"""
	Edit an existing job 
	"""
	# cannot edit a job which does not exist
	new_job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "* * * * 5 1",
		"max-runtime" : 1234,
	}	
	result = requests.post('http://127.0.0.1:4242/edit_job/does_not_exist',data=new_job)
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# the JSON data must not be empty
	result = requests.post('http://127.0.0.1:4242/edit_job/list_home',data={})
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_EMPTY_REST_REQUEST']

	# cannot edit a job that is running
	result = requests.post('http://127.0.0.1:4242/start_job/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	time.sleep(2)
	result = requests.post('http://127.0.0.1:4242/edit_job/sleep_10',data=new_job)
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_RUNNING']

	requests.post('http://127.0.0.1:4242/stop_job/sleep_10')

	# change the job parameters
	result = requests.get('http://127.0.0.1:4242/job/list_home')
	assert 200 == result.status_code	
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']	
	before_edit = result.json()

	result = requests.post('http://127.0.0.1:4242/edit_job/list_home',data=new_job)
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/job/list_home')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	assert result.json()["program"]     == new_job["program"]
	assert result.json()["max-runtime"] == new_job["max-runtime"]
	assert result.json()["schedule"]    == new_job["schedule"]	
	assert result.json()["avg-runtime"] == before_edit["avg-runtime"]
	assert result.json()["var-runtime"] == before_edit["var-runtime"]
	assert result.json()["status"]      == before_edit["status"]
	assert result.json()["start-time"]  == before_edit["start-time"]
	assert result.json()["nbr-runs"]    == before_edit["nbr-runs"]

	# must specify the 'program', 'schedule' and 'max-runtime' parameters
	after_edit = result.json()

	job = { "param1"      : "this will be ignored",
	        "status"      : "jumping",
	        "avg-runtime" : 1.0,
	        }

	result = requests.post('http://127.0.0.1:4242/edit_job/list_home',data=job)
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_EMPTY_REST_REQUEST']

	result = requests.get('http://127.0.0.1:4242/job/list_home')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	for k in after_edit.keys():
		assert result.json()[k] == after_edit[k]

	assert not "param1" in result.json().keys()

def test_delete_job():
	"""
	Delete an existing job 
	"""
	# cannot creating an unknown job
	result = requests.post('http://127.0.0.1:4242/delete_job/my_shiny_job')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# cannot delete a job that is running
	result = requests.post('http://127.0.0.1:4242/start_job/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	time.sleep(2)

	result = requests.post('http://127.0.0.1:4242/delete_job/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_RUNNING']

	result = requests.post('http://127.0.0.1:4242/stop_job/sleep_10')
	
	# delete an existing job
	result = requests.post('http://127.0.0.1:4242/delete_job/list_home')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/jobs_list')
	assert not 'list_home' in result.json()