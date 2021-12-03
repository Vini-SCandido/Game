all:
	gcc main.c -o main.exe -O1 -Wall -std=c99 -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm -Wl,--subsystem,windows