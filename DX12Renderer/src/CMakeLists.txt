project(DX12Renderer)

set(SRC_FILES
"main.cpp"
"Application/Window.cpp"
"Application/Application.cpp"
"Application/CommandQueue.cpp"
"Application/Game.cpp"
"Games/TestGame1.cpp"
"Games/Tutorial2.cpp")


add_executable(DXRenderer ${SRC_FILES})

# Include directories specific to this target
target_include_directories(DXRenderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(DXRenderer PRIVATE user32.lib kernel32.lib)

# Copy resources after Shaders
add_custom_target(copy_resources ALL)
add_custom_command(
    TARGET copy_resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:DXRenderer>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:DXRenderer>/resources
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/resources
    DEPENDS Shaders
)