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
Test the authentication behavior of the REST interface. 
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

def test_no_authentication_token(): 
	"""
	All REST calls, except listing jobs, fail without authentication
	"""
	# REST data calls fail without an authentication token
	for call in ["read", "write", "clear", "clear_all"]:
		if call == "read":
			result = requests.get('https://127.0.0.1:4242/rest/data/%s/test' % call,verify=False)
		elif call == "clear_all":
			result = requests.post('https://127.0.0.1:4242/rest/data/%s' % call,verify=False)
		else: 
			result = requests.post('https://127.0.0.1:4242/rest/data/%s/test' % call,verify=False)

		assert 404 == result.status_code
		assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# almost all REST job calls fail without an authentication token
	for call in ["start", "stop", "create", "edit", "delete","clear_pending","details"]:
		if call == "details":
			result = requests.get('https://127.0.0.1:4242/rest/job/%s/hello_world' % call,verify=False)
		else: 
			result = requests.post('https://127.0.0.1:4242/rest/job/%s/hello_world' % call,verify=False)

		assert 404 == result.status_code
		assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# list the existing jobs, and those that are scheduled, is possible without authentication
	for call in ["list", "scheduled"]:
		result = requests.get('https://127.0.0.1:4242/rest/jobs/%s' % call,verify=False)
		assert 200 == result.status_code

def test_invalid_authentication_token():
	"""
	Must have a valid authentication token for almost all REST calls 
	"""
	token = { "auth" : "this will fail"}

	# REST data calls fail without a valid authentication token
	for call in ["read", "write", "clear", "clear_all"]:
		if call == "read":
			result = requests.get('https://127.0.0.1:4242/rest/data/%s/test' % call,params=token,verify=False)
		elif call == "clear_all":
			result = requests.post('https://127.0.0.1:4242/rest/data/%s' % call,data=token,verify=False)
		else: 
			result = requests.post('https://127.0.0.1:4242/rest/data/%s/test' % call,data=token,verify=False)

		assert 404 == result.status_code
		assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# almost all REST job calls fail without a valid authentication token
	for call in ["start", "stop", "create", "edit", "delete","clear_pending","details"]:
		if call == "details":
			result = requests.get('https://127.0.0.1:4242/rest/job/%s/hello_world' % call,params=token,verify=False)
		else: 
			result = requests.post('https://127.0.0.1:4242/rest/job/%s/hello_world' % call,data=token,verify=False)

		assert 404 == result.status_code
		assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# list the existing jobs, and those that are scheduled, is possible without authentication
	# and providing one does not cause an error
	for call in ["list", "scheduled"]:
		result = requests.get('https://127.0.0.1:4242/rest/jobs/%s' % call,params=token,verify=False)
		assert 200 == result.status_code

def test_no_authentication_tokens_file():
	"""
	If the file with the authentication tokens is removed, then
	no authentication is possible.
	"""
	token = { "auth" : "validation-rest-calls"}

	# delete the authentication tokens
	fname = os.path.join(util.KIWIBES_HOME,"kiwibes.auth")
	os.remove(fname)	
	time.sleep(2)

	result = requests.get('https://127.0.0.1:4242/rest/job/details/hello_world',params=token,verify=False)
	assert 404 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# write an empty authentication tokens file
	empty = {}
	fname = os.path.join(util.KIWIBES_HOME,"kiwibes.auth")
	open(fname,"w").write(json.dumps(empty))
	time.sleep(2)

	result = requests.get('https://127.0.0.1:4242/rest/job/details/hello_world',params=token,verify=False)
	assert 404 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

def test_update_authentication_tokens_file():
	"""
	Add/remove a token to/from the authentication file
	"""
	# attempt to use a token which does not exist
	token = { "auth" : "not present yet"}
	fname = os.path.join(util.KIWIBES_HOME,"kiwibes.auth")

	result = requests.get('https://127.0.0.1:4242/rest/job/details/hello_world',params=token,verify=False)
	assert 404 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']

	# after adding the token, the call is allowed
	tokens = json.load(open(fname,"r"))
	tokens.append(token["auth"])
	open(fname,"w").write(json.dumps(tokens))
	time.sleep(2)

	result = requests.get('https://127.0.0.1:4242/rest/job/details/hello_world',params=token,verify=False)
	assert 200 == result.status_code
	
	# now remove the token again, the call is no longer allowed
	tokens.remove(token["auth"])
	open(fname,"w").write(json.dumps(tokens))
	time.sleep(2)

	result = requests.get('https://127.0.0.1:4242/rest/job/details/hello_world',params=token,verify=False)
	assert 404 == result.status_code
	assert result.json()["error"] == util.KIWIBES_ERRORS['ERROR_AUTHENTICATION_FAIL']