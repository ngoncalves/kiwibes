# -*- coding: utf-8 -*-
"""
Kiwibes Client
==============
Copyright 2018, Nelson Filipe Ferreira Gonçalves
nelsongoncalves@patois.eu

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
Provides a client for the Kiwibes Automation Server
"""
import requests 
import logging 

class KiwibesServerError(Exception):
    """
    Representation of an exception when interacting with
    the Kiwibes server.
    """

    def __init__(self,message,error):
        """
        Initialize the exception with diagnostic information

        Arguments:
            - message : test message to display
            - erro    : the numeric error code
        """
        super(KiwibesServerError,self).__init__(message)
        self.error = error

    def __str__(self):
        return "Kiwibes Error:(%d) %s" % (self.error,self.message)

class KiwibesServer():
    """
    Provides an abstraction to interact with the Kiwibes server.
    A KiwibesServerError exception is thrown when:
        - the server cannot be reached
        - the server certificate is self-signed and verify_cert is set to True
    """

    # definition of the error codes that might be returned by the REST interface
    ERROR_NO_ERROR                = 0
    ERROR_JOB_NAME_UNKNOWN        = 9
    ERROR_JOB_NAME_TAKEN          = 10
    ERROR_JOB_DESCRIPTION_INVALID = 11
    ERROR_EMPTY_REST_REQUEST      = 12
    ERROR_JOB_IS_RUNNING          = 13
    ERROR_JOB_IS_NOT_RUNNING      = 14
    ERROR_JOB_SCHEDULE_INVALID    = 15
    ERROR_PROCESS_LAUNCH_FAILED   = 16
    ERROR_DATA_KEY_TAKEN          = 17
    ERROR_DATA_KEY_UNKNOWN        = 18
    ERROR_DATA_STORE_FULL         = 19
    ERROR_AUTHENTICATION_FAIL     = 20
    ERROR_HTTPS_CERTS_FAIL        = 21
    ERROR_SERVER_NOT_FOUND        = 22
   
    def __init__(self,auth_token,host='localhost',port=4242,verify_cert=True):
        """
        Initializes the class with the host and port
        of the Kiwibes server
    
        Arguments:
            - auth_token  : string with the authentication token
            - host        :  the host address, defaults to 'localhost'
            - port        :  the host listening port, defaults to 4242 
            - verify_cert : verify the SSL certificate, defaults to True
        """
        self.token       = auth_token
        self.url         = 'https://%s:%d' % (host,port)
        self.verify_cert = verify_cert

    def __post(self,route,data): 
        """
        POST method for the given HTTP route.
        In case of SSL error or it cannot reach the host, it
        throws a KiwibesServerError exception.

        Arguments:
            - route : the server HTTP route 
            - data  : the POST call data 

        Return:
            - Kiwibes server error
        """
        try: 
            path = self.url + route
            result = requests.post(path,data=data,verify=self.verify_cert)
            if 200 != result.status_code:
                logging.error("POST - %s: (%d) %s" % (path,result.json()["error"],result.json()["message"])) 
                return result.json()["error"]
            else:
                return self.ERROR_NO_ERROR
        except requests.exceptions.SSLError:
            message = "Invalid or self-signed Kiwibes server certificate !"
            logging.error(message)    
            raise KiwibesServerError(message,self.ERROR_HTTPS_CERTS_FAIL)
        except requests.exceptions.ConnectionError:
            message = "failed to connect to Kiwibes server at: %s" % self.url
            logging.error(message)    
            raise KiwibesServerError(message,self.ERROR_SERVER_NOT_FOUND)

    def __get(self,route,data): 
        """
        GET method for the given HTTP route.
        In case of SSL error or it cannot reach the host, it
        throws a KiwibesServerError exception.

        Arguments:
            - route : the server HTTP route 
            - data  : the POST call data 

        Returns:
            - the response, None in case of error
        """
        try:
            path = self.url + route
            result = requests.get(path,params=data,verify=self.verify_cert)
            if 200 != result.status_code:
                logging.error("GET - %s: (%d) %s" % (path,result.status_code,result.text)) 
                return None
            else:
                return result
        except requests.exceptions.SSLError:
            message = "Invalid or self-signed Kiwibes server certificate !"
            logging.error(message)
            raise KiwibesServerError(message,self.ERROR_HTTPS_CERTS_FAIL)    
        except requests.exceptions.ConnectionError:
            message = "failed to connect to Kiwibes server at: %s" % self.url
            logging.error(message)
            raise KiwibesServerError(message,self.ERROR_SERVER_NOT_FOUND)    
            
    def ping(self):
        """
        Verify if the server is up and running and the authentication
        token is valid.

        Returns:
            - "pong" if successfull, None otherwise
        """
        try:
            logging.info("Pinging the server hat: %s" % self.url)
            data = { "auth" : self.token }
            if self.ERROR_NO_ERROR == self.__post("/rest/ping",data):
                return "pong"
            else:
                return None
        except KiwibesServerError:
            return None 

    def datastore_write(self,key,value):
        """
        Write the key-value pair to the Kiwibes data store.

        Arguments:
            - key   : the name of the key
            - value : (string) the value of the key
        """
        logging.info("Writting to datastore: %s|%s" % (key,value))
        data = { "value" : value, "auth"  : self.token }
        return self.__post("/rest/data/write/%s" % key,data)

    def datastore_read(self,key):
        """
        Read the value associated with the given key,

        Arguments:
            - key : the name of the key

        Returns:
            - the value associated with the key, None in case of error 
        """
        logging.info("Reading from datastore: %s" % key)
        params = { "auth"  : self.token }
        response = self.__get("/rest/data/read/%s" % key,params)
        if response:
            return response.json()["value"]
        else:
            return None

    def datastore_update(self,key,value): 
        """
        Update the value of an existing key.
        If the key does not exist, the key-value pair is added.

        Arguments:
            - key   : the name of the key
            - value : the value of 
        
        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        if self.ERROR_DATA_KEY_TAKEN == self.datastore_write(key,value):
            # already used, delete it first
            self.datastore_clear(key)
            return self.datastore_write(key,value)
        else:
            return self.ERROR_NO_ERROR
        
    def datastore_clear(self,key):
        """
        Clear the key-value pair from the Kiwibes server data store

        Arguments:
            - key : the name of the key
        """
        logging.info("Removing from datastore: %s" % key)
        data = { "auth"  : self.token }
        return self.__post("/rest/data/clear/%s" % key,data)

    def datastore_clear_all(self):
        """
        Clear all of the key-value pairs from the Kiwibes server data store
        """
        logging.info("Removing all key-value pairs from datastore")
        data = { "auth"  : self.token }
        return self.__post("/rest/data/clear_all",data)

    def datastore_get_keys(self):
        """
        Return all keys stored in the data store.
        """
        logging.info("Retrieving keys from datastore")
        params = { "auth"  : self.token }
        return self.__get("/rest/data/keys",params).json()

    def get_all_jobs(self):
        """
        Return a list of all job names.
        """
        logging.info("Retrieving list of all jobs")
        params = { "auth"  : self.token }
        response = self.__get("/rest/jobs/list",params)
        if response:
            return response.json()
        else:
            return None

    def get_scheduled_jobs(self):
        """
        Return a list with the names of all scheduled jobs.
        """
        params = { "auth"  : self.token }
        response = self.__get("/rest/jobs/scheduled",params)
        if response:
            return response.json()
        else:
            return None
   
    def start_job(self,name):
        """
        Start the execution of the job. If the job is already running,
        it queues the execution.

        Arguments:
            - name : the name of the job

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Starting job: %s" % name)        
        data = { "auth"  : self.token }
        return self.__post("/rest/job/start/%s" % name,data)

    def stop_job(self,name):
        """
        Stop the execution of the job. If there are pending executions,
        the next one will be started.

        Arguments:
            - name : the name of the job

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Stopping job: %s" % name)        
        data = { "auth"  : self.token }
        return self.__post("/rest/job/stop/%s" % name,data)

    def delete_job(self,name):
        """
        Delete the job from the database. This will fail if the job is
        currently in execution.

        Arguments:
            - name : the name of the job

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Deleting job: %s" % name)        
        data = { "auth"  : self.token }
        return self.__post("/rest/job/delete/%s" % name,data)

    def clear_job_queue(self,name):
        """
        Clear all pending executions for the job.

        Arguments:
            - name : the name of the job

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Clearing pending executions for job: %s" % name)        
        data = { "auth"  : self.token }
        return self.__post("/rest/job/clear_pending/%s" % name,data)


    def get_job_details(self,name):
        """
        Return a dictionary with the job complete information, as
        it is stored in the database.

        Arguments:
            - name : the name of the job

        Returns:
            - dictionary with the job detailed information, None in case of error
        """
        logging.info("Retrieving complete information for job: %s" % name)        
        params = { "auth"  : self.token }
        response = self.__get("/rest/job/details/%s" % name,params)
        if response:
            return response.json()
        else:
            return None
       
    def create_job(self,name,schedule="",program=[],max_runtime=0):
        """
        Create a job with the given properties.

        Arguments:
            - name        : the name of the job
            - schedule    : string with a Cron like expression
            - program     : array with the program and its arguments
            - max_runtime : maximum runtime for the program, in seconds

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Creating job: %s" % name)        
        data = { "auth"        : self.token,
                 "program"     : program,
                 "max-runtime" : max_runtime,
                 "schedule"    : schedule,
                }
        return self.__post("/rest/job/create/%s" % name,data)

    def edit_job_schedule(self,name,schedule):
        """
        Update the job schedule

        Arguments:
            - name     : the name of the job
            - schedule : string with a Cron like expression

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Updating job schedule: %s" % name)        

        details = self.get_job_details(name)
        if details:
            data = { "auth"        : self.token,
                     "program"     : details["program"],
                     "schedule"    : schedule,
                     "max-runtime" : details["max-runtime"]
                    }
            return self.__post("/rest/job/edit/%s" % name,data)
        else:
            return self.ERROR_JOB_NAME_UNKNOWN

    def edit_job_program(self,name,program):
        """
        Update the job program that is executed.

        Arguments:
            - name    : the name of the job
            - program : vector with the program and its arguments

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Updating job program: %s" % name)        

        details = self.get_job_details(name)
        if details:
            data = { "auth"        : self.token,
                     "program"     : program,
                     "schedule"    : details["schedule"],
                     "max-runtime" : details["max-runtime"]
                    }
            return self.__post("/rest/job/edit/%s" % name,data)
        else:
            return self.ERROR_JOB_NAME_UNKNOWN

    def edit_job_max_runtime(self,name,max_runtime):
        """
        Update the job maximum runtime.

        Arguments:
            - name        : the name of the job
            - max_runtime : maximmum allowed runtime for the job

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Updating job maximmum runtime: %s" % name)        

        details = self.get_job_details(name)
        if details:
            data = { "auth"        : self.token,
                     "program"     : details["program"],
                     "schedule"    : details["schedule"],
                     "max-runtime" : max_runtime,
                    }
            return self.__post("/rest/job/edit/%s" % name,data) 
        else:
            return self.ERROR_JOB_NAME_UNKNOWN            