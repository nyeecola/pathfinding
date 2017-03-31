OS := $(shell uname)

ifeq ($(OS),Darwin)
# Run MacOS commands
	OS_COMMAND := clang++ -I/usr/local/Cellar/fltk/1.3.4/include -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -o 'main' 'main.cpp' /usr/local/Cellar/fltk/1.3.4/lib/libfltk.a -lpthread -framework Cocoa

else
# check for Linux and run other commands
	OS_COMMAND := g++ -I/usr/local/include -I/usr/local/include/FL/images -I/usr/include/freetype2 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -DDEBUG -o 'main' 'main.cpp' /usr/local/lib/libfltk.a /usr/local/lib/libfltk_images.a -lXrender -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lm -lX11 -lpng --std=c++11 -Wall -Werror

endif

all: build run

	
build:
	$(OS_COMMAND)

run:
	./main
