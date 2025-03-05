project(DX12Renderer)

set(SRC_FILES
"main.cpp"
"Window.cpp"
"Application.cpp"
"CommandQueue.cpp"
"Game.cpp"
"Games/TestGame1.cpp")

add_executable(DXRenderer ${SRC_FILES})

# Include directories specific to this target
target_include_directories(DXRenderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(DXRenderer PRIVATE user32.lib kernel32.lib)