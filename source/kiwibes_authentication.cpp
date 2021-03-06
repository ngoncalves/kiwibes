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
#include "kiwibes_authentication.h"

#include "NanoLog/NanoLog.hpp"
#include "nlohmann/json.h"

#include <chrono>
#include <fstream>
#include <iostream>


#if defined(__linux__)
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
#else 
  #error "OS not supported !"
#endif 

/*----------------- Private Functions Declarations -----------------------------*/

/** Watcher Thread 

  This function monitors the authentication tokens file and loads it whenever
  it is modified.

  @param fname        full path to the authentication tokens file
  @param tokens       map of job names to tokens
  @param tlock        access lock for the map of tokens
  @param exitFlag     set to true when the thread should exit 
 */
static void watcher_thread(const std::string *fname,
                           std::set<std::string> *tokens,
                           std::mutex *tlock,
                           bool *exitFlag);

/*--------------- Class Implemementation --------------------------------------*/  
KiwibesAuthentication::KiwibesAuthentication(const std::string &fname)
{
  auth_fname = new std::string(fname);

  /* start the watcher thread */
  watcherExit = false;

  /* start the watcher thread */
  watcher.reset(new std::thread(watcher_thread,auth_fname,&tokens,&tlock,&watcherExit)); 
}

KiwibesAuthentication::~KiwibesAuthentication()
{
  watcherExit = true;

  LOG_INFO << "waiting for the authentication watcher thread to finish"; 
  watcher->join();
  LOG_INFO << "the watcher authentication thread has finished";

  delete auth_fname;
}

bool KiwibesAuthentication::verify_auth_token(const std::string &token)
{
  std::lock_guard<std::mutex> lock(tlock);
  
  return (tokens.end() != tokens.find(token));
}

/*----------------- Private Functions Definitions -----------------------------*/
static void watcher_thread(const std::string *fname, std::set<std::string> *tokens, std::mutex *tlock, bool *exitFlag)
{
  std::time_t last_modified = 0;
  bool        has_warned    = false;

  while(false == *exitFlag)
  {
    tlock->lock();

#if defined(__linux__)
    struct stat result;

    if(0 == stat(fname->c_str(),&result))
    {
      has_warned = false;
      
      if(last_modified < result.st_mtime)
      {
        /* load the tokens */
        tokens->clear();
        std::ifstream auth_file((*fname));

        if(true == auth_file.is_open())
        {
          try
          {
              nlohmann::json auth_tokens;
              auth_file >> auth_tokens;

              for(nlohmann::json::iterator item = auth_tokens.begin() ; item != auth_tokens.end(); item++)
              {
                tokens->insert(item->get<std::string>());
              }

              LOG_CRIT << "loaded authentication tokens from JSON file: " << (*fname);
          }
          catch(nlohmann::detail::parse_error &e)
          {
            LOG_CRIT << "failed to parse authentication JSON file: " << (*fname);
            LOG_CRIT << "JSON error: " << e.what();
          }
        }
        else
        {
          LOG_WARN << "could not open the authentication JSON file: " << (*fname);
          LOG_WARN << "no authentication tokens have been loaded";
        }

        last_modified = result.st_mtime;
      }
    }
    else
    {
      /* clear all tokens since the file was probably removed */
      tokens->clear();
      if(false == has_warned)
      {
        LOG_WARN << "failed to locate authentication JSON file: " << (*fname);
        has_warned = true;
      }
    }
#endif

    tlock->unlock();

    /* wait a little before checking again */
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}