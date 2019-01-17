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

SOURCE 	   := source
TESTS  	   := tests
BUILD  	   := build
UNIT_TESTS := $(TESTS)/unit-tests
VLD_TESTS  := $(TESTS)/validation-tests
CERTS      := $(TESTS)/data/certificates

#----------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------

all: help 

help:
	@echo '--------------------------------------------------------------------------'
	@echo 'Available targets:'
	@echo '  kiwibes      		: build the Kiwibes Automation Server'
	@echo '  ut-kiwibes   		: build and run the unit tests for Kiwibes'
	@echo '  vld-kiwibes		: run the validation tests for Kiwibes'
	@echo '  kiwibes-cert		: create the server private key and self-signed certificate'
	@echo '  kiwibes-demo		: setup and run a demo instance of Kiwibes'
	@echo '  test-python-client	: test the Python client'
	@echo '  clean        		: clear the build directory'
	@echo '  help         		: this text'
	@echo '--------------------------------------------------------------------------'

kiwibes:
	make -C $(SOURCE)

ut-kiwibes:
	python $(TESTS)/util/generate_ut.py Kiwibes $(UNIT_TESTS)
	make -C $(UNIT_TESTS)
	make -C $(UNIT_TESTS) run

vld-kiwibes: kiwibes
	-python -W ignore -m pytest -v $(VLD_TESTS)

test-python-client: kiwibes
	-python -W ignore -m pytest -v $(VLD_TESTS)/clients/python

#----------------------------------------------------------------------------
# OpenSSL Options
#----------------------------------------------------------------------------
OPENSSL_ALG  := rsa
OPENSSL_BITS := 2048
OPENSSL_OPTS := req -x509 -subj '/CN=localhost' -nodes -newkey

kiwibes-cert:
	@echo '================================================================'
	@echo ' Generating the server private key and self-signed certificate  '
	@echo '================================================================'
	-openssl $(OPENSSL_OPTS) $(OPENSSL_ALG):$(OPENSSL_BITS) -keyout $(CERTS)/kiwibes.key -out $(CERTS)/kiwibes.cert -days 365
	-openssl $(OPENSSL_ALG) -pubout -in $(CERTS)/kiwibes.key -out $(CERTS)/kiwibes.pub_key

kiwibes-demo: kiwibes kiwibes-cert
	@echo '============================================'
	@echo ' Starting a demo of Kiwibes                 '
	@echo '============================================'
	-cp $(TESTS)/data/databases/demo.json $(BUILD)/kiwibes.json
	-cp $(TESTS)/data/auth_tokens/demo.auth $(BUILD)/kiwibes.auth
	-cp $(CERTS)/* $(BUILD)/.
	-$(BUILD)/kiwibes ./$(BUILD)/ -l 2

.PHONY: clean 

clean:
	@echo '============================================'
	@echo ' Cleaning the previous builds               '
	@echo '============================================'
	-rm -rf $(BUILD)/*
	-rm -rf $(TESTS)/unit-tests/main.cpp
	-rm -rf $(TESTS)/unit-tests/ut_declarations.h