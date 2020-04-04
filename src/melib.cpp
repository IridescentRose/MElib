#include <melib.h>
#include <driver/me.h>

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
		m_async = false;
		m_priority = false;
		m_dynamic = false;
		isExecuting = false;
		cpu_time = 0;
		me_time = 0;
		dynaMap.clear();
		lastTick = 0;
		jobQueue.clear();
		forceCPU = false;
	}

	void JobManager::Init(ManagerInfo info) {
		int ret = pspSdkLoadStartModule(path, PSP_MEMORY_PARTITION_KERNEL);

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
	}

	void JobManager::Cleanup() {
		sceKernelTerminateDeleteThread(thread_id);
		ClearJob();
	}

	void JobManager::ClearJob() {
		for (auto job : jobQueue) {
			delete job;
		}
		jobQueue.clear();
	}

	void JobManager::Update() {



		//Delay till the next frame
		sceKernelDelayThread(16 * 1000);
	}

	int JobManager::thread_update(SceSize args, void* argp) {
		while (true) {
			g_JobManager->Update();
		}
	}

	void JobManager::AddJob(Job* job) {
		jobQueue.push_back(job);
	}
}