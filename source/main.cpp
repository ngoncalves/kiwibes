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

  Application entry point.
*/
#include <iostream>
#include <memory>
#include "kiwibes.h"

/* Headers necessary for catching CTRL-C.
 * This is platform specific.  
 */
#if defined(__linux__)
  #include <signal.h>
  #include <unistd.h>
#else
  #error "OS not supported !"
#endif 

/*--------------------------Private Data Definitions -------------------------------*/
/** Copyright and version information
 */
#define KIWIBES_VERSION     "0.0.1"
#define KIWIBES_COPYRIGHT_YEARS "2018"

/** The Kiwibes server
 */
static std::unique_ptr<Kiwibes> server;

/*--------------------------Private Function Declarations -------------------------------*/
/** Show the copyright information
 */
static void show_copyright(void);

/** Handler for the CTRL-C signal

  @param sig  the signal that was received
 */
static void signal_handler(int sig);

/*--------------------------Public Function Definitions -------------------------------*/

int main(int argc, char **argv)
{
  /* main program steps:
    - setup the signal handlers (platform specific) 
    - show the copyright information
    - parse the command line 
    - run the server main loop
   */
#if defined(__linux__)
  struct sigaction sigHandler;

  sigHandler.sa_handler = signal_handler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;

  sigaction(SIGINT,&sigHandler,NULL);
#endif

	show_copyright();

	server.reset(new Kiwibes);
	server->init(argc,argv);
  
  return server->run();
}

/*--------------------------Private Function Definitions -------------------------------*/
static void show_copyright(void)
{
  std::cout << "Kiwibes Automation Server v" << KIWIBES_VERSION << std::endl;
  std::cout << "Copyright (c) " << KIWIBES_COPYRIGHT_YEARS;
  std::cout <<  "by Nelson Filipe Ferreira Gonçalves." << std::endl;
  std::cout << "All rights reserved." << std::endl << std::endl;
}

static void signal_handler(int sig)
{
  std::cout << "Caught interrupt signal, exiting..." << std::endl;
  exit(EXIT_ERROR_FAIL_INTERRUPTED);
}