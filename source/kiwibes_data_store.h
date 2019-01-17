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

  This class implements a key-value data store, which jobs can use 
  to exchange data between them.
*/
#ifndef __KIWIBES_DATA_STORE_H__
#define __KIWIBES_DATA_STORE_H__

#include "kiwibes_errors.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

class KiwibesDataStore {

public:
  /** Class constructor

    @param maxSize  the maximum size of all key-value pairs that can be stored, in MB
   */
  KiwibesDataStore(unsigned int maxSize);

  /** Clear the data store 
   */
  ~KiwibesDataStore();

  /** Associate the key with the value in the store

    @param key   the name assigned to this data
    @param value the string data

    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR write(const std::string &key, const std::string &value);

  /** Read the value associated with the key from the store

    @param value on return, it contains the the JSON formatted data
    @param key   the name assigned to this data
    
    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR read(std::string &value, const std::string &key);

  /** Return the list of all keys

    @param keys  on return contains the list of keys
  */
  void get_keys(std::vector<std::string> &keys);

  /** Clear the given key-value pair

    @param key  the name assigned to this data
    
    @return ERROR_NO_ERROR if successfull, error code otherwise
  */
  T_KIWIBES_ERROR clear(const std::string &key);

  /** Clear all stored data

   @return number of entries deleted
  */
  unsigned int clear_all(void);

private:
  std::map<std::string,std::string> store;  /* the data store, kept in memory */ 
  std::mutex   dslock;                      /* synchronize access to the data store */
  unsigned int maxSize;                     /* maximum size of the data storage, in MB */  
  unsigned int currSize;                    /* current size of the data storage, in MB */  
};

#endif
