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
  Implements the unit tests for the data store.  
 */
#include "unit_tests.h"
#include "kiwibes_data_store.h"

#include <fstream>
#include <chrono>

#include <iostream>
#include <cstring>

/*----------------------- Public Functions Definitions ------------*/
void test_data_store_write(void)
{
  // create a 1 MB data store
  KiwibesDataStore ds(1);

  // storing a key-value works
  ASSERT(ERROR_NO_ERROR == ds.write("test","my test"));

  // storing on the same key gives an error 
  ASSERT(ERROR_DATA_KEY_TAKEN == ds.write("test","my test"));

  // storing is allowed until the maximum storage size is reached 
  unsigned int currSize = strlen("test") + strlen("my test");
  for(unsigned int i = 0; i < 1024*1024; i++)
  {
    std::string key = std::string("key") + std::to_string(i);
    currSize += key.size() + strlen("my test");

    if(currSize > 1024*1024)
    {
      ASSERT(ERROR_DATA_STORE_FULL == ds.write(key,"my test"));  
      break;
    }
    else
    {
      ASSERT(ERROR_NO_ERROR == ds.write(key,"my test"));    
    }
  }
}

void test_data_store_read(void)
{
  // create a 1 MB data store
  KiwibesDataStore ds(1);

  std::string sample = std::string("sw4rwv h j45yr3q  d  ar356  36 gvXCSae   3we e ");
  ASSERT(ERROR_NO_ERROR == ds.write("test",sample));

  // reading a key that does not exist is an error
  std::string value;
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"blabla"));

  // this should work
  ASSERT(ERROR_NO_ERROR == ds.read(value,"test"));  
  ASSERT(0 == value.compare(sample));
}

void test_data_store_clear(void)
{
  // create a 1 MB data store
  KiwibesDataStore ds(1);

  ASSERT(ERROR_NO_ERROR == ds.write("test","the quick bown fox jumped over the lazy dog"));

  // erasing a key that does not exist is an error
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.clear("blabla"));  

  // erasing a key that does exist must work
  ASSERT(ERROR_NO_ERROR == ds.clear("test"));

  std::string value;
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"test"));

  // clearing a key-value pair releases space in the storage
  ASSERT(ERROR_NO_ERROR == ds.write("k",std::string(1024*1024 - 4,'c'))); // size is now 1 MB - 3
  ASSERT(ERROR_DATA_STORE_FULL == ds.write("k1",std::string(2,'b')));

  ASSERT(ERROR_NO_ERROR == ds.clear("k"));
  ASSERT(ERROR_NO_ERROR == ds.write("k1",std::string(2,'b')));
}

void test_data_store_clear_all(void)
{
  // create a 1 MB data store
  KiwibesDataStore ds(1);

  ASSERT(ERROR_NO_ERROR == ds.write("test","the quick bown fox jumped over the lazy dog"));
  ASSERT(ERROR_NO_ERROR == ds.write("test 1","the quick bown fox jumped over the lazy dog"));
  ASSERT(ERROR_NO_ERROR == ds.write("test 2","the quick bown fox jumped over the lazy dog"));
  ASSERT(ERROR_NO_ERROR == ds.write("test 3","the quick bown fox jumped over the lazy dog"));

  // clearing all returns the number of records deleted
  ASSERT(4 == ds.clear_all());  

  std::string value;
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"test"));
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"test 1"));
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"test 2"));
  ASSERT(ERROR_DATA_KEY_UNKNOWN == ds.read(value,"test 3"));
}


