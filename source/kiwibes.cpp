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
#include "kiwibes.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "NanoLog/NanoLog.hpp"

typedef struct{
  const char *option;
  const char *description;
  unsigned int *value;
} OPTIONS_T;

static OPTIONS_T kiwibes_options[] = {
  { "-l","Log level. Defaults to 0",NULL},
  { "-s","Maximum size of the log, in MB. Defaults to 1 MB",NULL},
  { "-p","HTTP server listening port. Defaults to 4242",NULL},
  { "-r","Maximum job runtime, in seconds. Defaults to 3600",NULL},
};

Kiwibes::Kiwibes()
{
  /* set the default values for the options */
  home           = NULL; 
  logMaxSize     = 1; 
  logLevel       = 0;  
  port           = 4242;
  jobMaxRuntime  = 3600;

  kiwibes_options[0].value = &logLevel;
  kiwibes_options[1].value = &logMaxSize;
  kiwibes_options[2].value = &port;
  kiwibes_options[3].value = &jobMaxRuntime;
}

Kiwibes::~Kiwibes()
{

}

void Kiwibes::init(int argc,char **argv)
{
  std::cout << "[INFO] initialization of the Kiwibes server" << std::endl;

  /* parse the command line arguments */
  parse_cmd_line(argc,argv);

  /* setup the home folder */
  setup_home();

  /* start logging */
  nanolog::initialize(nanolog::GuaranteedLogger(),
                      home + std::string("/logs/"),
                      "kiwibes.log",
                      logMaxSize);  
   
  if(0 == logLevel)
  {
    nanolog::set_log_level(nanolog::LogLevel::CRIT);
  }
  else if(1 == logLevel)
  {
    nanolog::set_log_level(nanolog::LogLevel::WARN);  
  }
  else
  {
    nanolog::set_log_level(nanolog::LogLevel::INFO);    
  }

  /* initialization is complete, when reaching here */
  std::cout << "[INFO] the Kiwibes server is initialized" << std::endl;
  LOG_INFO << "the Kiwibes server is initialized";
}

void Kiwibes::parse_cmd_line(int argc,char **argv)  
{
  if(2 > argc)
  {
    show_help();
    exit(1);
  }
  else
  {
    home = argv[1];

    for(int a = 2; a < argc; a++)
    {
      bool found = false;
      
      for(unsigned int opt = 0; opt < sizeof(kiwibes_options)/sizeof(OPTIONS_T); opt++)
      {
        if((0 == strcmp(kiwibes_options[opt].option,argv[a])) && (a + 1) < argc) 
        {
          a++;
          *(kiwibes_options[opt].value) = strtol(argv[a],NULL,10);
          found = true;
          break;  
        }
      }

      if(!found)
      {
        show_help();
        exit(1);
      }
    }
  }
}

void Kiwibes::setup_home(void)
{
  /* create, if it does not exist, the following structure:
    <home>
      +---- logs/
      +---- jobs/
   */
  std::string folder[] = {
    std::string(home),
    std::string(home) + std::string("/") + std::string("logs"),
    std::string(home) + std::string("/") + std::string("jobs"),
  };

  for(unsigned int i = 0; i < sizeof(folder)/sizeof(std::string); i++)
  {
    struct stat path;
    
    if(0 != stat(folder[i].c_str(),&path))
    {
      /* folder does not exist, create it with the permissions:
          - read, write and execute by the owner
          - read and execute by the group
          - read and execute by others 
       */
      if(0 != mkdir(folder[i].c_str(),S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
      {
        std::cout << "[ERROR] failed to create folder: " << folder[i] << std::endl;
        exit(2);
      }
      else
      {
        std::cout << "[INFO] created folder: " << folder[i] << std::endl;  
      }
    }
  }
}

void Kiwibes::show_help(void)
{
  std::cout << "Usage: kiwibes folder [OPTIONS]" << std::endl << std::endl;
  std::cout << "The first argument *must* be the full path to the Kiwibes folder." << std::endl;  
  std::cout << "The others are optional and set different working parameters:" << std::endl;
  
  for(unsigned int opt = 0; opt < sizeof(kiwibes_options)/sizeof(OPTIONS_T); opt++)
  {
    std::cout << "  " << kiwibes_options[opt].option << " UINT\t" << kiwibes_options[opt].description << std::endl;
  } 
  std::cout << std::endl;
}

int Kiwibes::run(void)
{
  /* TODO */

  return 0;
}
