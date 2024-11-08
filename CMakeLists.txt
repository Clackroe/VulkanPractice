cmake_minimum_required(VERSION 3.5)
project(VulkanProject)
include(FetchContent)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE SOURCES "src/*.cpp")

# Include directories
include_directories(src)

# GLFW
set(BUILD_SHARED_LIBS OFF)           
set(GLFW_VULKAN_STATIC ON)
set(GLFW_LIBRARY_TYPE STATIC)        
set(GLFW_BUILD_EXAMPLES OFF)         
set(GLFW_BUILD_TESTS OFF)            
set(GLFW_BUILD_DOCS OFF)             

add_subdirectory(lib/GLFW)
include_directories(lib/GLFW/include)
set_target_properties(glfw PROPERTIES POSITION_INDEPENDENT_CODE ON)

# GLM
include_directories(lib/GLM)

# SPDLOG
add_subdirectory(lib/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
include_directories(lib/spdlog/include)


# Vulkan
find_package(Vulkan REQUIRED)

# Platform-specific logic for Vulkan linking
if(WIN32)
    message(STATUS "Configuring for Windows")
    find_package(Vulkan REQUIRED)
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan SDK not found!")
    endif()
elseif(APPLE)
    message(STATUS "Configuring for macOS")
    find_package(Vulkan REQUIRED)
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "MoltenVK is required for Vulkan on macOS!")
    endif()
    # You may need to add MoltenVK specific includes or libraries
    # Use Vulkan SDK setup from https://vulkan.lunarg.com/
else()
    message(STATUS "Configuring for Linux")
    find_package(Vulkan REQUIRED)
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan SDK not found!")
    endif()
endif()

# Compile source files
add_executable(VulkanProject ${SOURCES})

# Link GLFW and Vulkan to the project
target_link_libraries(VulkanProject glfw spdlog Vulkan::Vulkan)
