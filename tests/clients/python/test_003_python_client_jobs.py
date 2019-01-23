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
Test the generic behavior of the Python client for Kiwibes
"""
import sys
sys.path.append("./clients/python")
sys.path.append("./tests/validation-tests")

import util
import requests
import json
import pytest 
import time 
import os 
import kiwibes

@pytest.fixture(autouse=True)
def setup_cleanup():
	"""
	Setup the Kiwibes server to start with a fresh database,
	then run the test and finally kill the server
	"""
	# setup the home folder and launch the Kiwibes server
	util.clean_home_folder()
	util.copy_database('rest_test_db.json')
	util.copy_auth_tokens('demo.auth')
	util.copy_ssl_certs()
	
	kiwibes = util.launch_non_blocking([util.KIWIBES_HOME,'-l','2','-d','1'])

	# run the test case
	yield

	# cleanup and backup the db
	kiwibes.terminate()

def test_start_and_stop_jobs():
	"""
	Verify that jobs can be started and stop via the client
	"""
	server = kiwibes.KiwibesServer("python-client-token",verify_cert=False)
	
	# attempt to start or stop a job that does not exist
	assert server.ERROR_JOB_NAME_UNKNOWN == server.start_job("jobby")
	assert server.ERROR_JOB_NAME_UNKNOWN == server.stop_job("jobby")

	# cannot stop a job that is not running
	assert server.ERROR_JOB_IS_NOT_RUNNING == server.stop_job("sleep_10")

	# can start a job that is already running, it will be queue
	assert server.ERROR_NO_ERROR == server.start_job("sleep_10")
	assert server.ERROR_NO_ERROR == server.start_job("sleep_10")
	assert server.ERROR_NO_ERROR == server.start_job("sleep_10")
	assert server.ERROR_NO_ERROR == server.start_job("sleep_10")

	details = server.get_job_details("sleep_10")
	assert "running" == details["status"]
	assert 3 == details["pending-start"]

	# clear the wait queue
	assert server.ERROR_NO_ERROR == server.clear_job_queue("sleep_10")

	details = server.get_job_details("sleep_10")
	assert "running" == details["status"]
	assert 0 == details["pending-start"]	

	# stop the job 
	assert server.ERROR_NO_ERROR == server.stop_job("sleep_10")
	time.sleep(1.0)

	details = server.get_job_details("sleep_10")
	assert "stopped" == details["status"]

def test_manage_jobs():
	"""
	Verify that it is possible to create, edit and manage
	jobs via the Python client
	"""
	server = kiwibes.KiwibesServer("python-client-token",verify_cert=False)

	# get the initial list of jobs
	initial_jobs = sorted(["sleep_10","list_home","hello_world"])
	assert initial_jobs == sorted(server.get_all_jobs())

	# verify that no jobs are currently scheduled
	assert [] == server.get_scheduled_jobs()

	# add a new job, without a schedule
	assert server.ERROR_NO_ERROR == server.create_job("jobby","",["/bin/ls","-hal"])
	assert "jobby" in server.get_all_jobs()

	# try to add a job with an invalid Cron schedule
	assert server.ERROR_JOB_SCHEDULE_INVALID == server.create_job("jaba","* * ? 34",["/bin/ls","-hal"])	
	assert not "jaba" in server.get_all_jobs()

	# cannot add a job if the name already exists
	assert server.ERROR_JOB_NAME_TAKEN == server.create_job("jobby","",["/bin/ls","-h"])

	# cannot delete a job that does not exist
	assert server.ERROR_JOB_NAME_UNKNOWN == server.delete_job("ASASASDSDFsf")

	# can delete a job that exists
	assert server.ERROR_NO_ERROR == server.delete_job("sleep_10")
	assert not "sleep_10" in server.get_all_jobs()

def test_edit_job_parameters():
	"""
	Verify it is possible to edit the job parameters
	"""
	server = kiwibes.KiwibesServer("python-client-token",verify_cert=False)

	# cannot modify the details of a non-existing job 
	assert server.ERROR_JOB_NAME_UNKNOWN == server.edit_job_schedule("asdasd","")
	assert server.ERROR_JOB_NAME_UNKNOWN == server.edit_job_program("asdasd",[])
	assert server.ERROR_JOB_NAME_UNKNOWN == server.edit_job_max_runtime("asdasd",0)

	# can modify the details of a job
	server.ERROR_NO_ERROR == server.edit_job_schedule("sleep_10","* * * * 5 1")
	server.ERROR_NO_ERROR == server.edit_job_max_runtime("sleep_10",20)
	server.ERROR_NO_ERROR == server.edit_job_program("sleep_10",["/bin/sleep","20"])

	details = server.get_job_details("sleep_10")
	assert "* * * * 5 1" == details["schedule"]
	assert ["/bin/sleep","20"] == details["program"]
	assert 20 == details["max-runtime"]

	assert "sleep_10" in server.get_scheduled_jobs()


