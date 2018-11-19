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

  This file defines the error conditions.
*/
#ifndef __KIWIBES_ERRORS_H__
#define __KIWIBES_ERRORS_H__

typedef enum{
  ERROR_NO_ERROR,                 /* not error */
  ERROR_CMD_LINE_PARSE,           /* failed to parse the command line */
  ERROR_HOME_SETUP,               /* failed to setup the home folder */
  ERROR_LOAD_DATABASE,            /* failed to load the database */ 
  ERROR_JSON_PARSE_FAIL,          /* failed to parse the JSON database file */ 
  ERROR_MAIN_INTERRUPTED,         /* caught CTRL-C */ 
  ERROR_JOB_NAME_UNKNOWN,         /* unknown job name */
  ERROR_JOB_NAME_TAKEN,           /* the job name is already taken */ 
  ERROR_JOB_DESCRIPTION_INVALID,  /* the job JSON description is invalid */
  ERROR_JOB_NOT_STARTED,          /* failed to start the job */
  ERROR_JOB_NOT_STOPPED,          /* failed to stop the job */
  ERROR_EMPTY_REST_REQUEST,       /* REST request data is empty */
  ERROR_JOB_IS_RUNNING,           /* job is running */
  ERROR_JOB_IS_NOT_RUNNING,       /* job is running */
  ERROR_JOB_SCHEDULE_INVALID,     /* the job schedule is invalid */
  ERROR_PROCESS_LAUNCH_FAILED,    /* failed to launch the process for this job */
} T_KIWIBES_ERROR;

#endif
