#include "pc_common.h"
#include "config.h"

double potential(__global const particle* stateA,double x,double y){
	double U=0;
//	U=sqrt(pow((INTERNAL_WIDTH/2-x),2)+pow((INTERNAL_HEIGHT/2-y),2));
	if(
		(x<0||INTERNAL_WIDTH<x)
		||(y<0||INTERNAL_HEIGHT<y)
		){
		U+=sqrt(pow((double)(INTERNAL_WIDTH/2-x),2.0)+pow((double)(INTERNAL_HEIGHT/2-y),2.0))*100;
	}
	for(int i=0;i<PARTICLE_COUNT;i++){
		double r;
		double dU;
		r=sqrt(pow((double)(x-stateA[i].x),2.0)+pow((double)(y-stateA[i].y),2.0));
		dU=(1/r+20*(exp(r/10)-1)/(exp(r/10)+1))*10;
		if(!isnan(dU)){
			U+=dU;
		}
	}
	return U;
}

double potential2(__global const particle* stateA,double x,double y){
	double U=0;
//	U=sqrt(pow((INTERNAL_WIDTH/2-x),2)+pow((INTERNAL_HEIGHT/2-y),2));
	if(
		(x<0||INTERNAL_WIDTH<x)
		||(y<0||INTERNAL_HEIGHT<y)
		){
		U+=sqrt(pow((double)(INTERNAL_WIDTH/2-x),2.0)+pow((double)(INTERNAL_HEIGHT/2-y),2.0))*100;
	}
	for(int i=0;i<PARTICLE_COUNT;i++){
		double r;
		double dU;
		r=sqrt(pow((double)(x-stateA[i].x),2.0)+pow((double)(y-stateA[i].y),2.0));
		dU=1000/r;
		if(!isnan(dU)){
			U+=dU;
		}
	}
	return U;
}


__kernel void kmain(__global const particle* stateA,__global particle* stateB,__global option* opt){
	double dt=0.1;
	double dp=0.00001;
	double fx,fy;
	double x,y,vx,vy;

	int i = get_global_id(0);

	x=stateA[i].x;
	y=stateA[i].y;
	vx=stateA[i].vx;
	vy=stateA[i].vy;

	switch(opt->mode){
	case 0:
		fx=-(
			 potential(stateA,x+dp,y)
			-potential(stateA,x-dp,y)
			)/(dp*2);
		fx+=-vx/10;

		fy=-(
			 potential(stateA,x,y+dp)
			-potential(stateA,x,y-dp)
			)/(dp*2);
		fy+=-vy/10;
		break;
	case 1:
		fx=-(
			 potential2(stateA,x+dp,y)
			-potential2(stateA,x-dp,y)
			)/(dp*2);
		fx+=-vx/10;

		fy=-(
			 potential2(stateA,x,y+dp)
			-potential2(stateA,x,y-dp)
			)/(dp*2);
		fy+=-vy/10;
		break;
	}

	if(isnan(fx) || isnan(fy) || isinf(fx) || isinf(fy)){
		fx=0;
		fy=0;
	}

	stateB[i].x=x+vx*dt;
	stateB[i].y=y+vy*dt;
	stateB[i].vx=vx+fx*dt;
	stateB[i].vy=vy+fy*dt;
}
