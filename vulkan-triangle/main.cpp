
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>
#include <set>


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
  // We do not need to provide a surface if we only need Vulkan for
  // off-screen rendering (e.g. computation only)
  // 
  // Because Vulkan is platform agnostic we need to use extensions from
  // Window System Integration.
  //
  // First we'll need to enable VK_KHR_SURFACE, this was already enabled
  // through glfwGetRequiredInstanceExtensions
  //
  // We don't have to specify which platform we are using (that doesn't really
  // make sense since we're working with Vulkan anyway) because GLFW figures out
  // which extensions are required based off of what platform it's running on
  VkSurfaceKHR surface;
  // Handle for the for VkQueue for the logical device creation procedure
  VkQueue present_queue;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  // stores logical device handle
  VkDevice device;
  // class member to store a handle to the graphics queue
  // device queues are implicitly cleaned up when the device is destroyed
  // therefore we don't need to worry about cleaning this up in cleanup()
  VkQueue graphics_queue;


  void init_vulkan()
  {
    create_instance();
    setup_debug_callback();
    create_surface();
    pick_physical_device();
    create_logical_device();
  }

  void main_loop()
  {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

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



  void cleanup()
  {
    vkDestroyDevice(device, nullptr);
    if (enable_validation_layers) {
      destroy_debug_report_callback_ext(instance, callback, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  // look for and select graphics card for use by vulkan api features
  // we need to store this device as a VkPhysicalDevice but we do not need to
  // do anything special to destroy it once we are done as that is taken care of
  // already, implicitly, when we destroy the VkInstance
  void pick_physical_device()
  {
    // listing graphics cards is somewhat like listing extensions
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    // if we can't find any devices with vulkan support then we stop here
    if (device_count == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    // otherwise we can create an array to hold all vkdevice handles
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    for (const auto& device : devices) {
      if (is_device_suitable(device)) {
          physical_device = device;
          break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
      throw std::runtime_error("Failed to find a suitable GPU!");
    }
  }

  // must check to make sure that the device we have is capable of the tasks
  // we plan to use it for in Vulkan
  // 
  // This function is obviously extensible, as any new functional requirement
  // can be quickly checked for here
  // 
  // You can either pick the first device that meets the requirements by
  // returning a long conditional (return featureX && featureY && capabilityb)
  // or you can create an orderd list and return the device with the highest
  // score, create a separate function that looks at the devices capabilities
  // by mutating and returning a local score variable.
  // 
  bool is_device_suitable(VkPhysicalDevice device) {
    /*
    // We can obtain basic device properties like name, type, etc. by querying
    // for them using the following function
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    // support for optional features can be obtained by using the following
    // function
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);
    return true;
    */
    QueueFamilyIndices indices = find_queue_families(device);

    return indices.is_complete();
  }

  // We must check if we have access to certain queues and queue families
  //
  // We need queues because almost every operation in Vulkan requires
  // commands to be submitted throuhg a queue.
  //
  // Queues from differnt queue families only allow certain subsets of
  // operations to be loaded in
  //
  // To accomplish this task we will make use of a new struct and find function
  // we implement here
  struct QueueFamilyIndices {
    // -1 denotes "not found"
    // We must set both the families required for presentation and drawing
    // because it is possible that one family may be able to draw images but
    // not present them and vice versa
    int graphics_family = -1;
    int present_family  = -1;

    bool is_complete() {
      return graphics_family >= 0 && present_family >= 0;
    }
  };

  QueueFamilyIndices find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());


    // the VkQueueFamilyProperties struct contains information such as the type
    // of operations supported by the family and how many queues can be created
    // based off that family
    //
    // We need to find a queue family with at least the support of
    // VK_QUEUE_GRAPHICS_BIT
    //
    // Here we treat the queue families that support presentation and drawing
    // as separate but we can add logic to this function so that we prefer to
    // use the queue families that are both draw and present capable. This would
    // possibly increase performance.
    int i = 0;
    for (const auto& queue_family : queue_families) {
      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
      if (queue_family.queueCount > 0 && present_support) {
        indices.present_family = i;
      }
      if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphics_family = i;
      }
      if (indices.is_complete()) {
        break;
      }
    
      ++i;
    }

    return indices;
  }

  // We must specify many details to properly create a logical device
  //
  // (For now?) We only really need one queue in a family, that's because
  // we can create all command buffers on multiple threads and then combine
  // them all at once on the main thread via a low overhead function call
  //
  void create_logical_device() {
    // One such detail is how many queues we want inside of a queue family
    // we will use a struct that we create to accomodate this necessity
    QueueFamilyIndices indices = find_queue_families(physical_device);

    // Because we have multiple queue families that we can work with, we will
    // create a set of all unique queue families necessary for the required
    // queues???
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<int> unique_queue_families = {indices.graphics_family, indices.present_family};

    float queue_priority = 1.0f;
    for (int queue_family : unique_queue_families) {
      VkDeviceQueueCreateInfo queue_create_info = {};
      queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_create_info.queueFamilyIndex = queue_family;
      queue_create_info.queueCount = 1;
      queue_create_info.pQueuePriorities = &queue_priority;
      queue_create_infos.push_back(queue_create_info);
    }
      
    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // First we add pointers to the queue creation info and the device features
    // we obtained above
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    // We will enable the same validation layers for devices as we did for the
    // instance
    create_info.enabledExtensionCount = 0;

    // We must also specify which features we will be using. These features were
    // queried with vkGetPhysicalDeviceFeatures. Instantiating a 
    // VkPhysicalDeviceFeatures object defaults all features to VK_FALSE;

    if (enable_validation_layers) {
      create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
      create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
      create_info.enabledLayerCount = 0;
    }

    // Instantiate the logical device with call from following function
    // The parameters to vkCreateDevice() are:
    //   -physical device to interface to
    //   -the queue and usage info we just set up
    //   -optional allocation callbacks pointer
    //   -pointer to variable to store the logical device in
    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create logical device!");
    }

    // This function retrieves queue handles for each queue family
    // It's parameters are:
    //   -logical device
    //   -queue family
    //   -queue index
    //   -pointer to variable to store the handle in
    vkGetDeviceQueue(device, indices.graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);

    /*
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphics_family;
    queue_create_info.queueCount = 1;
    // Vulkan requires that you assign priorities to the queues so that you can
    // influence the scheduling of the command buffer execution. To do this you
    // must assign a floating point number between 0.0 and 1.0
    queue_create_info.pQueuePriorities = &queue_priority;
    */
  }

  void create_surface() {
    // Because the glfw function takes simple parameters and not a struct
    // (unlike similar vk functions seen above) this function will be much
    // simpler
    //
    // function parameters:
    //   -VkInstance
    //   -GLFW window pointer
    //   -custom allocators
    //   -pointer to VkSurfaceKHR variable
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create window surface!");
      // This must be destroyed through vkDestroySurfaceKHR(...) in cleanup()
    }
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
