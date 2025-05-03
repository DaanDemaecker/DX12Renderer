# Tell CMake where the header files are
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Lib")

# Link the static .lib directly to your target
target_link_libraries(DX12Lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/dxcompiler.lib"
)
