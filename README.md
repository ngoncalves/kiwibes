# Kiwibes

Simple, efficient automation server. Cron with a REST interface.

## Features

 - Built-in limit on the maximum time a job can run
 - Straightforward REST interface for managing jobs
 - Built in key-value data store
 - The database is a plain old JSON file

## Background

This project grew out of the frustration I had with Jenkins. The causes
are not relevant, what matters is that it got me thinking on what would
be the ideal automation server. At its core, it only needs to provide:

 - a way to schedule and run jobs
 - an interface for controlling it via scripting

A nice visual interface and the availability of the jobs execution history are
useful features, but not strictly necessary.

The result is Kiwibes which, can best be described as Cron with a REST interface.
Like Cron, it provides a simple approach for scheduling the execution of jobs.
But unlike Cron, Kiwibes can be managed through a REST interface.

The jobs started by Kiwibes can interact with the server and manage other
jobs and use the server data store. While running on the same machine as the server
or interacting with a server on another remote machine. Without a  bucketful of
potentially unsafe plugins to download, install or maintain. And all of this from
the comfort of your favorite scripting language.

## Security Concerns

Kiwibes is accessible only via HTTPS and almost all REST require a valid authentication
token as argument. The token is simply an arbitrary long string, a shared secret
between the server and the client. Each client can have its own shared secret,
different from the other clients.

The authentication tokens are loaded from the file `kiwibes.auth` in the home folder.
The server does not provide the functionality to update or change the file with
the authentication tokens. However, the server periodically monitors this file
and if its contents are modified, then it is reloaded. If the file is not present
at startup, then no authentication is possible and most REST calls are unavailable. 

Kiwibes also enforces the maximum time a job can run. But it does not prevent or
control jobs from accessing the file system or accessing any other system resources.
Therefore, and even though most REST calls need authentication, it is __strongly discouraged__
to run Kiwibes on a system accessible to the public.

The Kiwibes server runs with the permissions of the user that launched it, and so
do the jobs which are launched by the automation server. The server does not run
in daemon mode nor does it have the functionality to change its permissions.

## User Manual

Kiwibes is a stand-alone command line application, which can be launched as
`kiwibes <home>`. The mandatory argument is the full path to the folder wher
the jobs database, the HTTP server certificates and the authentication tokens are
stored. The server log is also written to this folder.

Once started, Kiwibes first loads all jobs from the database and schedules those
with a periodic execution schedule. Finally it sits and listens for incoming calls
on its REST interface.

The authentication tokens file is a JSON file containing an array of arbitrarily
long strings. These are shared secrets between the server and the clients. See
under `test/data` for examples of database files as well as authentication tokens
files.

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
  -d UINT : maxium size in MB, for the data store. Default is 10 MB, must be less than 100 MB

```
Except for the first argument, all others are optional. The home folder
must exist, and it must contain the HTTPS server certificates. If they are not
present, the server will complain and then immediately exit.

The file with the authentication tokens is optional. If not present, then most
REST calls are unavailable. 

The home folder should also contain a database with the known jobs. At startup,
if there is no database, Kiwibes will create an empty database and then exit with
an error message. Start Kiwibes a second time, so that it will load the empty
database.

### REST Interface 

The Kiwibes server manages its jobs via a REST interface, with the following
calls:

 - (POST) /rest/job/start/{name} 
 - (POST) /rest/job/stop/{name}
 - (POST) /rest/job/create/{name}
 - (POST) /rest/job/edit/{name}
 - (POST) /rest/job/delete/{name}
 - (POST) /rest/job/clear_pending/{name}
 - (GET)  /rest/job/details/{name}
 - (GET)  /rest/jobs/list
 - (GET)  /rest/jobs/scheduled
 - (POST) /rest/ping
 - (POST) /rest/data/write/{key}
 - (GET)  /rest/data/read/{key}
 - (POST) /rest/data/clear/{key}
 - (POST) /rest/data/clear_all
 - (GET)  /rest/data/keys

The `job` REST calls are used to control, create, edit or delete a job. All of
these calls require a valid authentication token, otherwise they are refused. 

The two `jobs` calls provide a way to list all of the known jobs at the server,
as well as those that are scheduled for execution. None of this calls require
an authentication token. Therefore a client without any authentication token can
use these REST calls. 

The `data` REST calls are used to write, read and clear items from the data store.
It is a simply key-value store, in which both the key and the value are arbitrarily 
long strings. Note that the total amount of data in the store is limited by default
to 10 MB. This limit can be increased up to 100 MB with the `-d` command line option.
All of the `data` REST calls require a valid authentication token.

Finally, the purpose of the `ping` REST call is for the client to verify that
it can interact with the server. The call requires a valid authentication token.
Thus, the client can use this call to verify that it can reach a given server
and also that it has a valid authentication token, for that server.

All REST calls use JSON for formatting the input and output data.

### Database Format

The database is a JSON array with a list of jobs, each an object with the following
mandatory properties:

 - program       : an array wit the job program and command line arguments
 - schedule      : a Cron string with the execution schedule
 - max-runtime   : the maximum time, in seconds, that the job is allowed to run
 - avg-runtime   : the average runtime of the job
 - var-runtime   : the sample variance of the job runtime
 - status        : the job status, one of either "stopped" or "running"
 - pending-start : number of start requests that are queued, for this job
 - start-time    : if the job is running, the time instant when it started
 - nbr-runs      : count of the number of times that the job was started

The first three properties are specified by the user when creating or editing the
job details. The others are updated by Kiwibes when the job is started and stopped.
When the job is initially created, these properties are reseted.

The job has no schedule if the respective field is either an empty string or an
invalid Cron expression. The Cron parser that is used by Kiwibes has 6 fields,
instead of the usual 5: 
  
  - seconds
  - minutes
  - hours
  - day of the month
  - month
  - year

The database can be edited by hand, but it is strongly against doing so because
Kiwibes will not start with a malformed JSON file, or if a job has missing properties.
Furthermore, in case of a malformed or incomplete database, Kiwibes provides very
little debug information.

### ServerStart and Stop

To start the Kiwibes server, use the command `kiwibes <home>`. And stop it by sending
a CTRL-C or killing the process. When the Kiwibes server is stopped, it also stops
all currently running processes.

The server does not run in daemon or background mode, although it is possible to
set it up to run on the system startup as service. See your OS specific information
on how to accomplish this.

## OS Support

Currently Kiwibes only builds in Linux, although it has very few OS specific parts.
Thus it can be ported to other platforms, provided that they support the PThread library.

## Authors

* **Nelson Gonçalves** - [ngoncalves](https://github.com/ngoncalves)

## License

This project is licensed under the GPL V3 License - see the [LICENSE.txt](LICENSE.txt) file for details

## Building

Kiwibes is written in C++, and built using Make. Currently only Linux is supported,
although it should&tm; be fairly simple to port it to Windows and Mac.

To view the list of targets, simply type `make help`.

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
 - validation tests for the Python client

All types of tests are targets in the provided Makefile. The Python tests require
the modules `requests` and `pytest`.

## Contributing

You contribute in different ways, namely by porting it to other OS's and improving
its test coverage.