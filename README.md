# Kiwibes

Simple, efficient automation server. Cron with a REST interface.

## Features

 - Built-in limit on the maximum time a job can run
 - Straightforward REST interface for managing jobs
 - All data is stored on a human readable JSON format

## Background

This project grew out of the frustration I had with Jenkins. The causes
are not relevant, what matters is that it got me thinking on what would
be the ideal automation server. At its core, it only needs to provide:

 - a way to schedule and run jobs
 - an interface for controlling it scripting

A nice visual interface and a store with the jobs execution history
are usefull features, but not necessary.

The result is Kiwibes which, can best be described as Cron with a REST
interface. Like Cron, it provides a simple approach for scheduling the
execution of jobs. But unlike Cron, Kiwibes can be managed through a
REST interface.

The jobs started by Kiwibes can interact with it to execute other jobs.
On the same machine or on other remote machines. Without a  bucketfull of 
plugins to download, install and maintain. All of this from the comfort
of your favourite scripting language.

## User Manual

Kiwibes is a stand-alone command line application, which can be
launched as `kiwibes <home>`. The mandatory argument is the full
path to the folder where the jobs database and the application log
are stored.

Once started, Kiwibes first loads the jobs database and schedules
those with a periodic execution schedule. It then listens for incoming
calls on its REST interface.

### Security Concerns

Kiwibes runs without authentication, and by default is compiled with
HTTP only support. It is possible to support HTTPS, via the OpenSSL library,
but at the moment this is not implemented.

Furthermore, Kiwibes only enforces the maximum runtime limit of a job. It
does not prevent or control jobs from accessing the file system or accessing
any other system resources.

As a result it is __strongly discouraged__ to run Kiwibes on a system
accessible to the public.

The Kiwibes server runs with the permissions of the user that launched
it, and so do the jobs which are launched by the automation server. The
server does not run in daemon mode nor does it have the functionality
to change its permissions.

### Command Line Arguments

Called with no arguments, Kiwibes displays its usage:
```
./kiwibes 
Kiwibes Automation Server v1.0.0
Copyright (c) 2018by Nelson Filipe Ferreira Gonçalves.
All rights reserved.

Usage: kiwibes HOME [OPTIONS]

HOME is the full path to the Kiwibes working folder.
The options set different working parameters:
  -l UINT : log level, must be in the range [0,2]. Default is 0 (aka critical messages only)
  -s UINT : log maximum size in MB, must be less than 100. Default is 1 MB
  -p UINT : HTTP listening port. Default is 4242

```
Except for the first argument, all others are optional. The home folder
must exist, and it should contain a database with the known jobs. If the
home folder does not contain a database, Kiwibes will create an empty
database and then exit with an error message. Start Kiwibes a second time,
so that it will load the empty library.

### REST Interface 

The Kiwibes server manages its jobs via a REST interface, with the following
calls:

 - (POST) /start_job/{name} 
 - (POST) /stop_job/{name}
 - (POST) /create_job/{name}
 - (POST) /edit_job/{name}
 - (POST) /delete_job/{name}
 - (GET)  /jobs_list
 - (GET)  /scheduled_jobs
 - (GET)  /job/{name}

where {name} is the name of the job. It can be composed by any alphanumeric character
and the underscore sign.

All REST calls use JSON for the inout and output parameters. With the exception of
`create_job` and `edit_job`, not of the REST calls have any input parameters. In
case of error, a non-zero decimal error code is returned. The list of errors are:

 - 0  : not an error 
 - 8  : unknown job name
 - 9  : the job name is already taken
 - 10 : the job JSON description is invalid
 - 11 : REST request data is empty
 - 12 : the job is running
 - 13 : the job is not running
 - 14 : the job schedule is invalid
 - 15 : failed to launch the process for this job

The error codes in the range [1,7] indicate startup error conditions and should
not be received via the REST interface.

#### GET Calls

The `jobs_list` call returns the list with the names of all jobs known in the server,
while `scheduled_jobs` returns the name of those that are scheduled to run periodically.
The `job` call returns a copy of all of the job properties as they are currently
in the database (see below).

#### POST Calls

The `start_job`, `stop_job` and `delete_job` respectively start, stop or delete a job.
Note that a job cannot be deleted if it is running. 

The `create_job` and `edit_job` both require the same parameters:

 - program     : list (of strings) with the full path to the program and its arguments
 - schedule    : (string) Cron like string for executing the job periodically
 - max-runtime : (unsigned ineger) the maximum allowed time, in seconds, for the job to run 

If the job is not supposed to run periodically, the schedule can be an empty string.
The `create_job` call creates a job with the name given in the URL, while the `edit_job`
changes the job details. The job cannot be edited while it is running.

The job execution schedule is a Cron string with six parameters. When the job is created
with an execution schedule, it is immediately scheduled. And when edited, it is unscheduled
if the schedule is set to an empty string. And conversely, it is scheduled if the edit
call sets a valid Cron execution scheduled.

### Database Format

The database is a JSON formated list of jobs, each with an object with
the following mandatory properties:

 - program     : an array wit the job program and command line arguments
 - schedule    : a Cron string with the execution schedule
 - max-runtime : the maximum time, in seconds, that the job is allowed to run
 - avg-runtime : the average runtime of the job
 - var-runtime : the sample variance of the job runtime
 - status      : the job status, one of either "stopped" or "running"
 - start-time  : if the job is running, the time instant when it started
 - nbr-runs    : count of the number of times that the job was started

The first three properties are specified by the user when creating or 
editing the job details. The others are updated by Kiwibes when the
job is started and stopped. When the job is initially created, these
last properties are reseted.

Although the database can be edited, it is strongly advised to do so
because Kiwibes will not start with a malformed JSON file, or if a job
has missing properties. It will also provide very little information 
on the cause of the database loading error.

### Start and Stop

To start the Kiwibes server, use the command `kiwibes <home>`. And 
stop it by sending a CTRL-C or killing the process. When the Kiwibes
server is stopped, it also stops all currently running processes.

The server does not run in daemon or background mode, although it
is possible to set it up to run on the system startup as service. See
your OS specific information on how to accomplish this.

## OS Support

Currently Kiwibes only builds in Linux, although it has very few OS
specific parts. Thus it can be ported to other platforms, provided
that they support the PThread library.

## Authors

* **Nelson Gonçalves** - [ngoncalves](https://github.com/ngoncalves)

## License

This project is licensed under the GPL V3 License - see the [LICENSE.txt](LICENSE.txt) file for details

## Building

Kiwibes is written in C++, and built using Make. Currently only Linux is supported,
although it should&tm; be fairly simple to port it to Windows and Mac.

To view the list of targets, simply type `make` without any arguments.

Kiwibes makes use of the following libraries, all copied to the folder 3rd_party:

* [httplib](https://github.com/yhirose/cpp-httplib)
* [nanolog](https://github.com/Iyengar111/NanoLog)
* [json](https://github.com/nlohmann/json)
* [cCronexpr](https://github.com/staticlibs/cCronexpr)

The Cron parser uses the local time when computing the next execution instant
of a job. This is set by a compilation flag and can be disabled to use UTC instead.

### Testing

There are three sets of tests: 

 - unit tests, written in C++
 - validation tests, written in Python
 - stress tests, also written in Python

All types of tests are targets of the Makefile provided. The Python tests require
Python and the modules requests and pytest. The stress tests also need valgrind, which
measures the server performance.

## Contributing

TODO

