#include "pc_common.h"

#define PC_SUCCESS	(0)
#define PC_FAIL		(-1)

int pc_init();
int pc_initialize(particle initialState[]);
int pc_calculate(particle result[]);
void pc_quit();
int pc_setoption(option* opt);
