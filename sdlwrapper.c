#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_ttf.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>
#include"sdlwrapper.h"

/*
ウィンドウは一つに限定(つまり、ウィンドウとレンダラは1つづつ)
原則、別バッファはテクスチャ。
キーボードやマウスのイベントはフレームごとに処理し、各キー及びマウスボタンについて、押されているか、今押されたか、今離されたか、今押した後に離されたかを取得できるようにする。
マウスポインタの位置も記録する。
ゲームパッドも同様にする。
*/

SDL_Window* Window;
SDL_Renderer* Renderer;
font CurrentFont=0;
SDL_Texture* Textures[TEXTURE_COUNT_MAX];
struct{
	int w;
	int h;
	Uint32 format;
	int access;
	struct {
		SDL_Texture* texture;
		int locked;
		void* pixels;
		int pitch;
		SDL_PixelFormat* format;
	} stream;
} TextureData[TEXTURE_COUNT_MAX];
TTF_Font*    Fonts[FONT_COUNT_MAX];
char KeyState[KEY_SCANCODE_MAX];
struct _MouseState MouseState;

void bindDrawable(drawable d,SDL_Texture* T);

void init(int avl,...){
	va_list ap;
	const char* title="test";
	int x=SDL_WINDOWPOS_UNDEFINED;
	int y=SDL_WINDOWPOS_UNDEFINED;
	int w=640;
	int h=480;
	int flag=SDL_WINDOW_OPENGL;

	va_start(ap,avl);
	if(avl&INIT_TITLE){
		title=va_arg(ap,char*);
	}
	if(avl&INIT_POS){
		x=va_arg(ap,int);
		y=va_arg(ap,int);
	}
	if(avl&INIT_SIZE){
		w=va_arg(ap,int);
		h=va_arg(ap,int);
	}
	if(avl&INIT_FLAG){
		flag=va_arg(ap,int);
	}
	va_end(ap);


	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	TTF_Init();
	Window=SDL_CreateWindow(title,x,y,w,h,flag);
	Renderer=SDL_CreateRenderer(Window,-1,SDL_RENDERER_ACCELERATED);

	TextureData[0].w=w;
	TextureData[0].h=h;
	TextureData[0].format=SDL_GetWindowPixelFormat(Window);
	TextureData[0].access=SDL_TEXTUREACCESS_TARGET;

	for(int i=0;i<TEXTURE_COUNT_MAX;i++)Textures[i]=NULL;
	for(int i=0;i<FONT_COUNT_MAX;i++)Fonts[i]=NULL;
	for(int i=0;i<KEY_SCANCODE_MAX;i++)KeyState[i]=0;
}

void quit(){
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void initStreamDrawing(drawable target){
	if(!TextureData[target].stream.texture){
		Uint32 format;
		format=TextureData[target].format;
		TextureData[target].stream.format=SDL_AllocFormat(format);
		TextureData[target].stream.texture=SDL_CreateTexture(Renderer,format,SDL_TEXTUREACCESS_STREAMING,drawableWidth(target),drawableHeight(target));
		SDL_SetTextureBlendMode(TextureData[target].stream.texture,SDL_BLENDMODE_BLEND);
	}
	SDL_LockTexture(TextureData[target].stream.texture,0,&TextureData[target].stream.pixels,&TextureData[target].stream.pitch);
	TextureData[target].stream.locked=1;
}

void quitStreamDrawing(drawable target){
	SDL_UnlockTexture(TextureData[target].stream.texture);
	SDL_SetRenderTarget(Renderer,Textures[target]);
	SDL_RenderCopy(Renderer,TextureData[target].stream.texture,0,0);
	TextureData[target].stream.locked=0;
}

void drawPoint(drawable target,int x,int y){
	if(TextureData[target].stream.locked){
		SDL_Texture* T=TextureData[target].stream.texture;
		Uint8 r,g,b,a;
		SDL_GetRenderDrawColor(Renderer,&r,&g,&b,&a);
		Uint32* targetPixel=(Uint32*)(TextureData[target].stream.pixels+TextureData[target].stream.pitch*y)+x;
		*targetPixel=SDL_MapRGBA(TextureData[target].stream.format,r,g,b,a);
	}else{
		SDL_SetRenderTarget(Renderer,Textures[target]);
		SDL_RenderDrawPoint(Renderer,x,y);
	}
}

void drawLine(drawable target,int x1,int y1,int x2,int y2){
	SDL_SetRenderTarget(Renderer,Textures[target]);
	SDL_RenderDrawLine(Renderer,x1,y1,x2,y2);
}

SDL_Rect RECT(int x1,int y1,int x2,int y2){
	return (SDL_Rect){(x1<x2)?x1:x2,(y1<y2)?y1:y2,(x1<x2)?x2-x1:x1-x2,(y1<y2)?y2-y1:y1-y2};
}

void drawRect(drawable target,int x1,int y1,int x2,int y2){
	SDL_Rect R=RECT(x1,y1,x2,y2);
	SDL_SetRenderTarget(Renderer,Textures[target]);
	SDL_RenderDrawRect(Renderer,&R);
}

void fillRect(drawable target,int x1,int y1,int x2,int y2){
	SDL_Rect R=RECT(x1,y1,x2,y2);
	SDL_SetRenderTarget(Renderer,Textures[target]);
	SDL_RenderFillRect(Renderer,&R);
}

void clear(drawable target){
	SDL_SetRenderTarget(Renderer,Textures[target]);
	SDL_RenderClear(Renderer);
}

void drawImage(drawable target,drawable source,int avl,...){
	va_list ap;
	SDL_Rect Rsrc,Rdst;
	SDL_Point C=(SDL_Point){0,0};
	SDL_Rect* srcrect=0;
	SDL_Rect* dstrect=0;
	int angle=0;
	SDL_Point* center=0;
	SDL_RendererFlip flip=SDL_FLIP_NONE;
	int EX=0;
	
	va_start(ap,avl);
	if(avl&DRAWIMAGE_SRC){
		int x1,y1,x2,y2;
		x1=va_arg(ap,int);
		y1=va_arg(ap,int);
		x2=va_arg(ap,int);
		y2=va_arg(ap,int);
		Rsrc=RECT(x1,y1,x2,y2);
		srcrect=&Rsrc;
	}
	if(avl&DRAWIMAGE_DST){
		int x1,y1,x2,y2;
		x1=va_arg(ap,int);
		y1=va_arg(ap,int);
		x2=va_arg(ap,int);
		y2=va_arg(ap,int);
		Rdst=RECT(x1,y1,x2,y2);
		dstrect=&Rdst;
	}
	if(avl&DRAWIMAGE_ANGLE){
		angle=va_arg(ap,double);
		EX=1;
	}
	if(avl&DRAWIMAGE_CENTER){
		C=(SDL_Point){va_arg(ap,int),va_arg(ap,int)};
		center=&C;
		EX=1;
	}
	if(avl&DRAWIMAGE_FLIP){
		flip=va_arg(ap,SDL_RendererFlip);
		EX=1;
	}
	va_end(ap);

	SDL_SetRenderTarget(Renderer,Textures[target]);
	if(EX){
		SDL_RenderCopyEx(Renderer,Textures[source],srcrect,dstrect,angle,center,flip);
	}else{
		SDL_RenderCopy(Renderer,Textures[source],srcrect,dstrect);
	}
}

drawable allocateDrawable(){
	int i;
	for(i=1;i<TEXTURE_COUNT_MAX;i++){
		if(!Textures[i]){
			Textures[i]=(SDL_Texture*)(-1);
			TextureData[i].w=0;
			TextureData[i].h=0;
			TextureData[i].stream.texture=0;
			TextureData[i].stream.locked=0;
			TextureData[i].stream.pixels=0;
			TextureData[i].stream.pitch=0;
			TextureData[i].stream.format=0;
			break;
		}
	}
	if(i==TEXTURE_COUNT_MAX)i=-1;
	return i;
}

void freeDrawable(drawable d){
	Textures[d]=0;
}

drawable newDrawable(int width,int height){
	drawable d=newDrawable2(width,height,SDL_PIXELFORMAT_RGBA8888);
	SDL_SetTextureBlendMode(Textures[d],SDL_BLENDMODE_BLEND);
	return d;
}

drawable newDrawable2(int width,int height,int format){
	SDL_Texture* T;
	drawable d=allocateDrawable();
	T=SDL_CreateTexture(Renderer,format,SDL_TEXTUREACCESS_TARGET,width,height);
	bindDrawable(d,T);
	return d;
}

drawable loadDrawable(const char* filename){
	SDL_Surface* S;
	SDL_Texture* T;
	int w,h;
	drawable d;
	Uint32 format;

	S=IMG_Load(filename);
	T=SDL_CreateTextureFromSurface(Renderer,S);
	SDL_FreeSurface(S);
	d=allocateDrawable();
	bindDrawable(d,T);
	return d;
}


int drawableWidth(drawable d){
	return TextureData[d].w;
}

int drawableHeight(drawable d){
	return TextureData[d].h;
}


void setColor(int r,int g,int b,int a){
	SDL_SetRenderDrawColor(Renderer,r,g,b,a);
}

void delDrawable(drawable d){
	SDL_DestroyTexture(Textures[d]);
	if(TextureData[d].stream.texture){
		SDL_DestroyTexture(TextureData[d].stream.texture);
		SDL_FreeFormat(TextureData[d].stream.format);
	}
	freeDrawable(d);
}

font allocateFont(){
	int i;
	for(i=0;i<FONT_COUNT_MAX;i++){
		if(!Fonts[i]){
			Fonts[i]=(TTF_Font*)(-1);
			break;
		}
	}
	if(i==FONT_COUNT_MAX)i=-1;
	return i;
}

void freeFont(font d){
	Fonts[d]=0;
}

font loadFont(const char* file,int ptsize){
	TTF_Font* F;
	font f;
	F=TTF_OpenFont(file,ptsize);
	f=allocateFont();
	Fonts[f]=F;
	return f;
}

void setFont(font f){
	CurrentFont=f;
}


void delFont(font f){
	TTF_CloseFont(Fonts[f]);
	freeFont(f);
}

void bindDrawable(drawable d,SDL_Texture* T){
	Uint32 format;
	int access;
	int w,h;
	Textures[d]=T;
	SDL_QueryTexture(T,
			&TextureData[d].format,
			&TextureData[d].access,
			&TextureData[d].w,
			&TextureData[d].h
			);
}

drawable renderText(font f,int r,int g,int b,int a,const char* str){
	SDL_Surface* S;
	SDL_Texture* T;
	SDL_Color color;
	drawable d;

	color.r=r;
	color.g=g;
	color.b=b;
	color.a=a;
	
	S=TTF_RenderUTF8_Blended(Fonts[f],str,color);
	T=SDL_CreateTextureFromSurface(Renderer,S);
	SDL_FreeSurface(S);
	d=allocateDrawable();
	bindDrawable(d,T);
	return d;
}

void drawTextf(drawable target,const char* format,int avl,...){
	char buf[2048];
	char buf2[4096];
	Uint8 r,g,b,a;
	va_list ap;
	int x=0,y=0;
	int W,H;
	double scale=1;
	SDL_GetRenderDrawColor(Renderer,&r,&g,&b,&a);
	va_start(ap,avl);
	if(avl&DRAWTEXT_POS){
		x=va_arg(ap,int);
		y=va_arg(ap,int);
	}
	if(avl&DRAWTEXT_SCALE){
		scale=va_arg(ap,double);
	}
	va_end(ap);
	int i=0;
	do{
		for(int k=0;;k++,i++){
			if(format[i]=='\n' || format[i]==0){
				buf[k]=0;
				i++;
				break;
			}else{
				buf[k]=format[i];
			}
		}
		drawable d;
		vsnprintf(buf2,4096,buf,ap);
		d=renderText(CurrentFont,r,g,b,a,buf2);
		W=drawableWidth(d)*scale;
		H=drawableHeight(d)*scale;
		drawImage(target,d,DRAWIMAGE_DST,x,y,x+W,y+H);
		y+=H;
		delDrawable(d);
	}while(format[i]);
}

int setAlphaMod(drawable d,int alpha){
	return SDL_SetTextureAlphaMod(Textures[d],alpha);
}

int setBlendMode(drawable d,int blendMode){
	return SDL_SetTextureBlendMode(Textures[d],blendMode);
}

int setColorMod(drawable d,int r,int g,int b){
	return SDL_SetTextureColorMod(Textures[d],r,g,b);
}

unsigned int SDL_BUTTON2MOUSE(int button){
	switch(button){
	case SDL_BUTTON_LEFT:
		return MOUSE_LEFT;
	case SDL_BUTTON_MIDDLE:
		return MOUSE_MIDDLE;
	case SDL_BUTTON_RIGHT:
		return MOUSE_RIGHT;
	case SDL_BUTTON_X1:
		return MOUSE_X1;
	case SDL_BUTTON_X2:
		return MOUSE_X2;
	}
}

void mainLoop(void (*FrameFunc)()){
	SDL_Event event;
	while(1){
		FrameFunc();
		SDL_RenderPresent(Renderer);
		MouseState.DownButton=0;
		MouseState.UpButton=0;
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_KEYDOWN:
				KeyState[event.key.keysym.scancode]=1;
				break;
			case SDL_KEYUP:
				KeyState[event.key.keysym.scancode]=0;
				break;
			case SDL_MOUSEMOTION:
				MouseState.x=event.motion.x;
				MouseState.y=event.motion.y;
				MouseState.vx=event.motion.xrel;
				MouseState.vy=event.motion.yrel;
				break;
			case SDL_MOUSEBUTTONUP:
				MouseState.state ^= MouseState.state & SDL_BUTTON2MOUSE(event.button.button);
				MouseState.UpButton |= SDL_BUTTON2MOUSE(event.button.button);
				break;
			case SDL_MOUSEBUTTONDOWN:
				MouseState.state|=SDL_BUTTON2MOUSE(event.button.button);
				MouseState.DownButton|=SDL_BUTTON2MOUSE(event.button.button);
				break;
			case SDL_QUIT:
				return;
			}
		}
	}
}
