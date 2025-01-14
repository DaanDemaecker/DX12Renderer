cmake_minimum_required(VERSION 3.11)

# For GUI applications (Windows with WinMain entry point
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Set the project name for the solution
project(DX12Renderer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WIN32_EXECUTABLE TRUE)

# Handle CMake policies for better compatibility
cmake_policy(SET CMP0079 NEW)

# Include directories (optional but helpful for include paths)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add subdirectories for source, resources, and third-party dependencies
add_subdirectory(src)
add_subdirectory(Resources)
add_subdirectory(3rdParty)
