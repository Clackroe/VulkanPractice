#include "Log/log.hpp"
#include "core.hpp"
#include <Application/Application.hpp>

#include <complex>
#include <fcntl.h>
#include <set>
#include <unistd.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h> // Add this if needed for beta features or portability extensions
#include <vulkan/vulkan_core.h>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

namespace VulkanProj {

// Validation Layer Callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity) {
    case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT):
        VKP_WARN("validation layer: {}", pCallbackData->pMessage);
        break;
    // case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT):
    //     VKP_INFO("validation layer: {}", pCallbackData->pMessage);
    //     break;
    case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):
        VKP_ERROR("validation layer: {}", pCallbackData->pMessage);
        break;
    default:
        break;
    }
    return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

static bool validationSupported()
{
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

static std::vector<const char*> getRequiredExtensions()
{
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Application::run()
{
    initVulkan();
    setupDebugCallbacks();
    createSurface();
    pickPhysicalDevice();
    setupLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();

    mainLoop();
    cleanup();
}

void Application::stopEngine()
{
    m_Running = false;
}
void Application::setupDebugCallbacks()
{
    // No Debug necessary
    if (!enableValidationLayers) {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    VkResult res = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
    VKP_ASSERT(res == VK_SUCCESS, "Failed to initialize the debug messenger")
}

void Application::createInstance()
{
    if (enableValidationLayers && !validationSupported()) {
        VKP_ASSERT(false, "Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo {};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanProject";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pEngineName = "No Engine";

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VkResult res = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    VKP_ASSERT(res == VK_SUCCESS, "Unable to create Vulkan Instance");
}

void Application::initVulkan()
{
    int success = glfwInit();
    VKP_ASSERT(success, "Failed to init glfw");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_NativeWindow = glfwCreateWindow(m_Width, m_Height, "VulkanProj", nullptr, nullptr);
    createInstance();
}

void Application::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        VKP_ASSERT(false, "No Graphics Devices With Vulkan Support Found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (VkPhysicalDevice d : devices) {
        if (isUsableDevice(d, m_Surface)) {
            m_PhysicalDevice = d;
            break;
        }
    }
    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        VKP_ASSERT(false, "No Graphics Devices With Vulkan Support Found");
    }
}

void Application::setupLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice, m_Surface);

    std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (u32 qFam : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo qInfo {};
        qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qInfo.queueFamilyIndex = qFam;
        qInfo.queueCount = 1;
        qInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(qInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo devCreateInfo {};
    devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    devCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    devCreateInfo.pEnabledFeatures = &deviceFeatures;

    devCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    devCreateInfo.enabledExtensionCount = (u32)(deviceExtensions.size());

    if (enableValidationLayers) {
        devCreateInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
        devCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        devCreateInfo.enabledLayerCount = 0;
    }

    VkResult res = vkCreateDevice(m_PhysicalDevice, &devCreateInfo, nullptr, &m_LogicalDevice);
    VKP_ASSERT(res == VK_SUCCESS, "Unable to create Logical Device");

    vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
}

void Application::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice, m_Surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_NativeWindow);

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Use for rendering to a seperate texture before presenting

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice, m_Surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain);
    VKP_ASSERT(res == VK_SUCCESS, "FAILED TO CREATE SWAPCHAIN");

    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
}

void Application::createImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (u32 i = 0; i < m_SwapChainImageViews.size(); i++) {
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_SwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_SwapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]);
        VKP_ASSERT(res == VK_SUCCESS, "FAILED TO CREATE IMAGE VIEW" + std::to_string(i));
    }
}

void Application::createSurface()
{
    VkResult res = glfwCreateWindowSurface(m_Instance, m_NativeWindow, nullptr, &m_Surface);
    VKP_ASSERT(res == VK_SUCCESS, "FAILED TO CREATE WINDOW SURFACE");
}

void Application::createGraphicsPipeline()
{
}

void Application::mainLoop()
{
    while (m_Running) {
        glfwPollEvents();
        if (glfwGetKey(m_NativeWindow, GLFW_KEY_ESCAPE)) {
            stopEngine();
        }
    }
}
void Application::cleanup()
{
    for (auto imageView : m_SwapChainImageViews) {
        vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    vkDestroyDevice(m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_NativeWindow);
    glfwTerminate();
}
}
