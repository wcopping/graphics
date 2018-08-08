
VULKAN_SDK_PATH = /home/wyatt/vulkan/1.1.77.0/x86_64
STB_INCLUDE_PATH = /home/wyatt/graphics/tutorial-beyond-ch19

CFLAGS  = -std=c++11 -O3 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH)
LDFLAGS =  -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

look-and-see: main.cpp
	g++ $(CFLAGS) -o look-and-see main.cpp $(LDFLAGS)

.PHONY: look clean

look: look-and-see 
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d ./look-and-see

clean:
	rm -f look-and-see

