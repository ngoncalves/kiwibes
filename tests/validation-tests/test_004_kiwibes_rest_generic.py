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
Test the generic behavior of the REST interface. 
"""
import util
import requests
import json
import pytest 
import time 
import os 
import signal 

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
	
	kiwibes = util.launch_non_blocking([util.KIWIBES_HOME,'-l','2'])

	# run the test case
	yield

	# cleanup
	kiwibes.kill()
	time.sleep(3.0)

def test_get_invalid_url(): 
	"""
	Attempt to request an invalid URL
	"""
	# asking for an invalid URL returns an error
	token = {"auth" : "validation-rest-calls"}
	result = requests.get('https://127.0.0.1:4242/does/not/exist',params=token,verify=False)
	assert 404 == result.status_code

def test_http_connections_not_accepted(): 
	"""
	Plain HTTP connections are not accepted
	"""
	# the list jobs REST call does not need authentication, but still must
	# be done over HTTPS. The 5 seconds timeout is more than enough given
	# that the server is running on the same machine
	try:
		requests.get('http://127.0.0.1:4242/rest/jobs/list',timeout=5)
		# should not get here
		assert False == True 
	except requests.ReadTimeout as e: 
		assert True == True

	# adding an authentication token still does won't work
	token = {"auth" : "validation-rest-calls"}
	try:
		requests.get('http://127.0.0.1:4242/rest/jobs/list',params=token,timeout=5)
		# should not get here
		assert False == True 
	except requests.ReadTimeout as e: 
		assert True == True

	# this call needs an authentication token
	try:
		requests.get('http://127.0.0.1:4242/rest/job/details/hello_world',params=token,timeout=5)
		# should not get here
		assert False == True 
	except requests.ReadTimeout as e: 
		assert True == True