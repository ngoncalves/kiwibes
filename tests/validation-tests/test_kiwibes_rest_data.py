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
Test the data related REST interface calls. 
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
	kiwibes = util.launch_non_blocking([util.KIWIBES_HOME,'-l','2','-d','1'])

	# run the test case
	yield

	# cleanup and backup the db
	kiwibes.terminate()

def test_post_data_write(): 
	"""
	Attempt to write data to the data store
	"""
	# write a key-value pair
	value = {"value" : "my test value"}	
	result = requests.post('http://127.0.0.1:4242/rest/data/write/test',data=value)
	assert 200 == result.status_code

	# cannot overwrite an existing key value
	value["value"] = "another test value"
	result = requests.post('http://127.0.0.1:4242/rest/data/write/test',data=value)
	assert 409 == result.status_code
	assert result.json()["code"] == util.KIWIBES_ERRORS['ERROR_DATA_KEY_TAKEN']

def test_get_data_read(): 
	"""
	Attempt to read data from the data store
	"""
	# write a key-value pair
	value = {"value" : "my test value"}	
	result = requests.post('http://127.0.0.1:4242/rest/data/write/test',data=value)
	assert 200 == result.status_code

	# cannot read from a non-existing key
	result = requests.get('http://127.0.0.1:4242/rest/data/read/does_not_exist')
	assert 404 == result.status_code
	assert result.json()["code"] == util.KIWIBES_ERRORS['ERROR_DATA_KEY_UNKNOWN']	

	# can read from an existing key
	result = requests.get('http://127.0.0.1:4242/rest/data/read/test')
	assert 200 == result.status_code
	assert result.json()["value"] == value["value"]	

def test_post_clear_data():
	"""
	Delete a key-value pair
	"""
	# write a key-value pair
	value = {"value" : "my test value"}	
	result = requests.post('http://127.0.0.1:4242/rest/data/write/test',data=value)
	assert 200 == result.status_code

	# cannot delete a non-existing key
	result = requests.post('http://127.0.0.1:4242/rest/data/clear/does_not_exist')
	assert 404 == result.status_code
	assert result.json()["code"] == util.KIWIBES_ERRORS['ERROR_DATA_KEY_UNKNOWN']	

	# can delete from an existing key
	result = requests.post('http://127.0.0.1:4242/rest/data/clear/test')
	assert 200 == result.status_code

	result = requests.get('http://127.0.0.1:4242/rest/data/read/test')
	assert 404 == result.status_code
	assert result.json()["code"] == util.KIWIBES_ERRORS['ERROR_DATA_KEY_UNKNOWN']	

def test_post_clear_all_data():
	"""
	Delete all key-value pairs
	"""
	# write a few key-value pairs
	for i in range(10):
		value = {"value" : "my test value"}	
		result = requests.post('http://127.0.0.1:4242/rest/data/write/key%d' % i,data=value)
		assert 200 == result.status_code

	# delete all existing pairs
	result = requests.post('http://127.0.0.1:4242/rest/data/clear_all')
	assert 200 == result.status_code
	assert result.json()["count"] == 10

	# nothing more to delete
	result = requests.post('http://127.0.0.1:4242/rest/data/clear_all')
	assert 200 == result.status_code
	assert result.json()["count"] == 0

def test_post_data_store_size():
	"""
	Write data on the store until it fills up
	"""
	# write key-value pairs until the maximum size is reached
	data_store_full = False 
	for i in range(1024):
		value = {"value" : 1024*"k"}
		size = len(value["value"]) + len("key%d" % i)	
		result = requests.post('http://127.0.0.1:4242/rest/data/write/key%d' % i,data=value)

		if 507 == result.status_code:
			assert result.json()["code"] == util.KIWIBES_ERRORS['ERROR_DATA_STORE_FULL']
			data_store_full = True
			break 
		else:
			assert 200 == result.status_code
	
	assert data_store_full