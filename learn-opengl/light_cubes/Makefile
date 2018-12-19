
PROJECT_PATH = /home/wyatt/graphics/learn-opengl

CFLAGS  = -std=c++11 -I$(PROJECT_PATH)/include -I$(PROJECT_PATH)/shaders -pedantic -Wall
LDFLAGS =  `pkg-config --static --libs glfw3`

make: main.cpp
	g++ $(CFLAGS) main.cpp glad.c -o main $(LDFLAGS)

.PHONY: main clean

clean:
	rm -f main 

