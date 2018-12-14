/**
  Kiwibes Automation Server
  =========================
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

  See the respective header file for details.
*/
#include "kiwibes_cmd_line.h"

#include <cstring>
#include <iostream>

#if defined(__linux__)
  #include <sys/types.h>
  #include <sys/stat.h>
#else
  #error "OS not supported"
#endif

/*-------------------------- Private Function Declarations -------------------------------*/

/** Parse the command line arguments
 
  @param options  on return, contains the parsed command line options
  @param argc     number of input arguments
  @param argv     the NULL terminated array of command line arguments
  @returns ERROR_NO_ERROR if sucessfull, error code otherwise
*/
static T_KIWIBES_ERROR parse_command_line(T_CMD_LINE_OPTIONS &options, int argc, char **argv);

/** Validate the command line arguments
 
  @param options  the parsed command line options
  @returns ERROR_NO_ERROR if sucessfull, error code otherwise
*/
static T_KIWIBES_ERROR validate_command_line(T_CMD_LINE_OPTIONS &options);

/*-------------------------- Public Function Definitions   -------------------------------*/
T_KIWIBES_ERROR parse_and_validate_command_line(T_CMD_LINE_OPTIONS &options, int argc, char **argv)
{
  /* set the default options */
  options.log_level       = 0;     /* log critical messages only */
  options.log_max_size    = 1;     /* 1 MB log file size */
  options.http_port       = 4242;  /* listen on port 4242 */
  options.data_store_size = 10;    /* maximum data store size, 10 MB */

  T_KIWIBES_ERROR error = parse_command_line(options,argc,argv);

  if(ERROR_NO_ERROR == error)
  {
    error = validate_command_line(options);
  }

  return error;
}

void show_cmd_line_help(void)
{
  std::cout << "Usage: kiwibes HOME [OPTIONS]" << std::endl << std::endl;
  std::cout << "HOME is the full path to the Kiwibes working folder." << std::endl;
  std::cout << "The options set different working parameters:" << std::endl;
  std::cout << "  -l UINT : log level, must be in the range [0,2]. Default is 0 (aka critical messages only)" << std::endl;
  std::cout << "  -s UINT : log maximum size in MB, must be less than 100. Default is 1 MB" << std::endl;
  std::cout << "  -p UINT : HTTP listening port. Default is 4242" << std::endl;
  std::cout << "  -d UINT : maxium size in MB, for the data store. Default is 10 MB, must be less than 100 MB" << std::endl;
  std::cout << std::endl;
}

/*-------------------------- Private Function Definitions  -------------------------------*/
static T_KIWIBES_ERROR parse_command_line(T_CMD_LINE_OPTIONS &options, int argc, char **argv)
{
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(2 > argc)
  {
    error = ERROR_CMDLINE_PARSE;  
  }
  else
  {
    /* get the path to the home folder */
    options.home.reset(new std::string(argv[1]));
    
    for(int a = 2; a < argc; a++)
    {
      if((0 == strcmp("-l",argv[a])) && (a + 1) < argc) 
      {
        a++;
        options.log_level = strtol(argv[a],NULL,10);  
      }
      else if((0 == strcmp("-s",argv[a])) && (a + 1) < argc) 
      {
        a++;
        options.log_max_size = strtol(argv[a],NULL,10);  
      }
      else if((0 == strcmp("-p",argv[a])) && (a + 1) < argc) 
      {
        a++;
        options.http_port = strtol(argv[a],NULL,10);  
      }
      else if((0 == strcmp("-d",argv[a])) && (a + 1) < argc) 
      {
        a++;
        options.data_store_size = strtol(argv[a],NULL,10);  
      }
      else
      {
#ifndef __KIWIBES_UT__
        /* no error messages if running unit tests, because it confuses
           the output
         */
        std::cerr << "[ERROR] cannot parse option: " << argv[a] << std::endl;
#endif
        error = ERROR_CMDLINE_PARSE;    
        break;
      }
    }
  }
  
  return error;
}

static T_KIWIBES_ERROR validate_command_line(T_CMD_LINE_OPTIONS &options)
{
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(2 < options.log_level)
  {
#ifndef __KIWIBES_UT__    
    std::cerr << "[ERROR] invalid log level: " << options.log_level;
#endif
    error = ERROR_CMDLINE_INV_LOG_LEVEL;
  }
  else if(100 < options.log_max_size)
  {
#ifndef __KIWIBES_UT__
    std::cerr << "[ERROR] invalid log maxium size: " << options.log_max_size;
#endif
    error = ERROR_CMDLINE_INV_LOG_MAX_SIZE; 
  }
  else if(100 < options.data_store_size)
  {
#ifndef __KIWIBES_UT__
    std::cerr << "[ERROR] invalid data store maxium size: " << options.data_store_size;
#endif
    error = ERROR_CMDLINE_INV_DATA_STORE_MAX_SIZE; 
  }
  else
  {
    /* verify that the home folder exists */
#if defined(__linux__)
    struct stat path;
    
    if(0 != stat(options.home->c_str(),&path))
    {
#ifndef __KIWIBES_UT__
      std::cerr << "[ERROR] home folder does not exist: " << *(options.home) << std::endl;
#endif
      error = ERROR_CMDLINE_INV_HOME;
    }
#endif
  }

  return error;
}