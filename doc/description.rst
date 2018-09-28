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

The job is described with the following attributes:
	- name 
	- execution schedule
	- full path to the program or script 

The automation server provides a web interface through which the user can:
	- create jobs
	- change their execution schedule
	- start a job on demand
	- stop a running job
	- delete existing jobs

The automation server provides a REST interface with the same functionality
as the web interface.


