#ファイル定義
#TODO:ファイルを定義する。
TARGET=planets.exe
OBJS=main.o sdlwrapper.o openclWrapper.o particleCalculator.o

#ビルドの過程で生成されるファイル
PRODUCTS=*.o *.out

#コンパイラ設定
LIBS=-L/mingw64/lib -lOpenCL -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf
GCC_COMPILEOPTION=-O3 -g -I/mingw64/include
GXX_COMPILEOPTION=
LINKOPTION=


.PHONY:all clean rebuild
all: $(TARGET)

#ターゲットの作成
$(TARGET):$(OBJS)
	g++ $(LINKOPTION) $^  $(LIBS) -o $@

#サフィックスルール
.SUFFIXES:.c .o .cpp
.c.o:
	gcc $(GCC_COMPILEOPTION) $< -c

.cpp.o:
	g++ $(GXX_COMPILEOPTION) $< -c

#ユーティリティコマンド定義
rebuild:
	make clean
	make

clean:
	rm -rf $(PRODUCTS) $(TARGET)

run:all
	make rebuild
	./$(TARGET)

debug:all
	$(DEBUGGER) $(TARGET)
