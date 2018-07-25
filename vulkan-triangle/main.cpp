
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>


const int WIDTH  = 800;
const int HEIGHT = 600;

const std::vector<const char*> validation_layers = {
  "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
  const bool enable_validation_layers = false;
#else
  const bool enable_validation_layers = true;
#endif

VkResult create_debug_report_callback_ext(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugReportCallbackEXT* p_callback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_callback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_report_callback_ext(VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* p_allocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, p_allocator);
    }
}

class HelloTriangleApplication
{
public:
  void run()
  {
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
  }

private:
  GLFWwindow* window;
  VkInstance instance;
  VkDebugReportCallbackEXT callback;


  void init_window()
  {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "vulkan", nullptr, nullptr);
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
      VkDebugReportFlagsEXT flags,
      VkDebugReportObjectTypeEXT obj_type,
      uint64_t obj,
      size_t location,
      int32_t code,
      const char* layer_prefix,
      const char* msg,
      void* user_data) {
    std::cerr << "Validation Layer: " << msg << std::endl; 
    return VK_FALSE;
  }
  
  std::vector<const char*> get_required_extensions()
  {
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (enable_validation_layers) {
      extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
  }

  bool check_validation_layer_support()
  {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : validation_layers) {
      bool layer_found = false;

      for (const auto& layer_properties : available_layers) {
        if (strcmp(layer_name, layer_properties.layerName) == 0) {
          layer_found = true;
          break;
        }
      }
      
      if (!layer_found) {
        return false;
      }
    }

    return true;
  }

  /*
  bool check_glfw_reqs()
  {
    uint32_t ext_count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&ext_count);

    uint32_t available_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(available_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &available_count, available_extensions.data());

    for (const char* ext_name : extensions) {
      bool ext_found = false;

      for (const auto& ext_properties : available_extensions) {
        if (strcmp(ext_name, ext_properties.extensionName) == 0) {
          ext_found = true;
          break;
        }
      }
      if (!ext_found) {
        return false;
      }
    }

  return true;
  }
  */



  void create_instance()
  {
    if (enable_validation_layers && !check_validation_layer_support()) {
      throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
    app_info.pEngineName        = "No Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1,0,0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;

    auto extensions = get_required_extensions();
    create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    if (enable_validation_layers) {
      create_info.enabledLayerCount     = static_cast<uint32_t>(validation_layers.size());
      create_info.ppEnabledLayerNames   = validation_layers.data();
    } else {
        create_info.enabledLayerCount   = 0;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }

  }

  void setup_debug_callback()
  {
    if (!enable_validation_layers) return;

    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags                = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback          = debug_callback;

    if (create_debug_report_callback_ext(instance, &create_info, nullptr, &callback) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug callback!");
    }
  }


  void init_vulkan()
  {
    create_instance();
    setup_debug_callback();
  }

  void main_loop()
  {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  void cleanup()
  {
    if (enable_validation_layers) {
      destroy_debug_report_callback_ext(instance, callback, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }
};

int main()
{
  HelloTriangleApplication app;

  try {
    app.run();
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
