#pragma once
#include <vector>
#include <pspkerneltypes.h>
#include <map>
#include <psptypes.h>
#include <psprtc.h>

/**
* The mode of execution for a specific job.
*/
#define MELIB_EXEC_DEFAULT	0x0 /** Executes on the ME, unless Dynamic Rebalancing is turned on */
#define MELIB_EXEC_CPU		0x1 /**  Executes specifically on the main CPU, regardless of job mode */
#define MELIB_EXEC_ME		0x2 /** Executes specifically on the Media Engine, regardless of job mode */

/**
* Importance of the job.
*/
#define MELIB_PRIORITY_NONE 0x0 /** No priority. Equivalent to medium when priority queue is enabled */
#define MELIB_PRIORITY_LOW	0x1 /** Low priority */
#define MELIB_PRIORITY_MED	0x2 /** Medium priority */
#define MELIB_PRIORITY_HIGH	0x4 /** High priority */

namespace MElib {

	/**
	* This structure is used to determine job execution.
	* Given the properties in this structure, the manager will execute correctly
	*/
	struct JobInfo {
		unsigned char id; /** This ID is used for tracking performance and dynamic rebalancing. Use one id per different job */
		unsigned char execMode; /** Uses execution mode to specify where the code will run and/or if said code can be dynamically rebalanced */
		unsigned char priority; /** States priority of the job */
		bool ignorePriority; /** Forces the code to run */
	};

	/**
	* This structure is used to determine the execution structure of the jobs.
	* Given the properties in this structure, the manager will execute accordingly.
	*/
	struct ManagerInfo {
		bool asynchronous; /** This setting determines whether or not the JobManager waits for the job to finish or if the main CPU can continue execution */
		bool priorityQueue; /** This setting determines whether or not the JobManager will prioritize higher priority or lower priority tasks */
		bool dynamicRebalancing; /** This setting determines whether or not the JobManager will try to balance system load */
	};


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
	struct Job {
		JobInfo jobInfo;
		JobFunction function;
		JobData data;
	};

	/** 
	* JobManager class. This class only can have one instance for the ME.
	*/
	class JobManager {
	public:
		JobManager(); /** Initializes data */

		void Init(ManagerInfo info); /** Initialize the job manager with the ManagerInfo */
		void Cleanup(); /** Cleans up and ends execution */

		void AddJob(Job* job); /** Adds a job to the queue */
		void ClearJob(); /** Clears and deletes all jobs */

	private:
		std::vector<Job*> jobQueue;	
		static int thread_update(SceSize args, void* argp);
		void Update();

		bool m_async;
		bool m_priority;
		bool m_dynamic;
		bool isExecuting;

		std::map<int, float> dynaMap;
		float cpu_time;
		float me_time;

		u32 tickResolution;
		u64 lastTick;
	};

	/** 
	* This is a global (defined) job manager. It is the only instance to be used!
	*/
	extern JobManager g_JobManager;

}