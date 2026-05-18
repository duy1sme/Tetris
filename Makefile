CXX = g++

# 1. Tự động phát hiện kiến trúc của Compiler (x86_64 hoặc i686)
ARCH := $(shell g++ -dumpmachine)

# Khởi tạo các cờ mặc định
CXXFLAGS = 
LDFLAGS = -lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer -mwindows

# 2. Kiểm tra xem cấu trúc thư mục lib/x64 hoặc lib/x86 có tồn tại không để dùng Portable
ifeq ($(findstring x86_64,$(ARCH)),x86_64)
    # 64-bit (Máy nhà)
    ifeq ($(wildcard lib/x64),lib/x64)
        CXXFLAGS += -I include
        LDFLAGS += -L lib/x64
        DLL_SRC = lib/x64/*.dll
    else ifeq ($(wildcard lib),lib)
        # Fallback cho thư mục lib cũ (chứa file 64-bit mặc định)
        CXXFLAGS += -I include
        LDFLAGS += -L lib
    endif
else
    # 32-bit / i686 (Máy trường)
    ifeq ($(wildcard lib/x86),lib/x86)
        CXXFLAGS += -I include
        LDFLAGS += -L lib/x86
        DLL_SRC = lib/x86/*.dll
    endif
endif

SRC = $(wildcard src/*.cpp)
OUT = bin/main.exe

all: copy_dlls
	$(CXX) $(SRC) -o $(OUT) $(CXXFLAGS) $(LDFLAGS)

copy_dlls:
	@if not exist bin mkdir bin
ifdef DLL_SRC
	@echo Copying DLLs for $(ARCH)...
	@copy /Y $(subst /,\,$(DLL_SRC)) bin >nul 2>&1 || copy $(DLL_SRC) bin >nul 2>&1 || echo "No local DLLs to copy, relying on system."
endif

clean:
	del /Q bin\main.exe