add_executable(vulkan-tutorial tutorial.cpp)
find_package(glfw3 REQUIRED CONFIG)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED CONFIG)

target_link_libraries(vulkan-tutorial
    PRIVATE
        glfw
        Vulkan::Vulkan
        glm::glm
)
add_dependencies(vulkan-tutorial antartar_shaders)
