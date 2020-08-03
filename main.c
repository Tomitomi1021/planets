#include"particleCalculator.h"
#include"config.h"
#include"sdlwrapper.h"
#include<stdlib.h>
#include<time.h>
#include<stdio.h>

struct {
	particle calc[PARTICLE_COUNT];
	struct {
		int r;
		int g;
		int b;
	} color[PARTICLE_COUNT];
}	Particles;

void particleInit(){
	for(int i=0;i<PARTICLE_COUNT;i++){
		Particles.color[i].r=rand()%256;
		Particles.color[i].g=rand()%256;
		Particles.color[i].b=rand()%256;
		Particles.calc[i].x=rand()%INTERNAL_WIDTH;
		Particles.calc[i].y=rand()%INTERNAL_HEIGHT;
		Particles.calc[i].vx=rand()%100-50;
		Particles.calc[i].vy=rand()%100-50;
	}
	pc_initialize(Particles.calc);
	{
		option opt;
		opt.mode=0;
		pc_setoption(&opt);
	}
}

void frame(){
	static int t=0;
	static drawable d=0;

	if(d==0)d=newDrawable(INTERNAL_WIDTH,INTERNAL_HEIGHT);

	pc_calculate(Particles.calc);

	setColor(0,0,0,17);
	clear(d);
	for(int i=0;i<PARTICLE_COUNT;i++){
		int r,g,b;
		double x,y;
		r=Particles.color[i].r;
		g=Particles.color[i].g;
		b=Particles.color[i].b;
		x=Particles.calc[i].x;
		y=Particles.calc[i].y;
		setColor(r,g,b,255);
		drawPoint(d,x,y);
	}
	setBlendMode(window,SDL_BLENDMODE_NONE);
	drawImage(window,d,0);

	if(KeyState[SDL_SCANCODE_Z]){
		option opt;
		opt.mode=1;
		pc_setoption(&opt);
	}else{
		option opt;
		opt.mode=0;
		pc_setoption(&opt);
	}
}


int WinMain(){
	int ret;
	ret=pc_init();
	if(ret==PC_FAIL)return -1;
	particleInit();
	init(INIT_SIZE|INIT_FLAG,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_WINDOW_FULLSCREEN_DESKTOP);
	mainLoop(frame);
	quit();
	pc_quit();
}
