#define CLW_SUCCESS	(0)
#define CLW_FAIL	(-1)

typedef int clw_memory;
typedef int clw_program;
typedef int clw_kernel;

char* clw_getError();

int clw_init();
void clw_quit();

clw_memory clw_makeMemoryObject(cl_mem_flags flag,size_t size);
int clw_memory_write(clw_memory dst,void* src,size_t offset,size_t size);
int clw_memory_read(void* dst,clw_memory src,size_t offset,size_t size);
clw_program clw_loadProgram(char* filename);
char* clw_getLatestBuildErrorLog();
int clw_buildProgram(clw_program prg);
clw_kernel clw_createKernel(clw_program prg,char* functionName);
int clw_setKernelArg(clw_kernel knl,int no,clw_memory mem);
int clw_kernel_run(clw_kernel knl,size_t global_item_size,size_t local_item_size);
