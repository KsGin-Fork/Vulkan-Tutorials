//
// Created by ksgin on 18-8-2.
//
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cassert>

using namespace std;

int main() {
    VkInstance m_instance;
    VkResult result;
    VkApplicationInfo appInfo = {};
    VkInstanceCreateInfo instanceCreateInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
    if (result == VK_SUCCESS) {
        uint32_t physicalDeviceCount = 0;
        result = vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> m_physicalDevices;
        if (result == VK_SUCCESS) {
            m_physicalDevices.resize(physicalDeviceCount);
            vkEnumeratePhysicalDevices(m_instance,
                                       &physicalDeviceCount,
                                       &m_physicalDevices[0]);
            VkPhysicalDeviceProperties m_deviceProperties;
            vkGetPhysicalDeviceProperties(m_physicalDevices[0], &m_deviceProperties);
            std::cout << m_deviceProperties.apiVersion << "\n"
                      << m_deviceProperties.deviceID << "\n"
                      << m_deviceProperties.deviceName << "\n"
                      << m_deviceProperties.deviceType << "\n"
                      << m_deviceProperties.driverVersion << std::endl;

        }
    }
    return result == VK_SUCCESS ? 1 : 0;
}
