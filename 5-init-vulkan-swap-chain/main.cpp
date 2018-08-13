//
// Created by ksgin on 18-8-3.
//


#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <assert.h>
#include <vector>
#include <cstring>
#include <iostream>

using namespace std;
const char *title = "init vulkan swap chain";
const int screenWidth = 1024, screenHeight = 768;
SDL_Window *window;

int main() {

    // 初始化 SDL
    assert(SDL_Init(SDL_INIT_EVERYTHING) != -1);
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                              SDL_WINDOW_RESIZABLE);
    assert(window);

    SDL_SysWMinfo sdlSysWMinfo;
    SDL_GetWindowWMInfo(window, &sdlSysWMinfo);

    // 实例相关操作
    Uint32 extensionsCount = 0;
    assert(SDL_Vulkan_GetInstanceExtensions(window, &extensionsCount, nullptr));
    vector<const char *> extensions(extensionsCount);
    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME,
            assert(SDL_Vulkan_GetInstanceExtensions(window, &extensionsCount, &extensions[1]));

    VkApplicationInfo vkApplicationInfo{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            title,
            VK_MAKE_VERSION(1, 0, 0),
            title,
            VK_MAKE_VERSION(1, 0, 0),
            VK_MAKE_VERSION(1, 0, 0),
    };

    VkInstanceCreateInfo vkInstanceCreateInfo{
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,    // sType
            nullptr,   // pNext
            0,
            &vkApplicationInfo,
            0,
            nullptr,
            static_cast<uint32_t>(extensions.size()),
            &extensions[0]
    };

    VkInstance vkInstance;
    assert(vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance) == VK_SUCCESS);

    // 创建 Surface
    VkSurfaceKHR vkSurface;
    assert(SDL_Vulkan_CreateSurface(window, vkInstance, &vkSurface) == true);

    // 设备相关
    uint32_t numDevices = 8;
    assert(vkEnumeratePhysicalDevices(vkInstance, &numDevices, nullptr) == VK_SUCCESS);
    vector<VkPhysicalDevice> vkPhysicalDevices(numDevices);
    assert(vkEnumeratePhysicalDevices(vkInstance, &numDevices, &vkPhysicalDevices[0]) == VK_SUCCESS);

    Uint32 vkPhysicalDeviceIndex = UINT32_MAX;
    Uint32 vkGraphicsQueueFamilyIndex = UINT32_MAX;
    Uint32 vkPresentQueueFamilyIndex = UINT32_MAX;
    for (Uint32 i = 0; i < numDevices; ++i) {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
        vkGetPhysicalDeviceProperties(vkPhysicalDevices[i], &vkPhysicalDeviceProperties);
        vkGetPhysicalDeviceFeatures(vkPhysicalDevices[i], &vkPhysicalDeviceFeatures);
        if (vkPhysicalDeviceProperties.limits.maxImageDimension2D < 4096 &&
            VK_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion) < 1) {
            cout << vkPhysicalDeviceProperties.deviceName << "不支持" << endl;
            continue;
        }

        Uint32 queueFamiliesCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices[i], &queueFamiliesCount, nullptr);
        vector<VkQueueFamilyProperties> vkQueueFamilyProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices[i], &queueFamiliesCount,
                                                 &vkQueueFamilyProperties[0]);
        vector<VkBool32> queueSupport(queueFamiliesCount);
        Uint32 graphicsQueueFamilyIndex = UINT32_MAX, presentQueueFamilyIndex = UINT32_MAX;
        bool isFound = false;
        for (Uint32 j = 0; j < queueFamiliesCount; ++j) {
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevices[i], j, vkSurface, &queueSupport[j]);
            if (vkQueueFamilyProperties[j].queueCount > 0 &&
                vkQueueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                if (graphicsQueueFamilyIndex == UINT32_MAX) {
                    vkPresentQueueFamilyIndex = j;
                    graphicsQueueFamilyIndex = j;
                }

                if (queueSupport[j]) {
                    vkGraphicsQueueFamilyIndex = j;
                    vkPresentQueueFamilyIndex = j;
                    isFound = true;
                    break;
                }
            }
        }

        if (!isFound) {
            for (Uint32 j = 0; j < queueFamiliesCount; ++j) {
                if (queueSupport[j]) {
                    presentQueueFamilyIndex = j;
                    break;
                }
            }
        }

        assert(!(graphicsQueueFamilyIndex == UINT32_MAX && presentQueueFamilyIndex == UINT32_MAX));

        vkGraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
        vkPresentQueueFamilyIndex = presentQueueFamilyIndex;

        vkPhysicalDeviceIndex = i;
    }
    assert(vkPhysicalDeviceIndex != UINT32_MAX);

    // 检查设备扩展
    Uint32 deviceExtensionsCount = 0;
    assert(vkEnumerateDeviceExtensionProperties(vkPhysicalDevices[vkPhysicalDeviceIndex], nullptr,
                                                &deviceExtensionsCount, nullptr) == VK_SUCCESS &&
           extensionsCount != 0);
    vector<VkExtensionProperties> vkDeviceExtensionProperties(deviceExtensionsCount);
    assert(vkEnumerateDeviceExtensionProperties(vkPhysicalDevices[vkPhysicalDeviceIndex], nullptr,
                                                &deviceExtensionsCount, &vkDeviceExtensionProperties[0]) ==
           VK_SUCCESS && extensionsCount != 0);
    vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    for (auto &extension : deviceExtensions) {
        for (auto &vkExtensionPropertie : vkDeviceExtensionProperties) {
            if (strcmp(vkExtensionPropertie.extensionName, extension) == 0)
                break;
        }
    }

    vector<VkDeviceQueueCreateInfo> vkDeviceQueueCreateInfos;
    vector<float> queuePriorities = {1.0f};
    vkDeviceQueueCreateInfos.push_back({
                             VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                             nullptr,
                             0,
                             vkGraphicsQueueFamilyIndex,
                             static_cast<uint32_t>(queuePriorities.size()),
                             &queuePriorities[0]
                     });
    if (vkGraphicsQueueFamilyIndex != vkPresentQueueFamilyIndex) {
        vkDeviceQueueCreateInfos.push_back({
                                 VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                 nullptr,
                                 0,
                                 vkPresentQueueFamilyIndex,
                                 static_cast<uint32_t>(queuePriorities.size()),
                                 &queuePriorities[0]
                         });
    }

    VkDeviceCreateInfo vkDeviceCreateInfo{
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(vkDeviceQueueCreateInfos.size()),
            &vkDeviceQueueCreateInfos[0],
            0,
            nullptr,
            static_cast<uint32_t>(deviceExtensions.size()),
            &deviceExtensions[0],
            nullptr
    };

    VkDevice vkDevice;
    assert(vkCreateDevice(vkPhysicalDevices[vkPhysicalDeviceIndex], &vkDeviceCreateInfo, nullptr, &vkDevice) ==
           VK_SUCCESS);

    VkQueue vkQueue;
    vkGetDeviceQueue(vkDevice, vkGraphicsQueueFamilyIndex, 0, &vkQueue);

    VkSemaphoreCreateInfo vkSemaphoreCreateInfo{
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            nullptr,
            0
    };

    VkSemaphore imageAvailableSemaphore, renderFinishSemaphore;
    assert(vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, nullptr, &imageAvailableSemaphore) == VK_SUCCESS);
    assert(vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, nullptr, &renderFinishSemaphore) == VK_SUCCESS);

    VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR;
    assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevices[vkPhysicalDeviceIndex], vkSurface,
                                                     &surfaceCapabilitiesKHR) == VK_SUCCESS);

    Uint32 formatsCount;
    assert(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevices[vkPhysicalDeviceIndex], vkSurface, &formatsCount,
                                                nullptr) == VK_SUCCESS);
    vector<VkSurfaceFormatKHR> vkSurfaceFormats(formatsCount);
    assert(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevices[vkPhysicalDeviceIndex], vkSurface, &formatsCount,
                                                &vkSurfaceFormats[0]) == VK_SUCCESS);

    // 查找可用的 present mode 并选择
    Uint32 presentModesCount;
    assert(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevices[vkPhysicalDeviceIndex], vkSurface,
                                                     &presentModesCount,
                                                     nullptr) == VK_SUCCESS);
    vector<VkPresentModeKHR> vkPresentModes(presentModesCount);
    assert(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevices[vkPhysicalDeviceIndex], vkSurface,
                                                     &presentModesCount,
                                                     &vkPresentModes[0]) == VK_SUCCESS);
    VkPresentModeKHR vkPresentMode;
    for (auto &mode : vkPresentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) vkPresentMode = mode;
    }
    if (vkPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
        for (auto &mode : vkPresentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) vkPresentMode = mode;
        }
    }


    Uint32 imageCount = surfaceCapabilitiesKHR.minImageCount + 1;
    if (surfaceCapabilitiesKHR.maxImageCount > 0 && imageCount > surfaceCapabilitiesKHR.maxImageCount) {
        imageCount = surfaceCapabilitiesKHR.maxImageCount;
    }

    VkSwapchainKHR vkOldSwapchain = VK_NULL_HANDLE;
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR{
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            nullptr,
            0,
            vkSurface,
            imageCount,
            vkSurfaceFormats[0].format,
            vkSurfaceFormats[0].colorSpace,
            surfaceCapabilitiesKHR.currentExtent,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr,
            surfaceCapabilitiesKHR.currentTransform,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            vkPresentMode,
            VK_TRUE,
            vkOldSwapchain
    };

    VkSwapchainKHR vkSwapchain;
    assert(vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &vkSwapchain));

    if (vkOldSwapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(vkDevice, vkOldSwapchain, nullptr);


    cout << "创建交换链成功" << endl;


    if (vkSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
    }

    if (vkDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vkDevice);
        vkDestroyDevice(vkDevice, nullptr);
    }

    if (vkInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(vkInstance, nullptr);
    }

}