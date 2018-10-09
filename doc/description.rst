Kiwibes
=======
:author: Nelson Gonçalves
:license: Proprietary

Summary
-------

Kiwibes is a somewhat glorified cron, that provides a simple but flexible and
robust automation server over which to build a continuous build environment.

Requirements
------------

The automation server executes jobs according to a pre-defined schedule,
or on demand.

A job consists of a script or program, that is launched by the automation
server and runs fo a limited time period.

The job is described in a file, using a key-pairs, with the following keys:
	- name 
	- maximum run time
	- (optional) execution schedule
	- full path to the program with command line
	- (optional) script to execute 

The automation server provides a web interface through which the user can:
	- start a job on demand
	- stop a running job
	- reload the jobs description from disk

The automation server provides a REST interface with the same functionality
as the web interface.

The jobs are managed by managing the respective files. After the changes to
the files, the user is responsible for reloading the jobs again.


