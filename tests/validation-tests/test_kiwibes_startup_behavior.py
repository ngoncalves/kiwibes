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
Test the startup behavior of the Kiwibes Automation Server
"""
import util

def test_start_invalid_arguments():
	"""
	Startup with invalid command line arguments 
	"""
	cmd_line_args   = [ [],								# empty command line 
						['/nowhere/does/not/exit'],		# non existing home folder
						['./','-r'],					# unknown option
						['./','-l'],					# incomplete option
						['./','-l','3'],				# invalid log level
						['./','-s','101'],				# invalid log maximum size
	                   ]

	expected_errors = [ 'ERROR_CMDLINE_PARSE',
						'ERROR_CMDLINE_INV_HOME',
						'ERROR_CMDLINE_PARSE',
						'ERROR_CMDLINE_PARSE',
						'ERROR_CMDLINE_INV_LOG_LEVEL',
						'ERROR_CMDLINE_INV_LOG_MAX_SIZE',
					  ]

	for scenario in zip(cmd_line_args,expected_errors):
		error = util.KIWIBES_ERRORS[scenario[1]]
		args  = scenario[0]
		
		assert error == util.launch_blocking(args)

def test_start_invalid_database():
	"""
	Startup with invalid database files
	"""
	# no database is available
	util.clean_home_folder()

	assert util.KIWIBES_ERRORS['ERROR_NO_DATABASE_FILE'] == util.launch_blocking([util.KIWIBES_HOME])

	# database with a JSON syntax error
	util.clean_home_folder()
	util.copy_database('syntax_error.json')

	assert util.KIWIBES_ERRORS['ERROR_JSON_PARSE_FAIL'] == util.launch_blocking([util.KIWIBES_HOME])

	# database where a job as incomplete data
	util.clean_home_folder()
	util.copy_database('job_incomplete_data.json')

	assert util.KIWIBES_ERRORS['ERROR_JOB_DESCRIPTION_INVALID'] == util.launch_blocking([util.KIWIBES_HOME])

	