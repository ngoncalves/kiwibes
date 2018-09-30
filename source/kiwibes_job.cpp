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
#include "kiwibes_job.h"
#include "NanoLog/NanoLog.hpp"
#include <fstream>
#include <regex>
#include <iostream>

KiwibesJob::KiwibesJob()
{
  maxRuntime = 0;
}

KiwibesJob::~KiwibesJob()
{

}

bool KiwibesJob::load(const char *fname)
{
  bool success = true;

  this->fname.reset(new std::string(fname));

  std::ifstream infile(fname);
  
  if(infile.is_open())
  {
    LOG_INFO  << "loading job description file: " << fname;

    std::regex reName("\\s*name\\s*:\\s*(.*)",std::regex_constants::ECMAScript);
    std::regex reSchedule("\\s*schedule\\s*:\\s*(.*)",std::regex_constants::ECMAScript);
    std::regex reExecutable("\\s*executable\\s*:\\s*(.*)",std::regex_constants::ECMAScript);
    std::regex reMaxRuntime("\\s*max_runtime\\s*:\\s*([0-9]+)",std::regex_constants::ECMAScript);
    std::regex reComment("\\s*#",std::regex_constants::ECMAScript);
    std::regex reScriptStart("\\s*---",std::regex_constants::ECMAScript);

    for(std::string line; std::getline(infile,line); )
    {
      std::smatch matches;
      if(std::regex_search(line,matches,reName))
      {
        name.reset(new std::string(matches[1]));
      }
      else if(std::regex_search(line,matches,reSchedule))
      {
        schedule.reset(new std::string(matches[1]));  
      } 
      else if(std::regex_search(line,matches,reExecutable))
      {
        executable.reset(new std::string(matches[1]));
      } 
      else if(std::regex_search(line,matches,reMaxRuntime))
      {
        maxRuntime = (unsigned int)strtol(matches[1].str().c_str(),NULL,10);
      } 
      else if(std::regex_search(line,matches,reComment))
      {
        /* ignore comments */
      }
      else if(std::regex_search(line,matches,reScriptStart))
      {
        /* stop parsing */
        break;
      }
      else
      {
        LOG_CRIT  << "error parsing key-value pair: '" << line << "'";
        success = false;
        break; 
      }
    }

    if(name == nullptr)
    {
      LOG_CRIT  << "no 'name' given, while parsing: " << fname;
      success = false;
    }
    else if(executable == nullptr)
    {
      LOG_CRIT  << "no 'executable' given, while parsing: " << fname;
      success = false;
    }
    else if(0 == maxRuntime)
    {
      LOG_CRIT  << "no 'max_runtime' given, while parsing: " << fname;
      success = false;
    }
  }
  else
  {
    LOG_CRIT  << "could not read job description file: " << fname;
    success = false;
  }

  return success;
}