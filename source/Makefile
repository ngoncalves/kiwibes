# Kiwibes Automation Server
# =========================
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
# This Makefile builds the Kiwibes Automation Server.

#----------------------------------------------------------------------------
# Directory organization 
#----------------------------------------------------------------------------

SOURCE := .
BUILD  := ../build
INCS   := ../3rd_party
LIBS   := -pthread $(shell pkg-config --libs openssl) -dl

#----------------------------------------------------------------------------
# Build Tools
#----------------------------------------------------------------------------
CC      := g++ 
OPTIONS := -DCPPHTTPLIB_OPENSSL_SUPPORT -DCRON_USE_LOCAL_TIME
CFLAGS  := -std=c++17 -Wall -Werror -O2 $(OPTIONS) $(shell pkg-config --cflags openssl) -I $(INCS) 

BIN     := $(BUILD)/kiwibes
SOURCES := $(wildcard $(SOURCE)/*.cpp)
OBJECTS := $(patsubst $(SOURCE)/%.cpp,$(BUILD)/%.o,$(SOURCES))

#----------------------------------------------------------------------------
# Target
#----------------------------------------------------------------------------

all: $(OBJECTS) NanoLog CCronExpr
	@echo '============================================='
	@echo 'Building the Kiwibes Automation Server       '
	@echo '============================================='
	$(CC) -o $(BIN) $(BUILD)/NanoLog.o $(BUILD)/ccronexpr.o $(OBJECTS) $(LIBS) 

NanoLog:
	$(CC) $(CFLAGS) -c -o $(BUILD)/NanoLog.o ../3rd_party/NanoLog/NanoLog.cpp

CCronExpr:
	$(CC) $(CFLAGS) -c -o $(BUILD)/ccronexpr.o ../3rd_party/ccronexpr/ccronexpr.c

$(BUILD)/%.o: $(SOURCE)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<