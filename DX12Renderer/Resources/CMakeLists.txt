set(SHADERS_DIR "${CMAKE_SOURCE_DIR}")


message(STATUS "Looking for shaders")

# Find all .hlsl shader files recursively
file(GLOB_RECURSE HLSL_FILES "*.hlsl")


message(STATUS "Found shaders: ${HLSL_FILES}")

# List to store compiled shader object files
set(COMPILED_SHADERS)

foreach(SHADER_FILE ${HLSL_FILES})
    # Get shader filename without extension
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)

    # Get relative path (to preserve folder structure in build output)
    get_filename_component(SHADER_PATH ${SHADER_FILE} DIRECTORY)
    file(RELATIVE_PATH REL_SHADER_PATH ${SHADERS_DIR} ${SHADER_PATH})
    set(COMPILED_SHADER_PATH "${CMAKE_BINARY_DIR}/bin/Resources/Shaders/${SHADER_NAME}.cso")

    # Ensure output directory exists
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Resources/Shaders/${REL_SHADER_PATH}")


    if(SHADER_NAME MATCHES "_VS$")
    set(SHADER_TYPE "vs_5_1")
elseif(SHADER_NAME MATCHES "_PS$")
    set(SHADER_TYPE "ps_5_1")
elseif(SHADER_NAME MATCHES "CS$")
    set(SHADER_TYPE "cs_5_1")
else()
    message(WARNING "Unknown shader type for ${SHADER_FILE}, defaulting to ps_5_1")
    set(SHADER_TYPE "ps_5_1")
endif()

    # Compile the shader using DXC
    add_custom_command(
        OUTPUT ${COMPILED_SHADER_PATH}
        COMMAND dxc -T ${SHADER_TYPE} -E main -Fo ${COMPILED_SHADER_PATH} ${SHADER_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${SHADER_FILE} to ${COMPILED_SHADER_PATH}"
    )

    list(APPEND COMPILED_SHADERS ${COMPILED_SHADER_PATH})
endforeach()

# Create a custom target to compile all shaders
add_custom_target(compile_shaders DEPENDS ${COMPILED_SHADERS})

# Ensure shaders are built before the main application
add_dependencies(DX12Lib compile_shaders)