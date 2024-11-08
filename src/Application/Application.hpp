#ifndef VKP_APPLICATION
#define VKP_APPLICATION
#include "GLFW/glfw3.h"
#include "core.hpp"
#include <vulkan/vulkan_core.h>
namespace VulkanProj {
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

    void pickPhysicalDevice();
    void setupLogicalDevice();

    // Vulkan
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_LogicalDevice = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue;

    // Window
    GLFWwindow* m_NativeWindow;
    u32 m_Width, m_Height;

    bool m_Running = true;
};
}

#endif
