#ifndef VKP_APPLICATION
#define VKP_APPLICATION

#include "spdlog/fmt/bundled/format.h"
#include <X11/XKBlib.h>
#include <limits>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "core.hpp"
#include <vulkan/vulkan_core.h>
namespace VulkanProj {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices ind;

    u32 queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

    for (i32 i = 0; i < queueFamilyCount; i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            ind.graphicsFamily = i;
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                ind.presentFamily = i;
            }
        }
    }

    return ind;
}

inline bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    u32 extCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);

    std::vector<VkExtensionProperties> availableExt;
    availableExt.resize(extCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExt.data());

    std::set<std::string> reqExt(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& ext : availableExt) {
        reqExt.erase(ext.extensionName);
    }

    return reqExt.empty();
}
inline bool isUsableDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
{

    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        VkExtent2D extent = {
            (u32)w,
            (u32)h,
        };
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }
}

inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

class Application {
public:
    Application(u32 width = 1280, u32 height = 720)
        : m_Width(width)
        , m_Height(height) {};
    void run();

    void stopEngine();

private:
    void initVulkan();
    void setupDebugCallbacks();
    void mainLoop();
    void cleanup();
    void createInstance();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createGraphicsPipeline();

    void pickPhysicalDevice();
    void setupLogicalDevice();

    // Vulkan
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_LogicalDevice = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkSurfaceKHR m_Surface;

    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    std::vector<VkImageView> m_SwapChainImageViews;

    // Window
    GLFWwindow* m_NativeWindow;
    u32 m_Width, m_Height;

    bool m_Running = true;
};
}

#endif
