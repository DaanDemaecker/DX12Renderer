project(DX12Renderer)

set(SRC_FILES
"main.cpp"
)

add_executable(DXRenderer
${SRC_FILES}
)


target_link_libraries(DXRenderer PRIVATE user32.lib kernel32.lib)