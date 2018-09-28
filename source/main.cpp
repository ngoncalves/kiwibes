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

#define KIWIBES_VERSION 		"0.0.1"
#define KIWIBES_COPYRIGHT_YEARS "2018"

std::unique_ptr<Kiwibes> server;

static void show_copyright(void)
{
	std::cout << "Kiwibes Automation Server v" << KIWIBES_VERSION << std::endl;
	std::cout << "Copyright (c) " << KIWIBES_COPYRIGHT_YEARS;
	std::cout <<  "by Nelson Filipe Ferreira Gonçalves." << std::endl;
	std::cout << "All rights reserved." << std::endl << std::endl;
}

int main(int argc, char **argv)
{
	show_copyright();

	server.reset(new Kiwibes);
	server->init(argc,argv);
  
  return server->run();
}
