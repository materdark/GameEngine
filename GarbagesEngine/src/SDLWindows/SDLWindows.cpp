#include<SDLWindows/SDLWindows.h>

#ifndef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#define VK_EXT_DEBUG_REPORT_EXTENSION_NAME "VK_EXT_debug_report"
#endif
const int WIDTH = 800;
const int HEIGHT = 600;

SDL_Window* window;
VkInstance instance;

void initWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        GE_CORE_ERROR("SDL could not initialize! SDL_Error: {}", SDL_GetError());
    }
    else
    {
        window = SDL_CreateWindow("Vulkan", WIDTH, HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        if (window == NULL)
        {
            GE_CORE_ERROR("Window could not be created! SDL_Error: {}", SDL_GetError());
        }
    }
}

void initVulkan(VkPhysicalDevice& physicalDevice) {
    Uint32 count_instance_extensions;
    const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

    int count_extensions = count_instance_extensions + 1;
    std::vector<const char*> extensions(count_extensions);
    extensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    SDL_memcpy(&extensions[1], instance_extensions, count_instance_extensions * sizeof(const char*));

    // Now we can make the Vulkan instance
    VkInstanceCreateInfo create_info = {};
    create_info.enabledExtensionCount = count_extensions;
    create_info.ppEnabledExtensionNames = extensions.data();

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    //��ѯ�����豸
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        GE_CORE_ERROR("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    // ѡ��һ�������豸��������ѡ���һ����Ϊʾ����
    physicalDevice = physicalDevices[0];

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

   GE_CORE_INFO("�����豸���ԣ�");
   GE_CORE_INFO("�豸���ƣ�{}", properties.deviceName);
   GE_INFO("�豸����: {}", properties.deviceName);
   GE_CORE_INFO("�豸���ͣ�{}", static_cast<int>(properties.deviceType));
   GE_CORE_INFO("�����汾��{}", properties.driverVersion);
   GE_CORE_INFO("API �汾��{}", properties.apiVersion);
   GE_CORE_INFO("��Ӧ�� ID��{}", properties.vendorID);
   GE_CORE_INFO("�豸 ID��{}", properties.deviceID);
   GE_CORE_INFO("�豸���ƣ�");
   GE_CORE_INFO("  ���ά�ȣ�{}", properties.limits.maxImageDimension2D);
   GE_CORE_INFO("  �������ά�ȣ�{}", properties.limits.maxImageDimension3D);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

   GE_CORE_INFO("�����豸�������ԣ�");
   GE_CORE_INFO("  ������ģ�����ˣ�{}", features.samplerAnisotropy);
   GE_CORE_INFO("  ˫��Դ��ϣ�{}", features.dualSrcBlend);
   GE_CORE_INFO("  �����ƵĶ������룺{}", features.vertexPipelineStoresAndAtomics);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
   GE_CORE_INFO("�����豸�ڴ����ԣ�");
   GE_CORE_INFO("  �ڴ��������{}", memoryProperties.memoryHeapCount);
}

void processInput(bool& running)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
            }
            else if (event.key.key == SDLK_UP) {
               GE_CORE_INFO("Up arrow key pressed");
            }
            else if (event.key.key == SDLK_DOWN) {
               GE_CORE_INFO("Down arrow key pressed");
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
               GE_CORE_INFO("Left mouse button pressed at ({}, {})", event.button.x, event.button.y);
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
           GE_CORE_INFO("Mouse moved to ({}, {})", event.motion.x, event.motion.y);
            break;
        default:
            break;
        }
    }
}

void cleanup() {
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
