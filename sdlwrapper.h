#include<SDL2/SDL.h>

#define TEXTURE_COUNT_MAX	(1024)
#define FONT_COUNT_MAX		(32)
#define KEY_SCANCODE_MAX	(285)

#define INIT_TITLE		(1)
#define INIT_POS		(2)
#define INIT_SIZE		(4)
#define INIT_FLAG		(8)

#define DRAWIMAGE_SRC		(1)
#define DRAWIMAGE_DST		(2)
#define DRAWIMAGE_ANGLE		(4)
#define DRAWIMAGE_CENTER	(8)
#define DRAWIMAGE_FLIP		(16)

#define DRAWTEXT_POS		(1)
#define DRAWTEXT_SCALE		(2)

#define window	((drawable)0)

typedef unsigned int drawable;

typedef unsigned int font;
//ウィンドウとテクスチャを統合した番号(ウィンドウは0)

#define MOUSE_LEFT	(1)
#define MOUSE_MIDDLE	(2)
#define MOUSE_RIGHT	(4)
#define MOUSE_X1	(8)
#define MOUSE_X2	(16)

struct _MouseState {
	int x;
	int y;
	int vx;
	int vy;
	unsigned int state;
	unsigned int DownButton;
	unsigned int UpButton;
}; 

extern char KeyState[KEY_SCANCODE_MAX];
extern struct _MouseState	MouseState;

void init(int avl,...);
void quit();
void initStreamDrawing(drawable target);
void quitStreamDrawing(drawable target);
void drawPoint(drawable target,int x,int y);
void drawLine(drawable target,int x1,int y1,int x2,int y2);
void drawRect(drawable target,int x1,int y1,int x2,int y2);
void fillRect(drawable target,int x1,int y1,int x2,int y2);
void clear(drawable target);
void drawImage(drawable target,drawable source,int avl,...);
drawable newDrawable(int width,int height);
drawable newDrawable2(int width,int height,int format);
drawable loadDrawable(const char* filename);
int drawableWidth(drawable d);
int drawableHeight(drawable d);
void delDrawable(drawable d);
void setFrameFunc(void (*F)());
void setColor(int r,int g,int b,int a);
font loadFont(const char* file,int ptsize);
void setFont(font f);
void delFont(font f);
drawable renderText(font f,int r,int g,int b,int a,const char* str);
void drawTextf(drawable target,const char* format,int avl,...);
int setAlphaMod(drawable d,int alpha);
int setBlendMode(drawable d,int blendmode);
int setColorMod(drawable d,int r,int g,int b);
void mainLoop(void (*FrameFunc)());
