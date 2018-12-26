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
  Implements the unit tests for the authentication tokens.  
 */
#include "unit_tests.h"
#include "kiwibes_authentication.h"

#include "nlohmann/json.h"

#include <fstream>
#include <chrono>

/*----------------------- Public Functions Definitions ------------*/
void test_authentication_verify(void)
{
  /* create a file with some authentication tokens 
   */
  {
    nlohmann::json tokens = { "token1-abcdefghijkl", "token2-sdfsdfsdfsdf"};

    std::ofstream dst("./unit_tests.auth");
    dst << tokens;
  }  

  KiwibesAuthentication authentication("./unit_tests.auth");
  std::this_thread::sleep_for(std::chrono::seconds(1));

  /* verify that both tokens are present */
  ASSERT(true == authentication.verify_auth_token("token1-abcdefghijkl"));
  ASSERT(true == authentication.verify_auth_token("token2-sdfsdfsdfsdf"));

  /* verify that a token is not present */
  ASSERT(false == authentication.verify_auth_token("tsadasdasdfsdf"));
  ASSERT(false == authentication.verify_auth_token("token1-abcdefghijl"));
  ASSERT(false == authentication.verify_auth_token("token2-sfsdfsdfsdf"));
}

void test_authentication_update(void)
{
  /* create a file with some authentication tokens 
   */
  {
    nlohmann::json tokens = { "token1-abcdefghijkl", "token2-sdfsdfsdfsdf"};

    std::ofstream dst("./unit_tests.auth");
    dst << tokens;
  }  

  KiwibesAuthentication authentication("./unit_tests.auth");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  /* verify that both tokens are present */
  ASSERT(true == authentication.verify_auth_token("token1-abcdefghijkl"));
  ASSERT(true == authentication.verify_auth_token("token2-sdfsdfsdfsdf"));

  /* verify that a token is not present */
  ASSERT(false == authentication.verify_auth_token("tsadasdasdfsdf"));
  ASSERT(false == authentication.verify_auth_token("token1-abcdefghijl"));
  ASSERT(false == authentication.verify_auth_token("token2-sfsdfsdfsdf"));

  /* update the tokens in the file, after two seconds they should be loaded */
  {
    nlohmann::json tokens = { "token1-abcdefghijkl", "token3-aaaaaaaaaaaaaaaaa", "token4-bbbbbbbbbbbbb"};

    std::ofstream dst("./unit_tests.auth");
    dst << tokens;
  }  

  std::this_thread::sleep_for(std::chrono::seconds(2));

  /* previous tokens are not valid anymore */
  ASSERT(false == authentication.verify_auth_token("token2-sdfsdfsdfsdf"));

  /* these tokens are valid */
  ASSERT(true == authentication.verify_auth_token("token1-abcdefghijkl"));
  ASSERT(true == authentication.verify_auth_token("token3-aaaaaaaaaaaaaaaaa"));
  ASSERT(true == authentication.verify_auth_token("token4-bbbbbbbbbbbbb"));
}
