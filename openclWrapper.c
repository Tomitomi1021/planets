#include<CL/cl.h>
#include<stdarg.h>
#include<stdio.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#include "openclWrapper.h"

#define MEMORY_OBJECT_MAX	(256)
#define PROGRAM_MAX		(256)
#define KERNEL_MAX		(256)
#define ERROR_BUFFER_SIZE	(2048)

cl_platform_id		platform=NULL;
cl_device_id		device=NULL;
cl_context		context=NULL;
cl_command_queue	cqueue=NULL;

static cl_mem		memoryObject[MEMORY_OBJECT_MAX];
static cl_program	programs[PROGRAM_MAX];
static cl_kernel	kernels[KERNEL_MAX];


static char ErrorBuffer[ERROR_BUFFER_SIZE];

static void setError(char* format,...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(ErrorBuffer,ERROR_BUFFER_SIZE,format,ap);
	va_end(ap);
}

char* clw_getError(){
	return ErrorBuffer;
}


static int platform_init(){
	cl_uint count=0;
	cl_platform_id ID_List[1024];
	int ret;

	ret = clGetPlatformIDs(
		1024,
		ID_List,
		&count
		);

	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	if(count==0){
		setError("platform not found.");
		return CLW_FAIL;
	}else{
		platform=ID_List[0];
		return CLW_SUCCESS;
	}
}

static int device_init(){
	assert(platform!=NULL);
	cl_uint count=0;
	cl_device_id ID_List[1024];
	int ret;
	
	ret = clGetDeviceIDs(
		platform,
		CL_DEVICE_TYPE_GPU,
		1024,
		ID_List,
		&count
		);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	if(count==0){
		setError("device not found.");
		return CLW_FAIL;
	}else{
		device=ID_List[0];
		return CLW_SUCCESS;
	}
}

static int context_init(){
	int ret;
	assert(device!=NULL);
	
	context = clCreateContext(NULL,1,&device,NULL,NULL,&ret);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}
	return CLW_SUCCESS;
}

static int cqueue_init(){
	int ret;
	assert(context!=NULL);
	assert(device!=NULL);

	cqueue = clCreateCommandQueueWithProperties(context,device,NULL,&ret);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}
	return CLW_SUCCESS;
}

int clw_init(){
	int ret;

	for(int i=0;i<MEMORY_OBJECT_MAX;i++){
		memoryObject[i]=NULL;
	}

	for(int i=0;i<PROGRAM_MAX;i++){
		programs[i]=NULL;
	}

	for(int i=0;i<KERNEL_MAX;i++){
		kernels[i]=NULL;
	}

	ret=platform_init();
	if(ret==CLW_FAIL)return CLW_FAIL;

	ret=device_init();
	if(ret==CLW_FAIL)return CLW_FAIL;

	ret=context_init();
	if(ret==CLW_FAIL)return CLW_FAIL;

	ret=cqueue_init();
	if(ret==CLW_FAIL)return CLW_FAIL;

	return CLW_SUCCESS;
}


clw_memory clw_makeMemoryObject(cl_mem_flags flag,size_t size){
	int ret;
	assert(context!=NULL);
	assert(size>0);
	
	clw_memory targetID=-1;

	for(int i=0;i<MEMORY_OBJECT_MAX;i++){
		if(memoryObject[i]==NULL){
			targetID=i;
			break;
		}
	}
	if(targetID==-1){
		setError("Memory object is too many.");
		return CLW_FAIL;
	}

	memoryObject[targetID] = clCreateBuffer(
					context,
					flag,
					size,
					NULL,
					&ret
					);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	return targetID;
}

int clw_memory_write(clw_memory dst,void* src,size_t offset,size_t size){
	int ret;
	assert(cqueue!=NULL);
	assert(src!=NULL);
	assert(0 <= dst && dst <= MEMORY_OBJECT_MAX);
	assert(size>0);

	if(memoryObject[dst]==NULL){
		setError(
			"Memory object is not exist. [object id=%d]"
			,dst
			);
		return CLW_FAIL;
	}
	if(src==NULL){
		setError("Source is NULL");
		return CLW_FAIL;
	}

	ret=clEnqueueWriteBuffer(
		cqueue,
		memoryObject[dst],
		CL_TRUE,
		offset,
		size,
		src,
		0,
		NULL,
		NULL
		);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	return CLW_SUCCESS;
}

int clw_memory_read(void* dst,clw_memory src,size_t offset,size_t size){
	int ret;
	assert(cqueue!=NULL);
	assert(dst!=NULL);
	assert(0 <= src && src <= MEMORY_OBJECT_MAX);
	assert(size>0);

	if(memoryObject[src]==NULL){
		setError(
			"Memory object is not exist. [object id=%d]"
			,src
			);
		return CLW_FAIL;
	}
	if(dst==NULL){
		setError("dst is NULL");
		return CLW_FAIL;
	}

	ret=clEnqueueReadBuffer(
		cqueue,
		memoryObject[src],
		CL_TRUE,
		offset,
		size,
		dst,
		0,
		NULL,
		NULL
		);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	return CLW_SUCCESS;
}

#define MAX_SOURCE_SIZE	(0x1000000)
static int loadSourceCode(const char* filename,char** code,size_t* size){
	FILE* fp;
	char* source_str;
	size_t source_size;

	assert(filename!=NULL);
	assert(code!=NULL);
	assert(size!=NULL);


	fp=fopen(filename,"r");
	if(fp==NULL){
		setError("fopen failed : %s",strerror(errno));
		return CLW_FAIL;
	}

	source_str=malloc(MAX_SOURCE_SIZE);
	if(source_str==NULL){
		setError("malloc failed : %s",strerror(errno));
		fclose(fp);
		return CLW_FAIL;
	}

	source_size = fread(source_str,1,MAX_SOURCE_SIZE,fp);
	fclose(fp);

	*code = source_str;
	*size = source_size;
	
	return CLW_SUCCESS;
}


clw_program clw_loadProgram(char* filename){
	int ret;
	clw_program targetID=-1;
	assert(context!=NULL);
	assert(filename!=NULL);

	for(int i=0;i<PROGRAM_MAX;i++){
		if(programs[i]==NULL){
			targetID=i;
			break;
		}
	}
	if(targetID==-1){
		setError("Program is too many.");
		return CLW_FAIL;
	}

	{
		char* source_str;
		size_t source_size;

		ret = loadSourceCode(filename,&source_str,&source_size);
		if(ret==CLW_FAIL)return CLW_FAIL;

		programs[targetID] = clCreateProgramWithSource(
					context,
					1,
					(const char**)&source_str,
					(const size_t*)&source_size,
					&ret
					);
		free(source_str);
	}


	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	return targetID;
}


#define BUILDLOG_MAX	(2048)
static char BuildErrorLog[BUILDLOG_MAX];

char* clw_getLatestBuildErrorLog(){
	return BuildErrorLog;
}

int clw_buildProgram(clw_program prg){
	int ret;
	assert(device!=NULL);
	assert(0 <= prg && prg < PROGRAM_MAX);

	if(programs[prg]==NULL){
		setError("program is NULL [id=%d]",prg);
		return CLW_FAIL;
	}

	ret = clBuildProgram(programs[prg],1,&device,NULL,NULL,NULL);
	if(ret != CL_SUCCESS){
		size_t size;
		clGetProgramBuildInfo(
			programs[prg],
			device,
			CL_PROGRAM_BUILD_LOG,
			BUILDLOG_MAX,
			BuildErrorLog,
			&size
			);
		BuildErrorLog[size]=0;
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}
	return CLW_SUCCESS;
}

clw_kernel clw_createKernel(clw_program prg,char* functionName){
	int ret;
	clw_kernel targetID=-1;

	assert(0 <= prg && prg < PROGRAM_MAX);
	assert(functionName!=NULL);

	if(programs[prg]==NULL){
		setError("program is NULL [id=%d]",prg);
		return CLW_FAIL;
	}

	for(int i=0;i<KERNEL_MAX;i++){
		if(kernels[i]==NULL){
			targetID=i;
			break;
		}
	}
	if(targetID==-1){
		setError("kernel is too many.");
		return CLW_FAIL;
	}
	
	kernels[targetID]=clCreateKernel(programs[prg],functionName,&ret);
	if(ret!=CL_SUCCESS){
		setError("opencl:code %d",ret);
		return CLW_FAIL;
	}

	return targetID;
}

int clw_setKernelArg(clw_kernel knl,int no,clw_memory mem){
	int ret;

	assert(0 <= knl && knl < KERNEL_MAX);
	assert(0 <= mem && mem < MEMORY_OBJECT_MAX);

	if(kernels[knl]==NULL){
		setError("kernel is NULL [id=%d]",knl);
		return CLW_FAIL;
	}
	
	if(memoryObject[mem]==NULL){
		setError("memory object is NULL [id=%d]",mem);
		return CLW_FAIL;
	}

	ret = clSetKernelArg(
		kernels[knl],
		no,
		sizeof(cl_mem),
		(void*)&memoryObject[mem]
		);
	if(ret!=CL_SUCCESS){
		setError("oepncl:code %d",ret);
		return CLW_FAIL;
	}

	return CLW_SUCCESS;
}



int clw_kernel_run(clw_kernel knl,size_t global_item_size,size_t local_item_size){
	int ret;

	assert(0 <= knl && knl < KERNEL_MAX);
	assert(global_item_size>0);
	assert(local_item_size>0);

	if(kernels[knl]==NULL){
		setError("kernel is NULL [id=%d]",knl);
		return CLW_FAIL;
	}
	
	ret = clEnqueueNDRangeKernel(
		cqueue,
		kernels[knl],
		1,
		NULL,
		&global_item_size,
		&local_item_size,
		0,
		NULL,
		NULL
		);
	if(ret!=CL_SUCCESS){
		setError("oepncl:code %d",ret);
		return CLW_FAIL;
	}
	return CLW_SUCCESS;
}

void clw_quit(){
	clFlush(cqueue);
	clFinish(cqueue);

	for(int i=0;i<KERNEL_MAX;i++){
		if(kernels[i]!=NULL){
			clReleaseKernel(kernels[i]);
			kernels[i]=NULL;
		}
	}

	for(int i=0;i<PROGRAM_MAX;i++){
		if(programs[i]!=NULL){
			clReleaseProgram(programs[i]);
			programs[i]=NULL;
		}
	}

	for(int i=0;i<MEMORY_OBJECT_MAX;i++){
		if(memoryObject[i]!=NULL){
			clReleaseMemObject(memoryObject[i]);
			memoryObject[i]=NULL;
		}
	}

	clReleaseCommandQueue(cqueue);
	clReleaseContext(context);
}
