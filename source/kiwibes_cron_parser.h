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

  This class implements a wrapper around the Cron expression parser.
*/
#ifndef __KIWIBES_CRON_PARSER_H__
#define __KIWIBES_CRON_PARSER_H__

#include <string>
#include <memory>
#include <chrono>
#include "ccronexpr/ccronexpr.h"

class KiwibesCronParser {

public:
  /** Class constructor

    @param  expression  string with the Cron expression
   */
  KiwibesCronParser(const std::string &expression);

  /** Class destructor
   */
  ~KiwibesCronParser();

  /** Return true if the expression is valid, false otherwise.
   */
  bool is_valid(void);

  /** Return the instant of the next occurrence for the Cron expression.
      If the expression in the constructor is invalid, it returns 0.
   */
  std::time_t next(void);

private:
  std::unique_ptr<cron_expr> cron;    /* cron expression */
  bool                       valid;   /* true if the expression is valid, false otherwise */
};

#endif
