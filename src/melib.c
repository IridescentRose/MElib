#include <melib.h>
#include <driver/me.h>
#include <malloc.h>
#include <stdio.h>
#include <pspkernel.h>

#define MAX_QUEUE_SIZE 512
struct Job* queue[MAX_QUEUE_SIZE];
int size = 0;

bool listDone = false;
float cpuTime = 0.0f;

void J_AddJob(struct Job* job) {
	if (size + 1 > MAX_QUEUE_SIZE) {
		J_DispatchJobs(cpuTime);

		while (!listDone) {}
	}

	queue[size++] = job;
}

void J_ClearJob() {
	for (int i = 0; i < size; i++) {
		free(queue[i]);
		queue[i] = NULL;
	}
	size = 0;
}

struct me_struct* mei;

void* malloc_64(int size)
{
	int mod_64 = size & 0x3f;
	if (mod_64 != 0) size += 64 - mod_64;
	return((void*)memalign(64, size));
}


bool forceCPU;

SceUID thread_id;

bool m_priority;
bool m_dynamic;
bool isExecuting;

u32 tickResolution;
u64 lastTick;

int JI_ThreadUpdate(SceSize args, void* argp);


void J_Init(bool dynamicRebalance) {
	m_priority = false;
	m_dynamic = false;
	isExecuting = false;
	lastTick = 0;
	forceCPU = false;

	int ret = pspSdkLoadStartModule("./mediaengine.prx", PSP_MEMORY_PARTITION_KERNEL);

	if (ret < 0) {
		forceCPU = true;
	}
	else {
		printf("ME Module Loaded\n");
	}

	mei = (volatile struct me_struct*)malloc_64(sizeof(struct me_struct));
	mei = (volatile struct me_struct*)( ((u32)(mei)) | 0x40000000);
	sceKernelDcacheWritebackInvalidateAll();

	if (!forceCPU) {
		ret = InitME(mei);

		if (ret != 0) {
			forceCPU = true;
		}
	}

	thread_id = sceKernelCreateThread("ManagementThread", JI_ThreadUpdate, 0x18, 0x10000, THREAD_ATTR_VFPU | THREAD_ATTR_USER, NULL);

	sceRtcGetCurrentTick(&lastTick);
	tickResolution = sceRtcGetTickResolution();
	m_dynamic = dynamicRebalance;
}

void J_Cleanup() {
	KillME(mei);
	sceKernelTerminateDeleteThread(thread_id);
	J_ClearJob();
}

float jbCPULoadLast = 0.0f;
float jbMELoadLast = 0.0f;

void J_Update(float cpuTime) {

	float jbCPULoad = 0.0f;
	float jbMELoad = 0.0f;

	listDone = false;

	printf("Dispatch Size: %d\n", size);
	for(int i = 0; i < size; i++){
		struct Job* job = queue[i];
		//Execute job
		isExecuting = true;


		int endMode = job->jobInfo.execMode;
		sceRtcGetCurrentTick(&lastTick);

		if (m_dynamic && !forceCPU) {

			if (endMode == MELIB_EXEC_DEFAULT) {
				if (jbCPULoad + cpuTime >= jbMELoad) {
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

			sceKernelDcacheWritebackInvalidateAll();
		}

		//Done
		isExecuting = false;

		u64 temp = lastTick;
		sceRtcGetCurrentTick(&lastTick);
		float dt = (double)(lastTick - temp) / ((double)tickResolution);
		if (endMode == MELIB_EXEC_CPU) {
			jbCPULoad += dt;
		}
		else {
			jbMELoad += dt;
		}

		free(job);
		queue[i] = NULL;
	}
	size = 0;

	jbCPULoadLast = jbCPULoad;
	jbMELoadLast = jbMELoad;

	listDone = true;
}

void J_DispatchJobs(float cpu) {
	cpuTime = cpu;
	sceKernelStartThread(thread_id, 0, NULL);
}

int JI_ThreadUpdate(SceSize args, void* argp) {
	J_Update(cpuTime);
}


float J_GetMETime() {
	return jbMELoadLast;
}
float J_GetCPUTime() {
	return jbCPULoadLast;
}