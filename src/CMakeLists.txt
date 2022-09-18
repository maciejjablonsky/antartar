add_executable(antartar)
target_include_directories(antartar PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_sources(
    antartar
    PRIVATE
    main.cpp

    app.cpp
)

find_package(fmt REQUIRED)
target_link_libraries(antartar PRIVATE fmt::fmt)