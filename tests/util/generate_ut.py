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
This script parses the files with names in the form test_<name>.cpp
and extracts their public functions. It then generates the main C++
file from where these tests are called.
"""
import sys
import os 
import glob 
import re

# the file main.c initial text
MAIN_CPP_START = '''
#include "unit_tests.h"
#include "ut_declarations.h"
#include <time.h>

#include "NanoLog/NanoLog.hpp"

// define the assert jump variable 
jmp_buf unit_tests_exception;

// helper function that displays current date and time
static void display_datetime(void)
{
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime(&rawtime);
  printf("%s",asctime(timeinfo));
}

int main(void)
{
	// ignore logging
#if defined(__linux__)
	nanolog::initialize(nanolog::GuaranteedLogger(), "/tmp/", "nanolog", 1);
#else
	#error "OS not supported !"
#endif

'''

# declarations of the unit tests 
UT_DECLARATIONS = '''
#ifndef __UT_KIWIBES_DECLARATIONS__
#define __UT_KIWIBES_DECLARATIONS__

'''

def generate_tests(name,ut_dir):
	"""
	Generate the unit tests main.

	Arguments:
		- name   : name or acronym for the project
		- ut_dir : the directory with the unit tests source

	Returns:
		- zero if successfull, non-zero value otherwise
	"""
	try:
		# create the main file and write the initial text. Then
		# parse the C files with names 'test_<suite>.c', extracting
		# all test cases and calling them from main.c
		print("[Info] Creating files in: " + ut_dir)
		main_file  = open(os.path.join(ut_dir,"main.cpp"),"w")
		decls_file = open(os.path.join(ut_dir,"ut_declarations.h"),"w")
		decls_file.write(UT_DECLARATIONS)
		main_file.write(MAIN_CPP_START)
		main_file.write('  printf("----------------------------------------\\n");\n');
		main_file.write('  printf("Unit Tests: %s\\n");\n' % name)
		main_file.write('  display_datetime();\n')
		main_file.write('  printf("----------------------------------------");\n');

		# search the source files that are named test_<name>.cpp, the test functions
		# are in the form: 'void test_<name>(void)'
		search = re.compile(r"void\s*(test_[a-zA-Z_]+[a-zA-Z0-9_]+)\s*\(\s*void\s*\)")
		for test_file in glob.glob(ut_dir + "/test_*.cpp"):
			print("  [Info] == Parsing: " + os.path.basename(test_file))
			with open(test_file) as source_file:
				suite, ext = os.path.splitext(test_file)
				suite = os.path.basename(suite).split('test_')[1].replace("_"," ").title()  
				main_file.write('  printf("\\n=== Suite %s");\n' % suite)
				for test in search.findall(source_file.read()):
					print("[Info] + adding: " + test)
					main_file.write("  UT_RUN_TEST(%s);\n" % test)
					decls_file.write("void %s(void);\n" % test)

		# now close the main file and the declarations include file
		main_file.write('  printf("\\n");\n');
		main_file.write("}\n")
		main_file.close()

		decls_file.write("\n#endif\n")
		decls_file.close();
		return 0
	except Exception as error:
		print("%s" % str(error))
		return 2

if __name__ == "__main__":
	if not 3 == len(sys.argv):
		print("Usage: ./generate_ut.py name ut")
		print("  - name : name or acronym for the project")
		print("  - ut   : relative path to the unit tests source directory")
		sys.exit(1)
	else:
		sys.exit(generate_tests(sys.argv[1],os.path.join(os.getcwd(),sys.argv[2])))