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
  Implements the unit tests for the database.  
 */
#include "unit_tests.h"
#include "kiwibes_database.h"

/*----------------------- Public Functions Definitions ------------*/
void test_database_load(void)
{
  KiwibesDatabase database; 

  /* attempt to load from a location where there is no database */
  ASSERT(ERROR_NO_DATABASE_FILE == database.load("/nowhere/noplace/does/no/exist"));
}