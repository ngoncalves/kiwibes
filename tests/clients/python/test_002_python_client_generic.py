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

def test_invalid_constructor_params(): 
	"""
	Give wrong parameters to the client constructor
	"""
	# not checking certificate for a self-signed cert, default parameters
	server = kiwibes.KiwibesServer("python-client-token")
	try:
		server.get_all_jobs()
		assert True == False
	except kiwibes.KiwibesServerError as e:
		assert e.error == server.ERROR_HTTPS_CERTS_FAIL
 
	try:
		server.start_job("sleep_10")
		assert True == False
	except kiwibes.KiwibesServerError as e:
		assert e.error == server.ERROR_HTTPS_CERTS_FAIL

	# invalid authentication token
	server = kiwibes.KiwibesServer("invalid-security-token",verify_cert=False)

	assert server.ERROR_AUTHENTICATION_FAIL == server.start_job("sleep_10")
	assert None == server.get_job_details("sleep_10")

	# server un-reachable
	server = kiwibes.KiwibesServer("python-client-token",host="somewhere",port=12345,verify_cert=False)

	try:
		server.start_job("sleep_10")
		assert True == False 
	except kiwibes.KiwibesServerError as e:
		assert e.error == server.ERROR_SERVER_NOT_FOUND
		
	try:
		server.get_job_details("sleep_10")
		assert True == False 
	except kiwibes.KiwibesServerError as e:
		assert e.error == server.ERROR_SERVER_NOT_FOUND

def test_ping_server():
	"""
	Use the ping() call to verify that the client can communicate
	with the server.
	"""
	# pinging a server that cannot be reached
	server = kiwibes.KiwibesServer("python-client-token",host="somewhere",port=12345,verify_cert=False)
	assert None == server.ping()

	# pinging a server, using an invalid authentication token
	server = kiwibes.KiwibesServer("blavlalbal",verify_cert=False)
	assert None == server.ping()	 

	# pinging a server with a self-signed certificate but explicitly checking it
	server = kiwibes.KiwibesServer("python-client-token")
	assert None == server.ping()	 
	
	# pinging a server with a correctly initialized client
	server = kiwibes.KiwibesServer("python-client-token",verify_cert=False)
	assert "pong" == server.ping()