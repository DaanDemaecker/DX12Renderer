cmake_minimum_required(VERSION 3.11)

# Set the project name for the solution
project(DX12Renderer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0079 NEW)

add_subdirectory(src)
add_subdirectory(Resources)
add_subdirectory(3rdParty)