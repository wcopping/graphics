
VULKAN_SDK_PATH = /home/wyatt/vulkan/1.1.77.0/x86_64

CFLAGS  = -std=c++11 -I$(VULKAN_SDK_PATH)/include
LDFLAGS =  -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

vulkan-test: main.cpp
	g++ $(CFLAGS) -o vulkan-test main.cpp $(LDFLAGS)

.PHONY: test clean

test: vulkan-test
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d ./vulkan-test

clean:
	rm -f vulkan-test
