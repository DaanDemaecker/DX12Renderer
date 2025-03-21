# DX12 libraries
target_link_libraries(DXRenderer PRIVATE
    d3d12.lib
    dxgi.lib
    dxguid.lib
    D3DCompiler.lib
)


add_library(directXHelpers INTERFACE)

target_include_directories(directXHelpers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(DXRenderer PRIVATE directXHelpers)