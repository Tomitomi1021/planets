#include "pc_common.h"

#define WIDTH	(990)
#define HEIGHT	(520)

double potential(__global const particle* stateA,double x,double y){
	double U=0;
//	U=sqrt(pow((WIDTH/2-x),2)+pow((HEIGHT/2-y),2));
	if(
		(x<0||WIDTH<x)
		||(y<0||HEIGHT<y)
		){
		U+=sqrt(pow((WIDTH/2-x),2)+pow((HEIGHT/2-y),2))*100;
	}
	for(int i=0;i<PARTICLE_COUNT;i++){
		double r;
		double dU;
		r=sqrt(pow(x-stateA[i].x,2)+pow(y-stateA[i].y,2));
		dU=(1/r+20*(exp(r/10)-1)/(exp(r/10)+1))*10;
		if(!isnan(dU)){
			U+=dU;
		}
		//U+=-3000/r
	}
	return U;
}

double Balls_force(__global const particle* stateA,int no,double* fx,double* fy){
	double dp=0.00001;
	*fx=-(
		 potential(stateA,stateA[no].x+dp,stateA[no].y)
		-potential(stateA,stateA[no].x-dp,stateA[no].y)
		)/(dp*2);
	*fx+=-stateA[no].vx/10;
	*fy=-(
		 potential(stateA,stateA[no].x,stateA[no].y+dp)
		-potential(stateA,stateA[no].x,stateA[no].y-dp)
		)/(dp*2);
	*fy+=-stateA[no].vy/10;
}

__kernel void kmain(__global const particle* stateA,__global particle* stateB){
	double dt=0.1;
	double fx,fy;
	int i = get_global_id(0);

	Balls_force(stateA,i,&fx,&fy);
	if(isnan(fx) || isnan(fy) || isinf(fx) || isinf(fy)){
		fx=0;
		fy=0;
	}
	stateB[i].x=stateA[i].x+stateA[i].vx*dt;
	stateB[i].y=stateA[i].y+stateA[i].vy*dt;
	stateB[i].vx=stateA[i].vx+fx*dt;
	stateB[i].vy=stateA[i].vy+fy*dt;
}

