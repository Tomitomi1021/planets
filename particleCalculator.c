#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <CL/cl.h>
#include "openclWrapper.h"
#include "particleCalculator.h"
#include "config.h"

clw_memory stateA_m,stateB_m,option_m;
clw_program program;
clw_kernel kernel;

#define ERRORCHECK(ret)  if(ret == CLW_FAIL){return PC_FAIL;}

int pc_init(){
	int ret;
	printf("opencl wrapper init ...");fflush(stdout);
	ret=clw_init();
	ERRORCHECK(ret);
	printf("DONE\n");

	printf("allocating memory for GPU ...");fflush(stdout);
	stateA_m=clw_makeMemoryObject(
		CL_MEM_READ_WRITE,
		sizeof(particle)*PARTICLE_COUNT
		);
	ERRORCHECK(stateA_m);

	stateB_m=clw_makeMemoryObject(
		CL_MEM_READ_WRITE,
		sizeof(particle)*PARTICLE_COUNT
		);
	ERRORCHECK(stateB_m);
	printf("DONE\n");

	printf("Loading program...");fflush(stdout);
	program=clw_loadProgram(PARTICLE_PROGRAM);
	ERRORCHECK(program);
	printf("DONE\n");
	
	printf("Building program...\n");
	ret=clw_buildProgram(program);
	fprintf(stderr,"%s\n",clw_getLatestBuildErrorLog());
	ERRORCHECK(ret);
	printf("DONE\n");

	printf("Creating kernel...");fflush(stdout);
	kernel = clw_createKernel(program,PC_ENTRY);
	ERRORCHECK(kernel);
	printf("DONE\n");

	return PC_SUCCESS;
}

int pc_initialize(particle initialState[]){
	int ret;
	ret=clw_memory_write(stateA_m,initialState,0,sizeof(particle)*PARTICLE_COUNT);
	ERRORCHECK(ret);
	ret=clw_memory_write(stateB_m,initialState,0,sizeof(particle)*PARTICLE_COUNT);
	ERRORCHECK(ret);
	return PC_SUCCESS;
}

int pc_setoption(option* opt){
	int ret;
	ret = clw_memory_write(option_m,opt,0,sizeof(option));
	ERRORCHECK(ret);
	return PC_SUCCESS;
}

int pc_calculate(particle result[]){
	int ret;
	ret=clw_setKernelArg(kernel,0,stateA_m);
	ERRORCHECK(ret);

	ret=clw_setKernelArg(kernel,1,stateB_m);
	ERRORCHECK(ret);

	ret=clw_setKernelArg(kernel,2,option_m);
	ERRORCHECK(ret);

	ret=clw_kernel_run(kernel,PARTICLE_COUNT,GROUP_COUNT);
	ERRORCHECK(ret);

	ret=clw_memory_read(result,stateB_m,0,sizeof(particle)*PARTICLE_COUNT);
	ERRORCHECK(ret);

	{
		clw_memory temp;
		temp = stateA_m;
		stateA_m = stateB_m;
		stateB_m = temp;
	}

	return PC_SUCCESS;
}

void pc_quit(){
	clw_quit();
}
