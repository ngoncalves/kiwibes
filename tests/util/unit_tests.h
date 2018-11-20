/* Unit Tests
  ==========
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
  Simple Unit Tests framework for C++ source code applications.  
 */
#ifndef __UNIT_TESTS_H__
#define __UNIT_TESTS_H__

#include <stdint.h>
#include <setjmp.h>
#include <stdio.h>

/*--------------- Public Data ------------------------------ */
extern jmp_buf unit_tests_exception;

/*--------------- Public Functions ------------------------- */

#define ASSERT(test_value) do {\
                    if(0 == (test_value))\
                    {\
                      printf("\n******************************************"); \
                      printf("\nASSERT FAILED: "); \
                      printf("\n@(%s:%u) %s",__FILE__,__LINE__,__FUNCTION__); \
                      printf("\n******************************************\n"); \
                      longjmp(unit_tests_exception,1); \
                    } \
                  } while(0)

/*
  This macro expands runs a test case, where 
  where <X> is the name of the function that
  implements the test case.
*/
#define UT_RUN_TEST(X) do {\
                            printf("\n + " #X ": "); \
                            if(0 == setjmp(unit_tests_exception))\
                            {\
                              X ();\
                              printf("PASS");\
                            }\
                            else\
                            {\
                              printf("FAIL");\
                            }\
                          } while(0)
#endif