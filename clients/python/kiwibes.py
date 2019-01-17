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

class KiwibesServer():
    """
    Provides an abstraction to interact with the Kiwibes server
    """

    # definition of the error codes that might be returned
    # by the REST interface
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

        Arguments:
            - route : the server HTTP route 
            - data  : the POST call data 

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
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
            logging.error("invalid or self-signed Kiwibes server certificate !")    
            return self.ERROR_HTTPS_CERTS_FAIL
        except requests.exceptions.ConnectionError:
            logging.error("failed to connect to Kiwibes server")    
            return self.ERROR_SERVER_NOT_FOUND

    def __get(self,route,data): 
        """
        GET method for the given HTTP route.

        Arguments:
            - route : the server HTTP route 
            - data  : the POST call data 

        Returns:
            - the response, None in case of error
        """
        try:
            path = self.url + route
            return requests.get(path,params=data,verify=self.verify_cert)
        except requests.exceptions.SSLError:
            logging.error("invalid or self-signed Kiwibes server certificate !")    
            return None

    def datastore_write(self,key,value):
        """
        Write the key-value pair to the Kiwibes data store.

        Arguments:
            - key   : the name of the key
            - value : (string) the value of the key

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
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
        if response and 200 == response.status_code:
            return response.json()["value"]
        else:
            return None

    def datastore_clear(self,key):
        """
        Clear the key-value pair from the Kiwibes server data store

        Arguments:
            - key : the name of the key

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
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

    def get_all_jobs(self):
        """
        Return a list of all job names.
        """
        logging.info("Retrieving list of all jobs")
        params = { "auth"  : self.token }
        response = self.__get("/rest/jobs/list",params)
        jobs = []
        if response:
            if 200 == response.status_code:
                jobs = response.json()
            else:
                logging.error("GET - %s/rest/jobs/list: (%d) %s" % (self.url,response.json()["error"],response.json()["message"])) 
        else:
            logging.error("GET - %s/rest/jobs/list: failed")
        return jobs 

    def get_scheduled_jobs(self):
        """
        Return a list with the names of all scheduled jobs.
        """
        params = { "auth"  : self.token }
        response = self.__get("/rest/jobs/scheduled",params)
        jobs = []
        if response:
            if 200 == response.status_code:
                jobs = response.json()
            else:
                logging.error("GET - %s/rest/jobs/scheduled: (%d) %s" % (self.url,response.json()["error"],response.json()["message"])) 
        else:
            logging.error("GET - %s/rest/jobs/scheduled: failed")
        return jobs 

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
        response = self.__get("/rest/jobs/scheduled",params)
        if response and 200 == response.status_code:
            return response.json()
        else:
            logging.error("GET - %s/rest/job/details/%s: (%d) %s" % (self.url,name,response.json()["error"],response.json()["message"])) 
            return None

    def create_job(self,name,schedule,program):
        """
        Create a job with the given properties.

        Arguments:
            - name     : the name of the job
            - schedule : string with a Cron like expression
            - program  : array with the program and its arguments

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Creating job: %s" % name)        
        data = { "auth"     : self.token,
                 "program"  : program,
                 "schedule" : schedule,
                }
        return self.__post("/rest/job/create/%s" % name,data)

    def edit_job(self,name,schedule,program):
        """
        Update the job properties. Unlike the create_job() method,
        the job must already exist.

        Arguments:
            - name     : the name of the job
            - schedule : string with a Cron like expression
            - program  : array with the program and its arguments

        Returns:
            - ERROR_NO_ERROR if successfull, error code otherwise
        """
        logging.info("Updating job: %s" % name)        
        data = { "auth"     : self.token,
                 "program"  : program,
                 "schedule" : schedule,
                }
        return self.__post("/rest/job/edit/%s" % name,data)
