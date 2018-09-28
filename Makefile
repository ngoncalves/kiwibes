# Kiwibes: Automation Server
# ==========================
# Copyright 2018, Nelson Filipe Ferreira Goncalves
# nelsongoncalves@patois.eu
#
# License
# -------
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details. You should have received
# a copy of the GNU General Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#   
# Summary
# -------
# This Makefile builds all the components of the Kiwibes Automation Server.
# It also builds and runs their unit tests. 

#----------------------------------------------------------------------------
# Directory organization
#----------------------------------------------------------------------------

SOURCE := source
TESTS  := tests
BUILD  := build

#----------------------------------------------------------------------------
# Targets
#   - help         : show information about the available targets
#   - kiwibes      : build the Kiwibes Automation Server 
#   - ut-kiwibes   : build and run the unit-tests for the Kiwibes
#----------------------------------------------------------------------------

all: help 

help:
	@echo '--------------------------------------------------------------------------'
	@echo 'Available targets:'
	@echo '  kiwibes      : build the Kiwibes Automation Server'
	@echo '  ut-kiwibes   : build and run the unit-tests for the Kiwibes'
	@echo '  clean        : clear the build directory'
	@echo '  help         : this text'
	@echo '--------------------------------------------------------------------------'

kiwibes:
	make -C $(SOURCE)/

ut-kiwibes:
	make -C $(TESTS)/

.PHONY: clean

clean:
	@echo '============================================'
	@echo ' Cleaning the previous builds               '
	@echo '============================================'
	-rm -rf $(BUILD)/*