cmake_minimum_required(VERSION 3.11)

# For GUI applications (Windows with WinMain entry point
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Set the project name for the solution
project(DX12Renderer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WIN32_EXECUTABLE TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

# Handle CMake policies for better compatibility
cmake_policy(SET CMP0079 NEW) 

# Add subdirectories for source, resources, and third-party dependencies
add_subdirectory(DX12Lib)
add_subdirectory(Tutorial2)
add_subdirectory(Tutorial3)
add_subdirectory(RayTracer)
add_subdirectory(Resources)
add_subdirectory(3rdParty)


# Copy resources after Shaders
add_custom_target(copy_resources ALL)
add_custom_command(
    TARGET copy_resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/Resources
    DEPENDS compile_shaders
)