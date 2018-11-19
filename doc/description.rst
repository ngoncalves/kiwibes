Kiwibes
=======
:author: Nelson Gonçalves
:license: Proprietary

Summary
-------

Kiwibes is a somewhat glorified cron, that provides a simple but flexible
automation server over which to build a continuous build environment.

Requirements
------------

The automation server executes jobs according to a pre-defined schedule,
or on demand.

A job consists of a script or program, that is launched by the automation
server and runs fo a limited time period.

The automation server provides both a web and a REST interfaces for managing
jobs, as well as starting and stopping them. 

The list of jobs is read on startup from a JSON file.

TODO:
	- UTF-8 support: not sure if it supports other languages, besides English