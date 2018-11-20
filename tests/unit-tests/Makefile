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
# This Makefile builds, and runs, the Kiwibes unit tests. 

#----------------------------------------------------------------------------
# Directory organization
#----------------------------------------------------------------------------

SOURCE     	  	 := .
SOURCE_TEST   	 := ../../source/
SOURCE_3RD_PARTY := ../../3rd_party
BUILD         	 := ../../build
TEST_UTIL     	 := ../util

#----------------------------------------------------------------------------
# Build Tools
#----------------------------------------------------------------------------
CC       := g++
OPTIONS  := -DCRON_USE_LOCAL_TIME
INLCUDES := -I $(SOURCE_TEST) -I $(SOURCE_3RD_PARTY) -I $(TEST_UTIL)
CFLAGS   := -std=c++11 -Wall -Werror -g $(OPTIONS) $(INLCUDES)
LDFLAGS  := -pthread

#----------------------------------------------------------------------------
# Build Tools
#----------------------------------------------------------------------------
BIN     := $(BUILD)/ut-kiwibes
SOURCES := $(wildcard $(SOURCE)/*.cpp)
OBJECTS := $(patsubst $(SOURCE)/%.cpp,$(BUILD)/%.o,$(SOURCES))

SOURCES_TEST := $(SOURCE_TEST)/kiwibes_cron.cpp \
				$(SOURCE_TEST)/kiwibes_scheduler_event.cpp \
				$(SOURCE_TEST)/kiwibes_database.cpp

OBJECTS_TEST := $(patsubst $(SOURCE_TEST)/%.cpp,$(BUILD)/%.o,$(SOURCES_TEST))

SOURCES_3RD_PARTY := $(SOURCE_3RD_PARTY)/NanoLog/NanoLog.cpp \
					 $(SOURCE_3RD_PARTY)/ccronexpr/ccronexpr.c

OBJECTS_3RD_PARTY := $(BUILD)/NanoLog.o \
					 $(BUILD)/ccronexpr.o

#----------------------------------------------------------------------------
# Kiwibes Unit Tests Target
#----------------------------------------------------------------------------

all: $(BIN)

$(BIN): $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_3RD_PARTY)
	@echo '================================================================'
	@echo 'Building the Unit Tests for the Kiwibes Automation Server       '
	@echo '================================================================'
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_3RD_PARTY) -o $@ 
	-$(BIN)  

$(BUILD)/%.o: $(SOURCE)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(SOURCE_TEST)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(SOURCE_3RD_PARTY)/NanoLog/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(SOURCE_3RD_PARTY)/ccronexpr/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

print-%  : ; @echo $* = $($*)