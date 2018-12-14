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
import os 

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
	result = requests.get('http://127.0.0.1:4242/jobs/list')

	assert 200 == result.status_code
	assert sorted(['hello_world','sleep_10','list_home']) == sorted(result.json())

def test_get_scheduled_jobs():
	"""
	Return a list of jobs scheduled to run periodically
	"""
	# the jobs in the REST database have no schedule
	result = requests.get('http://127.0.0.1:4242/jobs/scheduled')
	assert 200 == result.status_code
	assert 0 == len(result.json())

	# create a job with a schedule, it will be scheduled automatically
	scheduled_job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "0 * * * * *",
		"max-runtime" : 5,
	}

	result = requests.post('http://127.0.0.1:4242/job/create/list_hal',data=scheduled_job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']		

	result = requests.get('http://127.0.0.1:4242/jobs/scheduled')
	assert 200 == result.status_code
	assert ["list_hal"] == result.json()

	# create a job without a schedule, it won't be scheduled
	unscheduled_job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "",
		"max-runtime" : 5,
	}

	result = requests.post('http://127.0.0.1:4242/job/create/unscheduled_list_hal',data=unscheduled_job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']		

	result = requests.get('http://127.0.0.1:4242/jobs/scheduled')
	assert 200 == result.status_code
	assert ["list_hal"] == result.json()

	# set a schedule for an unscheduled job, it will become scheduled
	scheduled_job =  {
		"program"     : [ "/bin/sleep",'10'],
		"schedule"    : "0 0 * * * *",
		"max-runtime" : 12,
	}

	result = requests.post('http://127.0.0.1:4242/job/edit/sleep_10',data=scheduled_job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']		

	result = requests.get('http://127.0.0.1:4242/jobs/scheduled')
	assert 200 == result.status_code
	assert sorted(["list_hal","sleep_10"]) == sorted(result.json())

	# remove the schedule from a job, it will be unscheduled
	unscheduled_job =  {
		"program"     : [ "/bin/sleep",'10'],
		"schedule"    : "",
		"max-runtime" : 12,
	}

	result = requests.post('http://127.0.0.1:4242/job/edit/sleep_10',data=unscheduled_job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']		

	result = requests.get('http://127.0.0.1:4242/jobs/scheduled')
	assert 200 == result.status_code
	assert sorted(["list_hal"]) == sorted(result.json())

def test_get_job_details():
	"""
	Retrieve the details of a job
	"""
	# cannot retrieve the details of a non-existing job
	result = requests.get('http://127.0.0.1:4242/job/details/does_not_exist')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# verify that it is possible to get all information for an existing job
	result = requests.get('http://127.0.0.1:4242/job/details/sleep_10')

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

def test_post_create_job():
	"""
	Create a job 
	"""
	# cannot create a job without a description
	result = requests.post('http://127.0.0.1:4242/job/create/my_shiny_job',data={})
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_DESCRIPTION_INVALID']

	# cannot create a job using an existing name
	job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "",
		"max-runtime" : 1,
	}

	result = requests.post('http://127.0.0.1:4242/job/create/sleep_10',data=job)
	
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
		result = requests.post('http://127.0.0.1:4242/job/create/new_job',data=job)
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

	result = requests.post('http://127.0.0.1:4242/job/create/jobby_job_job',data=job)	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']	

	# verify that the job was created and that the details are correct
	result = requests.get('http://127.0.0.1:4242/jobs/list')
	assert 200 == result.status_code
	assert 'jobby_job_job' in result.json()

	result = requests.get('http://127.0.0.1:4242/job/details/jobby_job_job')

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

def test_post_edit_job(): 
	"""
	Edit an existing job 
	"""
	# cannot edit a job which does not exist
	new_job =  {
		"program"     : [ "/bin/ls",'-l','-a','-h'],
		"schedule"    : "* * * * 5 1",
		"max-runtime" : 1234,
	}	
	result = requests.post('http://127.0.0.1:4242/job/edit/does_not_exist',data=new_job)
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# the JSON data must not be empty
	result = requests.post('http://127.0.0.1:4242/job/edit/list_home',data={})
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_EMPTY_REST_REQUEST']

	# cannot edit a job that is running
	result = requests.post('http://127.0.0.1:4242/job/start/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	time.sleep(2)
	result = requests.post('http://127.0.0.1:4242/job/edit/sleep_10',data=new_job)
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_RUNNING']

	requests.post('http://127.0.0.1:4242/job/stop/sleep_10')

	# change the job parameters
	result = requests.get('http://127.0.0.1:4242/job/details/list_home')
	assert 200 == result.status_code	
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']	
	before_edit = result.json()

	result = requests.post('http://127.0.0.1:4242/job/edit/list_home',data=new_job)
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/job/details/list_home')
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

	result = requests.post('http://127.0.0.1:4242/job/edit/list_home',data=job)
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_EMPTY_REST_REQUEST']

	result = requests.get('http://127.0.0.1:4242/job/details/list_home')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	for k in after_edit.keys():
		assert result.json()[k] == after_edit[k]

	assert not "param1" in result.json().keys()

def test_post_delete_job():
	"""
	Delete an existing job 
	"""
	# cannot creating an unknown job
	result = requests.post('http://127.0.0.1:4242/job/delete/my_shiny_job')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# cannot delete a job that is running
	result = requests.post('http://127.0.0.1:4242/job/start/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	time.sleep(2)

	result = requests.post('http://127.0.0.1:4242/job/delete/sleep_10')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_RUNNING']

	result = requests.post('http://127.0.0.1:4242/job/stop/sleep_10')
	
	# delete an existing job
	result = requests.post('http://127.0.0.1:4242/job/delete/list_home')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/jobs/list')
	assert not 'list_home' in result.json()

def test_start_job():
	"""
	Start a job 
	"""
	# cannot start a job which does not exist
	result = requests.post('http://127.0.0.1:4242/job/start/does_not_exist')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# start a job and verify it has started
	assert False == os.path.isfile(os.path.join(util.KIWIBES_HOME,"hello_world.txt"))

	result = requests.post('http://127.0.0.1:4242/job/start/hello_world')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/job/details/hello_world')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	assert result.json()["status"] == "running"
	assert result.json()["start-time"] > 0 

	# cannot start a job that is already running
	result = requests.post('http://127.0.0.1:4242/job/start/hello_world')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_RUNNING']	

	# wait until the job has stopped
	time.sleep(7.0)

	result = requests.get('http://127.0.0.1:4242/job/details/hello_world')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	# not checking runtime statistics because it depends heavily on
	# the system load and one sample is not enough to meaningfully
	# compute the average and the variance
	assert result.json()["status"]     == "stopped"
	assert result.json()["start-time"] == 0.0
	assert result.json()["nbr-runs"]   == 1

	# verify that the job actually ran
	assert True == os.path.isfile(os.path.join(util.KIWIBES_HOME,"hello_world.txt"))
	
def test_stop_job():
	"""
	Stop a job 
	"""
	# cannot stop a job which does not exist
	result = requests.post('http://127.0.0.1:4242/job/stop/does_not_exist')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_NAME_UNKNOWN']

	# cannot stop a job that is not running
	result = requests.post('http://127.0.0.1:4242/job/stop/sleep_10')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_JOB_IS_NOT_RUNNING']	

	# start a job, then stopp it
	assert False == os.path.isfile(os.path.join(util.KIWIBES_HOME,"hello_world.txt"))

	result = requests.post('http://127.0.0.1:4242/job/start/hello_world')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	result = requests.get('http://127.0.0.1:4242/job/details/hello_world')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	assert result.json()["status"] == "running"
	assert result.json()["start-time"] > 0

	time.sleep(1.0)

	result = requests.post('http://127.0.0.1:4242/job/stop/hello_world')
	
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	# wait a little for the job thread to exit, then query the job details
	time.sleep(1.0)

	result = requests.get('http://127.0.0.1:4242/job/details/hello_world')
	assert 200 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_NO_ERROR']

	assert result.json()["avg-runtime"] < 10.0
	assert result.json()["status"]      == "stopped"
	assert result.json()["start-time"]  == 0.0
	assert result.json()["nbr-runs"]    == 1

	# verify that the job did not full ran
	assert False == os.path.isfile(os.path.join(util.KIWIBES_HOME,"hello_world.txt"))