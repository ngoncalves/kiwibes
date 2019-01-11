# -*- coding: utf-8 -*-
"""
Generate Unit Tests
===================
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
Provides the tools for launching, and killing, the Kiwibes
server process.
"""

import os 
import shutil
import subprocess 
import time 
import datetime

KIWIBES_ERRORS = { 
	'ERROR_NO_ERROR' 				 		: 0,
	'ERROR_CMDLINE_PARSE'			 		: 1,
	'ERROR_CMDLINE_INV_LOG_LEVEL' 	 		: 2,
	'ERROR_CMDLINE_INV_LOG_MAX_SIZE' 		: 3,
	'ERROR_CMDLINE_INV_DATA_STORE_MAX_SIZE'	: 4, 
  	'ERROR_CMDLINE_INV_HOME' 		 		: 5,
  	'ERROR_NO_DATABASE_FILE' 		 		: 6,
  	'ERROR_JSON_PARSE_FAIL' 		 		: 7,
  	'ERROR_MAIN_INTERRUPTED' 		 		: 8,
  	'ERROR_JOB_NAME_UNKNOWN' 		 		: 9,
  	'ERROR_JOB_NAME_TAKEN' 			 		: 10,
  	'ERROR_JOB_DESCRIPTION_INVALID'  		: 11,
  	'ERROR_EMPTY_REST_REQUEST' 		 		: 12,
  	'ERROR_JOB_IS_RUNNING' 			 		: 13,
  	'ERROR_JOB_IS_NOT_RUNNING' 		 		: 14,
  	'ERROR_JOB_SCHEDULE_INVALID' 	 		: 15,
  	'ERROR_PROCESS_LAUNCH_FAILED' 	 		: 16,
  	'ERROR_DATA_KEY_TAKEN'					: 17,
  	'ERROR_DATA_KEY_UNKNOWN'				: 18,	
  	'ERROR_DATA_STORE_FULL'                 : 19,
  	'ERROR_AUTHENTICATION_FAIL'             : 20,
	}

KIWIBES_HOME = './build/'
KIWIBES_BIN  = os.path.join(KIWIBES_HOME,'kiwibes')

def clean_home_folder(home=KIWIBES_HOME):
	"""
	Clear all log and JSON files in the home folder

	Arguments:
		- home : the path to the home folder, defaults to './build'
	"""
	for fname in os.listdir(home):
		if fname.endswith(".json") or fname.endswith(".txt"):
			os.remove(os.path.join(home,fname))

def copy_database(fname): 
	"""
	Copy the example database to the home folder

	Arguments:
		- fname : the name of the database
	"""
	shutil.copyfile(os.path.join('./tests','data','databases',fname),	# source 
		            os.path.join(KIWIBES_HOME,'kiwibes.json'))			# destination

def copy_auth_tokens(fname): 
	"""
	Copy the file with the authentication tokens to the home folder

	Arguments:
		- fname : the name of the database
	"""
	shutil.copyfile(os.path.join('./tests','data','auth_tokens',fname),	# source 
		            os.path.join(KIWIBES_HOME,'kiwibes.auth'))			# destination

def launch_blocking(args):
	"""
	Start the Kiwibes server process and wait for it
	to exit.

	Arguments:
		- args : list of command line arguments to pass 

	Returns:
		- tuple with the server output and its exit code
	"""
	return subprocess.call([KIWIBES_BIN] + args,stdout=subprocess.PIPE,stderr=subprocess.PIPE)

def launch_non_blocking(args,binary=KIWIBES_BIN):
	"""
	Start the Kiwibes server process in the background, with
	the given arguments.

	Arguments:
		- args   : list of arguments
		- binary : program to launch, defaults to the Kiwibes server 

	Returns:
		- handler for the process that is running
	"""
	handler = subprocess.Popen([binary] + args,stdout=subprocess.PIPE,stderr=subprocess.PIPE)	
	time.sleep(3)
	return handler 