#pragma once
#include <time.h>
#include <stdbool.h>
#include <pspkerneltypes.h>
#include <psptypes.h>
#include <psprtc.h>

/**
* The mode of execution for a specific job.
*/
#define MELIB_EXEC_DEFAULT	0x0 /** Executes on the ME, unless Dynamic Rebalancing is turned on */
#define MELIB_EXEC_CPU		0x1 /**  Executes specifically on the main CPU, regardless of job mode. Always runs synchronously (gives a small delay between jobs).*/
#define MELIB_EXEC_ME		0x2 /** Executes specifically on the Media Engine, regardless of job mode. Always runs asynchronously.*/

/**
* Importance of the job.
*/
#define MELIB_PRIORITY_NONE 0x0 /** No priority. Equivalent to medium when priority queue is enabled */
#define MELIB_PRIORITY_LOW	0x1 /** Low priority */
#define MELIB_PRIORITY_MED	0x2 /** Medium priority */
#define MELIB_PRIORITY_HIGH	0x4 /** High priority */


/**
* This structure is used to determine job execution.
* Given the properties in this structure, the manager will execute correctly
*/
typedef struct  {
	unsigned char id; /** This ID is used for tracking performance and dynamic rebalancing. Use one id per different job */
	unsigned char execMode; /** Uses execution mode to specify where the code will run and/or if said code can be dynamically rebalanced */
	unsigned char priority; /** States priority of the job */
	bool ignorePriority; /** Forces the code to run */
} JobInfo;

/**
* This structure is used to determine the execution structure of the jobs.
* Given the properties in this structure, the manager will execute accordingly.
*/
typedef struct {
	bool priorityQueue; /** This setting determines whether or not the JobManager will prioritize higher priority or lower priority tasks */
	bool dynamicRebalancing; /** This setting determines whether or not the JobManager will try to balance system load */
} ManagerInfo;


struct Job;

/**
* Job Data is an integer pointer to an address with the data.
*/
typedef int JobData;
	
/**
* This typedef defines a JobFunction as an integer function with given data.
*/
typedef int (*JobFunction)(JobData ptr);

/**
* This structure is used to give job information alongside the job itself and the data needed.
*/
typedef struct {
	JobInfo jobInfo;
	JobFunction function;
	JobData data;
}Job;

/** 
* JobManager class. This class only can have one instance for the ME.
*/

void J_Init(ManagerInfo info); /** Initialize the job manager with the ManagerInfo */
void J_Cleanup(); /** Cleans up and ends execution */

void J_AddJob(Job* job); /** Adds a job to the queue */
void J_ClearJob(); /** Clears and deletes all jobs */

void J_DispatchJobs();