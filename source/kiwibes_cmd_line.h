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

  This module implements the command line parsing.
*/
#ifndef __KIWIBES_CMD_LINE_H__
#define __KIWIBES_CMD_LINE_H__

#include <memory>
#include <string>

#include "kiwibes_errors.h"

/*-------------------------- Public Data Definitions -------------------------------*/
/** Command line options
 */
typedef struct{
  std::unique_ptr<std::string> home;              /* the full path to the hoem folder */
  unsigned int                 log_level;         /* the log level, must be in the range [0,2] */
  unsigned int                 log_max_size;      /* the log maximum size in MB, must be less than 100 */
  unsigned int                 https_port;        /* the HTTPS listening port */
  unsigned int                 data_store_size;   /* maximum size of the data store in MB, defaults to 10 */ 
} T_CMD_LINE_OPTIONS;

/*-------------------------- Public Function Declarations -------------------------------*/
/** Parse and validate the command line arguments
 
  @param options  on return, contains the parsed command line options
  @param argc     number of input arguments
  @param argv     the NULL terminated array of command line arguments
  @returns ERROR_NO_ERROR if sucessfull, error code otherwise
*/
T_KIWIBES_ERROR parse_and_validate_command_line(T_CMD_LINE_OPTIONS &options, int argc, char **argv);

/** Show the command line help
 */
void show_cmd_line_help(void);

#endif
