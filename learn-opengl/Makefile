
PROJECT_PATH = /home/wyatt/graphics/learn-opengl

CFLAGS  = -std=c++11 -I$(PROJECT_PATH)/include -pedantic -Wall
LDFLAGS =  `pkg-config --static --libs glfw3`

make: ./source/main.cpp
	g++ $(CFLAGS) ./source/main.cpp ./source/glad.c -o main $(LDFLAGS)

.PHONY: main clean

clean:
	rm -f main 

