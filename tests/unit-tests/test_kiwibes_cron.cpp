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
  Implements the unit tests for CRON expressions parser.  
 */
#include "unit_tests.h"
#include "kiwibes_cron.h"

#include <string>

/*----------------------- Public Functions Definitions ------------*/
void test_cron_valid_expressions(void)
{
  const char *valid_expressions[] = {
    /* valid expressions from here:
        https://www.freeformatter.com/cron-expression-generator-quartz.html
     */
    "* * * ? * *",                  /* every second */
    "0 * * ? * *",                  /* every minute */
    "0 15,30,45 * ? * *",           /* every hour at minutes 15, 30 and 45 */
    "0 0 0 * * ?",                  /* every day at midnight - 12am */
    "0 0 12 * * MON-FRI",           /* every Weekday at noon */
    "0 0 12 ? JAN *",               /* every day at noon in January only */
  };

  for(unsigned int t = 0; t < sizeof(valid_expressions)/sizeof(const char *); t++)
  {
    KiwibesCron cron(std::string(valid_expressions[t]));  

    ASSERT(true == cron.is_valid());
  }
}

void test_cron_invalid_expressions(void)
{
  const char *invalid_expressions[] = {
    "* * * ? *",                    /* missing field: day of the week  */
    "0 * * ? * * *",                /* not supported year field */
    "61 * * ? * *",                 /* invalid second */
    "* * * ? * SEG",                /* invalid day of the week */
    "0 0 12 1W * ?",                /* not supported: nearest week day specifier */
    "0 0 12 2L * ?",                /* not supported: last specified */
  };

  for(unsigned int t = 0; t < sizeof(invalid_expressions)/sizeof(const char *); t++)
  {
    KiwibesCron cron(std::string(invalid_expressions[t]));  

    ASSERT(false == cron.is_valid());
  }
}