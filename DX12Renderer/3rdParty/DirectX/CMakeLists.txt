# DX12 libraries
target_link_libraries(DXRenderer PRIVATE
    d3d12.lib
    dxgi.lib
    dxguid.lib
)

#include(FetchContent)
#
## DX12 extension library
#FetchContent_Declare(
#	DirectX-Headers
#	GIT_REPOSITORY	https://github.com/microsoft/DirectX-Headers
#	GIT_TAG 	v1.614.1
#	GIT_SHALLOW true
#)
#
#FetchContent_MakeAvailable(DirectX-Headers) 
#
#
#target_link_libraries(DXRenderer PRIVATE DirectX-Headers)