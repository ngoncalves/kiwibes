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
#include "kiwibes_cron_parser.h"
#include "NanoLog/NanoLog.hpp"
#include <chrono>

KiwibesCronParser::KiwibesCronParser(const std::string &expression)
{
  cron.reset(new cron_expr);
  const char *error;

  cron_parse_expr(expression.c_str(),cron.get(),&error);
          
  if(NULL != error)
  {
    LOG_CRIT << "invalid Cron expression: '" << expression << "', error: " << error;
    valid = false;
  }
  else
  {
    valid = true;
  }
}

KiwibesCronParser::~KiwibesCronParser()
{
  /* nothing to do */ 
}

bool KiwibesCronParser::is_valid(void)
{
  return valid;
}

std::time_t KiwibesCronParser::next(void)
{
  if(true == valid)
  {
    std::time_t now  = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return cron_next(cron.get(),now);
  }
  else
  {
    return 0;
  }
}