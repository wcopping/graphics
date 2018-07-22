
VULKAN_SDK_PATH = /home/wyatt/vulkan/1.1.77.0/x86_64

CFLAGS  = -std=c++11 -I$(VULKAN_SDK_PATH)/include
LDFLAGS =  -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

vulkan-triangle: main.cpp
	g++ $(CFLAGS) -o vulkan-triangle main.cpp $(LDFLAGS)

.PHONY: triangle clean

triangle: vulkan-triangle
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d ./vulkan-triangle

clean:
	rm -f vulkan-triangle

