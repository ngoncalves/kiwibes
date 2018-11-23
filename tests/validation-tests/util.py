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
	'ERROR_NO_ERROR' 				 : 0,
	'ERROR_CMDLINE_PARSE'			 : 1,
	'ERROR_CMDLINE_INV_LOG_LEVEL' 	 : 2,
	'ERROR_CMDLINE_INV_LOG_MAX_SIZE' : 3,
  	'ERROR_CMDLINE_INV_HOME' 		 : 4,
  	'ERROR_NO_DATABASE_FILE' 		 : 5,
  	'ERROR_JSON_PARSE_FAIL' 		 : 6,
  	'ERROR_MAIN_INTERRUPTED' 		 : 7,
  	'ERROR_JOB_NAME_UNKNOWN' 		 : 8,
  	'ERROR_JOB_NAME_TAKEN' 			 : 9,
  	'ERROR_JOB_DESCRIPTION_INVALID'  : 10,
  	'ERROR_EMPTY_REST_REQUEST' 		 : 11,
  	'ERROR_JOB_IS_RUNNING' 			 : 12,
  	'ERROR_JOB_IS_NOT_RUNNING' 		 : 13,
  	'ERROR_JOB_SCHEDULE_INVALID' 	 : 14,
  	'ERROR_PROCESS_LAUNCH_FAILED' 	 : 15,
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

def post_mortem_backup(): 
	"""
	Rename the log and database files to be kiwibes_<datetime>.{json,log}
	"""
	if os.path.isfile(os.path.join(KIWIBES_HOME,"kiwibes.log.1.txt")):
		backup = "kiwibes_%s.log" % (datetime.datetime.now().strftime("%Y%B%dT%H%M%S"))
		os.rename(os.path.join(KIWIBES_HOME,"kiwibes.log.1.txt"),os.path.join(KIWIBES_HOME,backup))

	if os.path.isfile(os.path.join(KIWIBES_HOME,"kiwibes.json")):
		backup = "kiwibes_%s.db" % (datetime.datetime.now().strftime("%Y%B%dT%H%M%S"))
		os.rename(os.path.join(KIWIBES_HOME,"kiwibes.json"),os.path.join(KIWIBES_HOME,backup))

def copy_database(fname): 
	"""
	Copy the example database to the home folder

	Arguments:
		- fname : the name of the database
	"""
	shutil.copyfile(os.path.join('./tests','data','databases',fname),	# source 
		            os.path.join(KIWIBES_HOME,'kiwibes.json'))			# destination

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

def launch_non_blocking(args):
	"""
	Start the Kiwibes server process in the background, with
	the given arguments.

	Arguments:
		- args : list of arguments

	Returns:
		- handler for the process that is running
	"""
	handler = subprocess.Popen([KIWIBES_BIN] + args,stdout=subprocess.PIPE,stderr=subprocess.PIPE)	
	time.sleep(3)
	return handler 