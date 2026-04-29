CXX = g++

CXXFLAGS = -I E:/SDL/x86_64-w64-mingw32/include \
           -I E:/SDL3_image/x86_64-w64-mingw32/include \
           -I E:/SDL3_ttf/x86_64-w64-mingw32/include \
           -I E:/SDL3_mixer/x86_64-w64-mingw32/include

LDFLAGS = -L E:/SDL/x86_64-w64-mingw32/lib \
          -L E:/SDL3_image/x86_64-w64-mingw32/lib \
          -L E:/SDL3_ttf/x86_64-w64-mingw32/lib \
          -L E:/SDL3_mixer/x86_64-w64-mingw32/lib \
          -lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer \
          -mwindows

SRC = $(wildcard src/*.cpp)
OUT = main.exe

all:
	$(CXX) $(SRC) -o $(OUT) $(CXXFLAGS) $(LDFLAGS)

clean:
	del $(OUT)