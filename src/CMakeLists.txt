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