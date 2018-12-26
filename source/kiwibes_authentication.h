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

  This class implements the token base authentication mechanism.
*/
#ifndef __KIWIBES_AUTHENTICATION_H__
#define __KIWIBES_AUTHENTICATION_H__

#include "kiwibes_errors.h"

#include <set>
#include <string>
#include <mutex>
#include <thread>
#include <memory>

class KiwibesAuthentication {

public:
  /** Class constructor

    @param fname  full path to the file with the tokens
   */
  KiwibesAuthentication(const std::string &fname);

  /** Class destructor
   */
  ~KiwibesAuthentication();

  /** Verify the authentication token is valid

    @param token  the string containing the token
    @returns true if the token is valid, false otherwise
   */
  bool verify_auth_token(const std::string &token);

private:
  std::string                  *auth_fname; /* full path to the file with the authentication tokens */
  std::set<std::string>        tokens;      /* mapping of entities to tokens */
  std::mutex                   tlock;       /* exclusive tokens map */
  std::unique_ptr<std::thread> watcher;     /* thread that loads changes in the tokens file */
  bool                         watcherExit; /* flag to indicate when the watcher thread should exit */
};  

#endif
