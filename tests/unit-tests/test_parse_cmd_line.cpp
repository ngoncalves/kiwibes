/* Kiwibes Automation Server Unit Tests
  =====================================
  Copyright 2018, Nelson Filipe Ferreira Goncalves
  nelsongoncalves@patois.eu

  License
  -------
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
  Implements the unit tests for the command line parser.  
 */
#include "unit_tests.h"
#include "kiwibes_cmd_line.h"

#include <string>

/*----------------------- Public Functions Definitions ------------*/
void test_parse_and_validate_command_line(void)
{
  /* unsuficient command line arguments */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_PARSE == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* valid command line arguments, check default values */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_NO_ERROR == parse_and_validate_command_line(options,argc,(char **)argv));    
    ASSERT(std::string("./") == *(options.home));    
    ASSERT(4242 == options.http_port);    
    ASSERT(1 == options.log_max_size);    
    ASSERT(0 == options.log_level);    
  }

  /* valid command line arguments, check parsed values */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-l","2",
      "-s","100",
      "-p","31415",
      "-d","3",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_NO_ERROR == parse_and_validate_command_line(options,argc,(char **)argv));    
    ASSERT(std::string("./") == *(options.home));    
    ASSERT(31415 == options.http_port);    
    ASSERT(100 == options.log_max_size);    
    ASSERT(2 == options.log_level);    
    ASSERT(3 == options.data_store_size);    
  }

  /* home folder does not exist */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "/nowhere/noplace/nergens/ergens",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_INV_HOME == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* log level is invalid */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-l","-1",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_INV_LOG_LEVEL == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* log level is invalid */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-l","3",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_INV_LOG_LEVEL == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* log maximum size is invalid */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-s","101",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_INV_LOG_MAX_SIZE == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* data store maximum size is invalid */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-d","101",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_INV_DATA_STORE_MAX_SIZE == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* option without value */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-s",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_PARSE == parse_and_validate_command_line(options,argc,(char **)argv));    
  }

  /* unknown option */
  {
    T_CMD_LINE_OPTIONS options;
    const char *argv[] = {
      "/bin/prog",
      "./",
      "-k","123",
      NULL,
    };
    int argc = sizeof(argv)/sizeof(char *) - 1;
    
    ASSERT(ERROR_CMDLINE_PARSE == parse_and_validate_command_line(options,argc,(char **)argv));    
  }
}