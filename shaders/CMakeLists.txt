find_program(GLSLC_EXECUTABLE glslc HINTS $ENV{VULKAN_SDK}/Bin)

function(compile_shaders SHADERS_NAME OUTPUT_DIR)
    message(STATUS "Creating shaders target: ${SHADERS_NAME}")
    cmake_parse_arguments(
        PARSED_ARGS
        ""
        ""
        "SOURCES"
        ${ARGN}
    )

    foreach(SOURCE ${PARSED_ARGS_SOURCES})
        get_filename_component(SHADER_NAME ${SOURCE} NAME_WE)
        set(OUTPUT_FILE "${OUTPUT_DIR}/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE} -o ${OUTPUT_FILE}
            DEPENDS ${SOURCE}
        )
        list(APPEND SHADER_OUTPUTS ${OUTPUT_FILE})
    endforeach()

    # set_source_files_properties(${SHADER_OUTPUTS} PROPERTIES GENERATED 1)
    # set_source_files_properties(${SHADER_OUTPUTS} PROPERTIES HEADER_FILE_ONLY 1)

    # Make sure the shaders are built before the main target
    add_custom_target(${SHADERS_NAME} DEPENDS ${SHADER_OUTPUTS})
endfunction()

compile_shaders(
    antartar_shaders
    ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES
        fragment_shader.frag
        vertex_shader.vert
)
