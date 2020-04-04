#include <melib.h>
#include <driver/me.h>
#include <malloc.h>
#include <pspkernel.h>

volatile me_struct* mei;

inline void* malloc_64(int size)
{
	int mod_64{ size & 0x3f };
	if (mod_64 != 0) size += 64 - mod_64;
	return((void*)memalign(64, size));
}

namespace MElib {
	JobManager g_JobManager;

	JobManager::JobManager() {
		m_priority = false;
		m_dynamic = false;
		isExecuting = false;
		lastTick = 0;
		forceCPU = false;
	}

	void JobManager::Init(ManagerInfo info) {
		int ret = pspSdkLoadStartModule("./mediaengine.prx", PSP_MEMORY_PARTITION_KERNEL);

		if (ret < 0) {
			forceCPU = true;
		}

		mei = (volatile struct me_struct*)malloc_64(sizeof(struct me_struct));
		mei = (volatile struct me_struct*)(reinterpret_cast<void*>(reinterpret_cast<u32>((mei)) | 0x40000000));
		sceKernelDcacheWritebackInvalidateAll();

		if (!forceCPU) {
			ret = InitME(mei);

			if (ret != 0) {
				forceCPU = true;
			}
		}

		thread_id = sceKernelCreateThread("ManagementThread", thread_update, 0x18, 0x10000, THREAD_ATTR_VFPU | THREAD_ATTR_USER, NULL);

		sceRtcGetCurrentTick(&lastTick);
		tickResolution = sceRtcGetTickResolution();

		m_priority = info.priorityQueue;
		m_dynamic = info.dynamicRebalancing;
	}

	void JobManager::Cleanup() {
		sceKernelTerminateDeleteThread(thread_id);
		ClearJob();
	}

	void JobManager::ClearJob() {
		int size = jobQueue.size();
		for (int i = 0; i < size; i++) {
			delete jobQueue.front();
			jobQueue.pop();
		}
	}

	void JobManager::Update() {

		float jbCPULoad = 0.0f;
		float jbMELoad = 0.0f;

		int size = jobQueue.size();
		while (jobQueue.size() > 0) {
			auto job = jobQueue.front();
			//Execute job
			isExecuting = true;

			int endMode = job->jobInfo.execMode;
			if (m_dynamic && !forceCPU) {
				sceRtcGetCurrentTick(&lastTick);

				if (endMode == MELIB_EXEC_DEFAULT) {
					if (jbCPULoad >= jbMELoad) {
						endMode = MELIB_EXEC_ME;
					}
					else {
						endMode = MELIB_EXEC_CPU;
					}
				}
			}


			if (job->jobInfo.execMode == MELIB_EXEC_CPU || forceCPU) {
				job->function(job->data);
				sceKernelDelayThread(400); //Give a "breather" time
			}
			else {
				BeginME(mei, (int)job->function, (int)job->data, -1, NULL, -1, NULL);
				while(!CheckME(mei))
					sceKernelDelayThread(100); //Poll for it
			}

			//Done
			isExecuting = false;

			if (m_dynamic && !forceCPU) {
				u64 temp = lastTick;
				sceRtcGetCurrentTick(&lastTick);
				float dt = (double)(lastTick - temp) / ((double)tickResolution);
				if (endMode == MELIB_EXEC_CPU) {
					jbCPULoad += dt;
				}
				else {
					jbMELoad += dt;
				}
			}

			delete job;
			jobQueue.pop();
		}


		//Delay till the next frame
		sceKernelDelayThread(16 * 1000);
	}

	int JobManager::thread_update(SceSize args, void* argp) {
		while (true) {
			g_JobManager.Update();
		}
	}

	void JobManager::AddJob(Job* job) {
		jobQueue.push(job);
	}
}