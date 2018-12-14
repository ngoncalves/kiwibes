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
#include "kiwibes_data_store.h"

#include "NanoLog/NanoLog.hpp"

KiwibesDataStore::KiwibesDataStore(unsigned int maxSize)
{
  this->maxSize = maxSize*1024*1024;
  currSize      = 0;
}

KiwibesDataStore::~KiwibesDataStore()
{
  store.clear(); 
}
  
T_KIWIBES_ERROR KiwibesDataStore::write(const std::string &key, const std::string &value)
{
  std::lock_guard<std::mutex> lock(dslock);
  
  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(0 < store.count(key))
  {
    error = ERROR_DATA_KEY_TAKEN;
  }
  else
  {
    if((key.size() + value.size() + currSize) > maxSize)
    {
      error = ERROR_DATA_STORE_FULL;
    }
    else
    {
      store.insert(std::pair<std::string,std::string>(key,value));
      currSize += key.size() + value.size();
    }
  }

  return error;
}

T_KIWIBES_ERROR KiwibesDataStore::read(std::string &value, const std::string &key)
{
  std::lock_guard<std::mutex> lock(dslock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(0 == store.count(key))
  {
    error = ERROR_DATA_KEY_UNKNOWN;
  }
  else
  {
    std::map<std::string,std::string>::iterator iter = store.find(key);
    value = iter->second;
  }

  return error;
}

T_KIWIBES_ERROR KiwibesDataStore::clear(const std::string &key)
{
  std::lock_guard<std::mutex> lock(dslock);

  T_KIWIBES_ERROR error = ERROR_NO_ERROR;

  if(0 == store.count(key))
  {
    error = ERROR_DATA_KEY_UNKNOWN;
  }
  else
  {
    std::map<std::string,std::string>::iterator iter = store.find(key);
    if(currSize > (iter->first.size() + iter->second.size()))
    {
      currSize = 0;
    }
    else
    {
      currSize -= (iter->first.size() + iter->second.size());
    }

    store.erase(iter);
  }

  return error;
}

unsigned int KiwibesDataStore::clear_all(void)
{
  std::lock_guard<std::mutex> lock(dslock);

  unsigned int size = store.size();

  store.clear();

  return size; 
}
