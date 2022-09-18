add_executable(antartar)

set(ANTARTAR_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
target_include_directories(antartar PRIVATE ${ANTARTAR_INCLUDE_DIR})


target_sources(
    antartar
    PRIVATE
    main.cpp
    app.cpp
    window.cpp
)

find_package(fmt REQUIRED)
target_link_libraries(antartar PRIVATE fmt::fmt)

find_package(Vulkan REQUIRED)
target_link_libraries(antartar PRIVATE Vulkan::Vulkan)

find_package(glfw3 REQUIRED)
target_link_libraries(antartar PRIVATE glfw)
