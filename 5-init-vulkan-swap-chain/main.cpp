//
// Created by ksgin on 18-8-3.
//

#define VK_USE_PLATFORM_XLIB_KHR

#include <vulkan/vulkan.h>
#include <assert.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

using namespace std;
const char *title = "init vulkan swap chain";
const int screenWidth = 1024, screenHeight = 768;
SDL_Window *window;

int main() {

    u_int32_t extensions_count = 0;
    assert(vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr) == VK_SUCCESS &&
           extensions_count != 0);
    vector<VkExtensionProperties> vk_extension_properties(extensions_count);
    assert(vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, &vk_extension_properties[0]) ==
           VK_SUCCESS && extensions_count != 0);

    vector<const char *> extensions{
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME
    };

    for (auto &extension : extensions) {
        for (auto &vk_extension_propertie : vk_extension_properties) {
            if (strcmp(vk_extension_propertie.extensionName, extension) == 0)
                break;
        }
    }

    VkApplicationInfo vkAppi{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            title,
            VK_VERSION_1_0,
            title,
            VK_VERSION_1_0,
            VK_API_VERSION_1_1
    };

    VkInstanceCreateInfo vkIci{
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,    // sType
            nullptr,   // pNext
            0,
            &vkAppi,
            0,
            nullptr,
            static_cast<uint32_t>(extensions.size()),
            &extensions[0]
    };

    VkInstance vk_instance;
    assert(vkCreateInstance(&vkIci, nullptr, &vk_instance) == VK_SUCCESS);

    uint32_t num_devices = 8;
    assert(vkEnumeratePhysicalDevices(vk_instance, &num_devices, nullptr) == VK_SUCCESS);
    vector<VkPhysicalDevice> vk_physical_devices(num_devices);
    assert(vkEnumeratePhysicalDevices(vk_instance, &num_devices, &vk_physical_devices[0]) == VK_SUCCESS);

    int32_t vk_physical_device_index = -1;
    int32_t vk_queue_family_propertie_index = -1;
    for (u_int32_t i = 0; i < num_devices; ++i) {
        VkPhysicalDeviceProperties vk_physical_device_properties;
        VkPhysicalDeviceFeatures vk_physical_device_features;
        vkGetPhysicalDeviceProperties(vk_physical_devices[i], &vk_physical_device_properties);
        vkGetPhysicalDeviceFeatures(vk_physical_devices[i], &vk_physical_device_features);
        if (vk_physical_device_properties.limits.maxImageDimension2D < 4096 &&
            VK_VERSION_MAJOR(vk_physical_device_properties.apiVersion) < 1) {
            cout << vk_physical_device_properties.deviceName << "不支持" << endl;
            continue;
        }

        u_int32_t queue_families_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_devices[i], &queue_families_count, nullptr);
        vector<VkQueueFamilyProperties> vk_queue_family_properties(queue_families_count);
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_devices[i], &queue_families_count,
                                                 &vk_queue_family_properties[0]);
        for (u_int32_t j = 0; j < queue_families_count; ++j) {
            if (vk_queue_family_properties[j].queueCount > 0 &&
                vk_queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                vk_queue_family_propertie_index = j;
                break;
            }
        }
        vk_physical_device_index = i;
    }
    assert(vk_physical_device_index >= 0);

    vector<float> queue_priorities = {1.0f};
    VkDeviceQueueCreateInfo vDqci{
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(vk_queue_family_propertie_index),
            static_cast<uint32_t>(queue_priorities.size()),
            &queue_priorities[0]
    };
    VkDeviceCreateInfo vDci{
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            1,
            &vDqci,
            0,
            nullptr,
            0,
            nullptr,
            nullptr
    };
    VkDevice vk_device;
    assert(vkCreateDevice(vk_physical_devices[vk_physical_device_index], &vDci, nullptr, &vk_device) == VK_SUCCESS);

    VkQueue vk_queue;
    vkGetDeviceQueue(vk_device, static_cast<uint32_t>(vk_queue_family_propertie_index), 0, &vk_queue);

    ///////////////////////////////////////// 以下为新增内容 ////////////////////////////////////////////////////////////////////

    assert(SDL_Init(SDL_INIT_EVERYTHING) != -1);
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                              SDL_WINDOW_RESIZABLE);
    assert(window);

    SDL_SysWMinfo sdl_sysWMinfo;
    SDL_GetWindowWMInfo(window, &sdl_sysWMinfo);

    VkXlibSurfaceCreateInfoKHR vSci{
            VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
            nullptr,
            0,
            sdl_sysWMinfo.info.x11.display,
            sdl_sysWMinfo.info.x11.window
    };

    VkSurfaceKHR vk_surface;
    assert(vkCreateXlibSurfaceKHR(vk_instance, &vSci, nullptr, &vk_surface) == VK_SUCCESS);

    u_int32_t device_extensions_count = 0;
    assert(vkEnumerateDeviceExtensionProperties(vk_physical_devices[vk_physical_device_index], nullptr,
                                                &device_extensions_count, nullptr) == VK_SUCCESS &&
           extensions_count != 0);
    vector<VkExtensionProperties> vk_device_extension_properties(device_extensions_count);
    assert(vkEnumerateDeviceExtensionProperties(vk_physical_devices[vk_physical_device_index], nullptr,
                                                &device_extensions_count, &vk_device_extension_properties[0]) ==
           VK_SUCCESS && extensions_count != 0);
    vector<const char *> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    for (auto &extension : device_extensions) {
        for (auto &vk_extension_propertie : vk_device_extension_properties) {
            if (strcmp(vk_extension_propertie.extensionName, extension) == 0)
                break;
        }
    }


    if (vk_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vk_device);
        vkDestroyDevice(vk_device, nullptr);
    }

    if (vk_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(vk_instance, nullptr);
    }

}