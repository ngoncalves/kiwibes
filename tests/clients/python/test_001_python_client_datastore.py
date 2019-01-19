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
Test the datastore interaction for the Python client for Kiwibes
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

def test_datastore():
	"""
	Use the datastore of the Kiwibes server
	"""
	server = kiwibes.KiwibesServer("python-client-token",verify_cert=False)
	
	# add two key-value pairs
	assert server.ERROR_NO_ERROR == server.datastore_write("key1","value1")
	assert server.ERROR_NO_ERROR == server.datastore_write("key2","value2")

	# cannot modify an existing key
	assert server.ERROR_DATA_KEY_TAKEN == server.datastore_write("key2","another value")

	# read an existing key
	assert "value1" == server.datastore_read("key1")

	# cannot read from a non-existing key
	assert None == server.datastore_read("does not exist")

	# return a list with the current keys
	assert sorted(["key1","key2"]) == sorted(server.datastore_get_keys())

	# update the value of an existing key
	assert server.ERROR_NO_ERROR == server.datastore_update("key1","my other value")
	assert "my other value" == server.datastore_read("key1")

	# create a key pair using the update method
	assert server.ERROR_NO_ERROR == server.datastore_update("key3","value3")
	assert "value3" == server.datastore_read("key3")

	# delete a key-value pair
	assert server.ERROR_NO_ERROR == server.datastore_clear("key1")
	assert not "key1" in server.datastore_get_keys()

	# delete all key-value pairs
	assert server.ERROR_NO_ERROR == server.datastore_clear_all()
	assert [] == server.datastore_get_keys()

