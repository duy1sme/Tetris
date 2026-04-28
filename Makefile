CXX = g++
CXXFLAGS = -I SDL/include -g
LDFLAGS = -L SDL/lib -lSDL3 -lSDL3_image -lSDL3_ttf -mwindows

SRC = $(wildcard src/*.cpp)
OUT = main.exe

all:
	$(CXX) $(SRC) -o $(OUT) $(CXXFLAGS) $(LDFLAGS)

clean:
	del $(OUT)