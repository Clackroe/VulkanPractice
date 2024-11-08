#include "Log/log.hpp"
#include <core.hpp>
#include <vulkan/vulkan.h>

#include <Application/Application.hpp>

int main()
{
    VulkanProj::Log::Init();

    VulkanProj::Application app;
    try {
        app.run();
    } catch (const std::exception& e) {
        VKP_ERROR("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
