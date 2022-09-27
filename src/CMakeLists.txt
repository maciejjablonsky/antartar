add_executable(antartar)

set(ANTARTAR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(antartar PRIVATE ${ANTARTAR_INCLUDE_DIR})


target_sources(
    antartar
    PRIVATE
    include/antartar/app.hpp
    include/antartar/file.hpp
    include/antartar/log.hpp
    include/antartar/vk.hpp
    include/antartar/window.hpp
    app.cpp
    main.cpp
    window.cpp
)

find_package(fmt REQUIRED CONFIG)
target_link_libraries(antartar PRIVATE fmt::fmt)

find_package(Vulkan REQUIRED CONFIG)
target_link_libraries(antartar PRIVATE Vulkan::Vulkan)

find_package(glfw3 REQUIRED CONFIG)
target_link_libraries(antartar PRIVATE glfw)

find_package(Microsoft.GSL REQUIRED CONFIG)
target_link_libraries(antartar PRIVATE Microsoft.GSL::GSL)

find_package(range-v3 REQUIRED CONFIG)
target_link_libraries(antartar PRIVATE range-v3::range-v3)
