#! /bin/make
  
CFLAGS = -g -Wall
CFLAGS += `pkg-config sdl2 --cflags`
CFLAGS += `pkg-config SDL2_ttf --cflags`
CFLAGS += `pkg-config --cflags fontconfig`
LIBS = `pkg-config sdl2 --libs`
LIBS += `pkg-config SDL2_ttf --libs`
LIBS += `pkg-config --libs fontconfig`
OBJS = program.o editstar.o buffer.o

editstar: main.cpp $(OBJS)
	g++ $(CFLAGS) $< $(OBJS) $(LIBS)

all: demo.cpp
	g++ $(CFLAGS) $< $(LIBS)

.cpp.o:
	g++ -c $(CFLAGS) $< -o $@

