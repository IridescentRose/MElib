#include <melib.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <malloc.h>
#include <stdio.h>

PSP_MODULE_INFO("MELIB TEST", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

int addFunc(JobData data) {
	int* i = (int*)data;

	(*i)++;

	return *i; //Result doesn't matter
}

int main(int argc, char* argv[])
{
	pspDebugScreenInit();

	J_Init(false);

	int myInt = 77;

	while (1)
	{
		pspDebugScreenSetXY(0, 0);
		pspDebugScreenPrintf("MY COUNTER: %x\n", myInt);

		struct Job* j = (struct Job*)malloc(sizeof(struct Job));
		j->jobInfo.id = 1;
		j->jobInfo.execMode = MELIB_EXEC_ME;

		j->function = &addFunc;
		j->data = (int)&myInt;

		J_AddJob(j);

		J_DispatchJobs(0.0f); //No dynamic rebalancing so this doesn't matter.
		sceDisplayWaitVblankStart();
	}

	J_Cleanup();

	return 0;
}