#include <string.h>
#include <stdarg.h>
#include <CL/cl.h>
#include "openclWrapper.h"
#include "particleCalculator.h"

#define PARTICLE_PROGRAM	"pc_kernel.c"
#define PC_ENTRY		"kmain"

clw_memory stateA_m,stateB_m;
clw_program program;
clw_kernel kernel;

#define ERRORCHECK(ret)  if(ret == CLW_FAIL){return PC_FAIL;}

int pc_init(){
	int ret;
	ret=clw_init();
	ERRORCHECK(ret);

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

	program=clw_loadProgram(PARTICLE_PROGRAM);
	ERRORCHECK(program);
	
	ret=clw_buildProgram(program);
	ERRORCHECK(ret);

	kernel = clw_createKernel(prg,PC_ENTRY);
	ERRORCHECK(kernel);

	return PC_SUCCESS;
}

int pc_initialize(particle initialState[]){
	int ret;
	ret=clw_memory_write(stateA_m,initialState);
	ERRORCHECK(ret);
	ret=clw_memory_write(stateB_m,initialState);
	ERRORCHECK(ret);
}

int pc_calculate(particle result[]){
	ret=clw_setKernelArg(kernel,0,stateA_m);
	ERRORCHECK(ret);

	ret=clw_setKernelArg(kernel,1,stateB_m);
	ERRORCHECK(ret);

	ret=clw_kernel_run(kernel,PARTICLE_COUNT,1);
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
